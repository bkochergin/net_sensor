/*
 * Copyright 2009-2011 Boris Kochergin. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <string.h>

#include <include/string.h>

#include "smtp.h"

SMTP::SMTP() {
  session = NULL;
  authContext = NULL;
}

bool SMTP::initialize(const std::string &server, const size_t auth,
                      const std::string &user, const std::string &password,
                      const std::vector <std::string> &recipients) {
  int error = pthread_mutex_init(&mutex, NULL);
  if (error != 0) {
    errorMessage = "SMTP::initialize(): pthread_mutex_init(): ";
    errorMessage += strerror(error);
    return false;
  }
  _server = server.substr(0, server.find(':'));
  session = smtp_create_session();
  if (session == NULL) {
    errorMessage = "smtp_create_session(): ";
    errorMessage += smtp_strerror(smtp_errno(), _error, sizeof(_error));
    return false;
  }
  _message = smtp_add_message(session);
  if (_message == NULL) {
    errorMessage = "smtp_add_message(): ";
    errorMessage +=  smtp_strerror(smtp_errno(), _error, sizeof(_error));
    return false;
  }
  if (smtp_set_server(session, server.c_str()) == 0) {
    errorMessage = "smtp_set_server(): " + server + ": " +
                   smtp_strerror(smtp_errno(), _error, sizeof(_error));
    return false;
  }
  if (auth == 1) {
    _user = user;
    _password = password;
    auth_client_init();
    authContext = auth_create_context();
    if (authContext == NULL) {
      errorMessage = "auth_create_context(): ";
      errorMessage += strerror(ENOMEM);
      return false;
    }
    auth_set_mechanism_flags(authContext, AUTH_PLUGIN_PLAIN, 0);
    auth_set_interact_cb(authContext, authCallback, this);
    smtp_auth_set_context(session, authContext);
  }
  if (smtp_set_header(_message, "To", NULL, NULL) == 0) {
    errorMessage = "smtp_set_header(): ";
    errorMessage += smtp_strerror(smtp_errno(), _error, sizeof(_error));
    return false;
  }
  for (size_t i = 0; i < recipients.size(); ++i) {
    recipient = smtp_add_recipient(_message, recipients[i].c_str());
    if (recipient == NULL) {
      errorMessage = "smtp_add_recipient(): ";
      errorMessage += smtp_strerror(smtp_errno(), _error, sizeof(_error));
      return false;
    }
  }
  if (smtp_set_messagecb(_message, &messageCallback, this) == 0) {
    errorMessage = "smtp_set_messagecb(): ";
    errorMessage += smtp_strerror(smtp_errno(), _error, sizeof(_error));
    return false;
  }
  if (smtp_set_monitorcb(session, &monitorCallback, this, 0) == 0) {
    errorMessage = "smtp_set_monitorcb(): ";
    errorMessage += smtp_strerror(smtp_errno(), _error, sizeof(_error));
    return false;
  }
  return true;
}

int SMTP::lock() {
  return pthread_mutex_lock(&mutex);
}

int SMTP::unlock() {
  return pthread_mutex_unlock(&mutex);
}

bool SMTP::from(const std::string from) {
  if (smtp_set_reverse_path(_message, from.c_str()) == 0) {
    errorMessage = "smtp_set_reverse_path(): ";
    errorMessage += smtp_strerror(smtp_errno(), _error, sizeof(_error));
    return false;
  }
  return true;
}

void SMTP::subject(const std::string subject) {
  _subject = "Subject: " + subject + "\r\n";
}

void SMTP::message(const std::string message) {
  __message = "\r\n" + message;
}

bool SMTP::send() {
  messageStatus = SUBJECT;
  if (smtp_message_reset_status(_message) == 0) {
    errorMessage = "smtp_message_reset_status(): ";
    errorMessage += smtp_strerror(smtp_errno(), _error, sizeof(_error));
    return false;
  }
  if (smtp_recipient_reset_status(recipient) == 0) {
    errorMessage = "smtp_recipient_reset_status(): ";
    errorMessage += smtp_strerror(smtp_errno(), _error, sizeof(_error));
    return false;
  }
  smtpErrors.clear();
  if (smtp_start_session(session) == 0) {
    errorMessage = "smtp_start_session(): " + _server + ": " + strerror(errno);
    return false;
  }
  if (smtpErrors.size() > 0) {
    errorMessage = "smtp_start_session(): " + _server + ": " +
                   implode(smtpErrors, ", ");
    return false;
  }
  return true;
}

const std::string &SMTP::error() const {
  return errorMessage;
}

const char *messageCallback(void *buffer[] __attribute__((unused)), int *length, void *smtp) {
  SMTP *_smtp = (SMTP*)smtp;
  if (length == NULL) {
    return NULL;
  }
  switch (_smtp -> messageStatus) {
    case SUBJECT:
      _smtp -> messageStatus = MESSAGE;
      *length = _smtp -> _subject.length();
      return _smtp -> _subject.c_str();
      break;
    case MESSAGE:
      _smtp -> messageStatus = DONE;
      *length = _smtp -> __message.length();
      return _smtp -> __message.c_str();
      break;
    case DONE:
      return NULL;
  }
  /* Not reached. */
  return NULL;
}

/*
 * Records SMTP errors from the server, which are defined as lines that do not
 * begin with '2' or '3' (the 200 and 300 series of SMTP response codes do not
 * indicate errors).
 */
void monitorCallback(const char *buffer, int length, int writing, void *smtp) {
  SMTP *_smtp = (SMTP*)smtp;
  size_t newline;
  if (writing == 0) {
    _smtp -> _buffer.append(buffer, length);
    while ((newline = _smtp -> _buffer.find("\r\n")) != std::string::npos) {
      if (_smtp -> _buffer[0] != '2' && _smtp -> _buffer[0] != '3') {
        _smtp -> smtpErrors.push_back(_smtp -> _buffer.substr(0, newline));
      }
      _smtp -> _buffer.erase(0, newline + 2);
    }
  }
}

int authCallback(auth_client_request_t request, char *result[], int fields,
                 void *smtp) {
  SMTP *_smtp = (SMTP*)smtp;
  for (int i = 0; i < fields; ++i) {
    if (request[i].flags & AUTH_USER) {
      result[i] = (char*)(_smtp -> _user.c_str());
    }
    else {
      result[i] = (char*)(_smtp -> _password.c_str());
    }
  }
  return 1;
}

SMTP::~SMTP() {
  smtp_destroy_session(session);
  auth_destroy_context(authContext);
  auth_client_exit();
}
