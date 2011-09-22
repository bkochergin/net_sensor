/*
 * Copyright 2011 Boris Kochergin. All rights reserved.
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
 *
 * This work was sponsored by the New York Internet Company.
 */

#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <tr1/memory>
#include <tr1/unordered_map>

#include <errno.h>
#include <string.h>
#include <time.h>

#include <include/address.h>
#include <include/configuration.h>
#include <include/endian.h>
#include <include/logger.h>
#include <include/memory.hpp>
#include <include/packet.h>
#include <include/smtp.h>

#include "stats.hpp"

using namespace std;
using namespace tr1;

/* A prefix tree of internal networks whose IPs we'll be monitoring. */
map <uint32_t, uint32_t> networks;
/*
 * The stats table is a hash table of shared pointers to Stats structures with
 * IP addresses as keys.
 */
static unordered_map <uint32_t, shared_ptr <Stats> > addressStats;
/* Stats memory allocator. */
static Memory <Stats> memory;
/* Locks for the stats table. */
static pthread_mutex_t *locks;
static bool warning = true;
static uint32_t timeout, threshold, mailInterval, lastFlush;
static Logger *logger;

static SMTP smtp;
static const string crlf = "\r\n";

static const string pad(const string _string, size_t length) {
  if (_string.length() < length * 8) {
    return _string + string(length - _string.length() / 8, '\t');
  }
  return _string;
}

/* Converts number of bytes to human-readable form. */
static string size(double bytes) {
  ostringstream size;
  if (bytes == 0) {
    return "0 bytes";
  }
  if (bytes < 1024) {
    size << bytes << " bytes";
    return size.str();
  }
  else {
    size.precision(2);
    size.setf(ios::fixed);
    bytes /= 1024;
    if (bytes < 1024) {
      size << bytes << " KiB";
      return size.str();
    }
    else {
      bytes /= 1024;
      if (bytes < 1024) {
        size << bytes << " MiB";
        return size.str();
      }
      else {
        bytes /= 1024;
        if (bytes < 1024) {
          size << bytes << " GiB";
          return size.str();
        }
        else {
          bytes /= 1024;
          if (bytes < 1024) {
            size << bytes << " TiB";
            return size.str();
          }
        }
      }
    }
  }
  bytes /= 1024;
  size << bytes << "PiB";
  return size.str();
}

