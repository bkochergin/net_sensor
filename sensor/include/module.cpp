/*
 * Copyright 2010-2015 Boris Kochergin. All rights reserved.
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

#include <cstring>
#include <limits>

#include <dlfcn.h>

#include <include/string.h>

#include "module.h"

Module::Module(const std::string& moduleDirectory,
               const std::string& configurationDirectory,
               const std::string& name) {
  std::string moduleErrorMessage;
  pcap_t *pcapDescriptor;
  name_ = name;
  fileName_ = moduleDirectory + '/' + name + ".so";
  if (!conf_.initialize(configurationDirectory + '/' + name + ".conf")) {
    error_ = true;
    errorMessage = conf_.error();
    return;
  }
  handle_ = dlopen(fileName_.c_str(), RTLD_NOW);
  if (handle_ == nullptr) {
    error_ = true;
    errorMessage = "dlopen(): ";
    errorMessage += dlerror();
    return;
  }
  initialize_ = dlsym(handle_, "initialize");
  flush = (flushFunction)dlsym(handle_, "flush");
  finish = (finishFunction)dlsym(handle_, "finish");
  processPacket = nullptr;
  callback_ = nullptr;
  if (initialize_ == nullptr || flush == nullptr || finish == nullptr) {
    error_ = true;
    errorMessage = fileName_ + ": dlsym(): " + dlerror();
    return;
  }
  char* callback__ = (char*)dlsym(handle_, "callback");
  if (callback__ != nullptr) {
    callback_ = *(char**)callback__;
  }
  pcapDescriptor = pcap_open_dead(
      DLT_EN10MB, std::numeric_limits <uint16_t>::max() /* Snapshot length. */);
  if (pcapDescriptor == nullptr) {
    error_ = true;
    errorMessage = pcap_geterr(pcapDescriptor);
    return;
  }
  /*
   * Older versions of libpcap expect the third argument of pcap_compile()
   * to be of type "char*".
   */
  std::vector<std::string> filters;
  for (const std::string& filter : conf_.getStrings("filter")) {
    filters.push_back('(' + filter + ')');
  }
  filter_ = '(' + implode(filters, " and ") + ')';
  if (pcap_compile(pcapDescriptor, &bpfProgram_, (char*)filter_.c_str(),
                   1 /* Optimize. */, 0 /* Netmask. */) == -1) {
    error_ = true;
    errorMessage = fileName_ + ": pcap_compile(): " +
                   pcap_geterr(pcapDescriptor);
    return;
  }
  pcap_close(pcapDescriptor);
  error_ = false;
}

int Module::initialize(Logger& logger) {
  initializeFunction initializeFunction_ = (initializeFunction)initialize_;
  dependencyInitializeFunction dependencyInitializeFunction_ =
      (dependencyInitializeFunction)initialize_;
  std::string moduleErrorMessage;
  int ret;
  if (callbacks_.empty()) {
    ret = initializeFunction_(conf_, logger, moduleErrorMessage);
  }
  else {
    ret = dependencyInitializeFunction_(conf_,logger, callbacks_,
                                        moduleErrorMessage);
  }
  if (ret != 0) {
    error_ = true;
    errorMessage = "initialize(): " + moduleErrorMessage;
    return 1;
  }
  return 0;
}

Module::operator bool() const {
  return !error_;
}

const std::string &Module::error() const {
  return errorMessage;
}

const std::string& Module::filter() const {
  return filter_;
}

const bpf_program& Module::bpfProgram() const {
  return bpfProgram_;
}

const std::string& Module::name() const {
  return name_;
}

const std::string& Module::fileName() const {
  return fileName_;
}

const Configuration& Module::conf() const {
  return conf_;
}

void* Module::handle() const {
  return handle_;
}

const char* Module::callback() const {
  return callback_;
}

std::vector<void*>& Module::callbacks() {
  return callbacks_;
}
