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

#ifndef COUNTRY_H
#define COUNTRY_H

#include <map>
#include <string>
#include <tr1/unordered_map>
#include <vector>

#include <stdint.h>

class Country {
  public:
    Country();
    Country(const std::vector <std::string> &allocations,
            const std::string names);
    bool initialize(const std::vector <std::string> &allocations,
                    const std::string names);
    operator bool() const;
    const std::string &error();
    const std::string &find(const uint32_t &ip);
  private:
    class _Country {
      public:
        _Country(const uint32_t &_lastIP, const std::string &_country);
        uint32_t lastIP;
        std::string country;
    };
    std::map <uint32_t, _Country> countryMap;
    std::map <uint32_t, _Country>::const_iterator countryItr;
    std::tr1::unordered_map <std::string, std::string> countryNames;
    std::tr1::unordered_map <std::string, std::string>::const_iterator countryNameItr;
    std::string empty;
    bool _error;
    std::string errorMessage;
};

#endif