extern "C" {
  int initialize(const Configuration &conf, Logger &logger, string &error) {
    int _error;
    timeout = conf.getNumber("timeout");
    threshold = conf.getNumber("threshold");
    mailInterval = conf.getNumber("mailInterval");
    lastFlush = time(NULL);
    ::logger = &logger;
    /*
     * Rehash the address stats table for as many IPs as we may need to hold in
     * it.
     */
    addressStats.rehash(conf.getNumber("maxIPs"));
    /*
     * Because Stats structures are fairly small, we will pre-allocate as many
     * of them as we may need so that we can later hand them out in constant
     * time.
     */
    if (!memory.initialize(conf.getNumber("maxIPs"))) {
      error = memory.error();
      return 1;
    }
    /*
     * We will be locking the stats hash table one bucket at a time, so
     * allocate one mutex per bucket.
     */
    locks = new(nothrow) pthread_mutex_t[addressStats.bucket_count()];
    if (locks == NULL) {
      error = "malloc(): ";
      error += strerror(errno);
      return 1;
    }
    for (size_t i = 0; i < addressStats.bucket_count(); ++i) {
      _error = pthread_mutex_init(&(locks[i]), NULL);
      if (_error != 0) {
        error = "pthread_mutex_init(): ";
        error += strerror(_error);
        return 1;
      }
    }
    if (!smtp.initialize(conf.getString("smtpServer"),
                         conf.getNumber("smtpAuth"),
                         conf.getString("smtpUser"),
                         conf.getString("smtpPassword"),
                         conf.getStrings("recipient"))) {
      error = smtp.error();
      return 1;
    }
    if (!smtp.from(conf.getString("from"))) {
      error = smtp.error();
      return 1;
    }
    for (size_t i = 0; i < conf.getStrings("addresses").size(); ++i) {
      networks.insert(cidrToIPs(conf.getStrings("addresses")[i]));
    }
    return 0;
  }

  /*
   * Determines whether an IP address belongs to any of our internal networks
   * in logarithmic time.
   */
  bool internal(const map <uint32_t, uint32_t> &networks, const uint32_t &ip) {
    static map <uint32_t, uint32_t>::const_iterator itr;
    itr = --networks.upper_bound(ip);
    return (ip >= itr -> first && ip <= itr -> second);
  }

  int processPacket(const Packet &packet) {
    static size_t bucket;
    static unordered_map <uint32_t, shared_ptr <Stats> >::iterator itr;
    static shared_ptr <Stats> stats;
    if (internal(networks, ntohl(packet.sourceIP())) == true) {
      bucket = addressStats.bucket(packet.sourceIP());
      pthread_mutex_lock(&(locks[bucket]));
      itr = addressStats.find(packet.sourceIP());
      if (itr == addressStats.end()) {
        stats = memory.allocate();
        if (stats == shared_ptr <Stats>()) {        
          if (warning == true) {
            logger -> lock();
            (*logger) << "PPS module: stats table is full." << endl;
            logger -> unlock();
            warning = false;
          }
          pthread_mutex_unlock(&(locks[bucket]));
          return 0;
        }
        itr = addressStats.insert(make_pair(packet.sourceIP(), stats)).first;
      }
      itr -> second -> lastUpdate = packet.time().seconds();
      ++(itr -> second -> outgoingPackets);
      itr -> second -> outgoingBytes += packet.capturedSize();
      pthread_mutex_unlock(&(locks[bucket]));
    }
    else {
      if (internal(networks, ntohl(packet.destinationIP())) == true) {
        bucket = addressStats.bucket(packet.destinationIP());
        pthread_mutex_lock(&(locks[bucket]));
        itr = addressStats.find(packet.destinationIP());
        if (itr == addressStats.end()) {
          stats = memory.allocate();
          if (stats == shared_ptr <Stats>()) {
            if (warning == true) {
              logger -> lock();
              (*logger) << "PPS module: stats table is full." << endl;
              logger -> unlock();
              warning = false;
            }
            pthread_mutex_unlock(&(locks[bucket]));
            return 0;
          }
          itr = addressStats.insert(make_pair(packet.destinationIP(), stats)).first;
        }
        itr -> second -> lastUpdate = packet.time().seconds();
        ++(itr -> second -> incomingPackets); 
        itr -> second -> incomingBytes += packet.capturedSize();
        pthread_mutex_unlock(&(locks[bucket]));
      }
    }
    return 0;
  }

  int flush() {
    static time_t _time;
    static unordered_map <uint32_t, shared_ptr <Stats> >::local_iterator localItr;
    static vector <uint32_t> erase;
    static uint64_t incomingPPS, outgoingPPS;
    static ostringstream subject, message;
    _time = time(NULL);
    if (addressStats.size() > 0) {
      for (size_t i = 0; i < addressStats.bucket_count(); ++i) {
        /*
         * Lock the bucket in which we will be checking for timed-out IPs to
         * prevent a race with processPacket().
         */
        pthread_mutex_lock(&(locks[i]));
        for (localItr = addressStats.begin(i); localItr != addressStats.end(i);
             ++localItr) {
          /*
           * Remove an IP from memory if it has been idle for at least as long
           * as the configured idle timeout.
           */
          if (_time - localItr -> second -> lastUpdate >= timeout) {
            erase.push_back(localItr -> first);
          }
          incomingPPS = localItr -> second -> incomingPackets / (_time - lastFlush);
          outgoingPPS = localItr -> second -> outgoingPackets / (_time - lastFlush);
          if ((incomingPPS >= threshold || outgoingPPS >= threshold) &&
              _time - localItr -> second -> lastEmail >= mailInterval) {
            localItr -> second -> lastEmail = _time; 
            subject << threshold << " Packets/s Threshold Exceeded by "
                    << textIP(localItr -> first);
            message << "The IP address " << textIP(localItr -> first)
                    << " has exceeded the configured threshold of "
                    << threshold << " packets/s in one direction." << crlf
                    << crlf << pad("Incoming packet rate:", 3)
                    << incomingPPS << "/s" << crlf
                    << pad("Outgoing packet rate:", 3)
                    << outgoingPPS << "/s" << crlf
                    << pad("Incoming data rate:", 3)
                    << size(localItr -> second -> incomingBytes / (_time - lastFlush))
                    << "/s" << crlf << pad("Outgoing data rate:", 3)
                    << size(localItr -> second -> outgoingBytes / (_time - lastFlush))
                    << "/s" << crlf << crlf << "Another e-mail about this IP "
                    << "will not be sent for " << mailInterval
                    << " seconds.";
            smtp.subject(subject.str());
            smtp.message(message.str());            
            if (!smtp.send()) {
              logger -> lock();
              (*logger) << logger -> time() << "PPS: smtp::send(): "
                        << smtp.error() << endl;
              logger -> unlock();
            }
            subject.str("");
            message.str("");
          }
          localItr -> second -> incomingPackets = 0;
          localItr -> second -> outgoingPackets = 0;
          localItr -> second -> incomingBytes = 0;
          localItr -> second -> outgoingBytes = 0;
        }
        for (size_t j = 0; j < erase.size(); ++j) {
          addressStats.erase(addressStats.find(erase[j]));
        }
        pthread_mutex_unlock(&(locks[i]));
        erase.clear();
      }
    }
    lastFlush = _time;
    return 0;
  }

  int finish() {
    return 0;
  }
}