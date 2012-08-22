/*
 * Copyright 2012 Boris Kochergin. All rights reserved.
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

#include "country.h"

#include <cerrno>
#include <climits>
#include <cstdlib>

#include <arpa/inet.h>
#include <fstream>
#include <stdint.h>

#include <include/address.h>
#include <include/string.h>



Country::_Country::_Country(const uint32_t &_lastIP,
                            const std::string &_country) {
  lastIP = _lastIP;
  country = _country;
}

Country::Country() {
  _error = true;
  errorMessage = "Country::Country(): class not initialized";
}

Country::Country(const std::vector <std::string> &allocations,
                 const std::string names) {
  initialize(allocations, names);
}

bool Country::initialize(const std::vector <std::string> &allocations,
                         const std::string names) {
  std::ifstream file;
  std::string line;
  std::vector <std::string> fields;
  uint32_t firstIP, lastIP;
  for (size_t i = 0; i < allocations.size(); ++i) {
    file.open(allocations[i].c_str());
    if (!file) {
      _error = true;
      errorMessage = "Country::initialize(): " + allocations[i] + ": " +
                     strerror(errno);
      return false;
    }
    while (getline(file, line)) {
      fields = explode(line, "|");
      if (fields.size() == 7 && fields[2] == "ipv4" && fields[1] != "*") {
        firstIP = ntohl(binaryIP(fields[3]));
        lastIP = firstIP + strtoul(fields[4].c_str(), NULL, 10);
        countryMap.insert(std::make_pair(firstIP, _Country(lastIP,
                                                           fields[1])));
      }
    }
    file.close();
  }
  file.open(names.c_str());
  if (!file) {
    errorMessage = "Country::initialize(): " + names + ": " +
                   strerror(errno);
    return false;
  }
  while (getline(file, line)) {
    if (line[0] != '#') {
      fields = explode(line, "\t");
      if (fields.size() == 4) {
        countryNames.insert(make_pair(fields[0], fields[3]));
      }
    }
  }
  _error = false;
  errorMessage.clear();
  return true;
}

Country::operator bool() const {
  return !_error;
}

const std::string &Country::error() {
  return errorMessage;
}

const std::string &Country::find(const uint32_t &ip) {
  countryItr = --countryMap.upper_bound(ip);
  if (ip >= countryItr -> first && ip <= countryItr -> second.lastIP) {
    countryNameItr = countryNames.find(countryItr -> second.country);
    if (countryNameItr != countryNames.end()) {
      return countryNameItr -> second;
    }
    return countryItr -> second.country;
  }
  return empty;
}
