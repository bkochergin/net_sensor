/*
 * Copyright 2007-2015 Boris Kochergin. All rights reserved.
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

#include "configuration.h"

Configuration::Configuration(const std::string filename) {
  initialize(filename);
}

Configuration::Configuration() {
  error_ = true;
  error_message_ = "Configuration::Configuration(): class not initialized";
}

bool Configuration::initialize(const std::string filename) {
  filename_ = filename;
  std::ifstream file(filename_.c_str());
  if (!file) {
    error_ = true;
    error_message_ = "Configuration::initialize(): " + filename + ": " +
                     strerror(errno);
    return false;
  }
  /* Allow initialize() to be called multiple times. */
  options_.clear();
  std::string line;
  while (getline(file, line)) {
    const size_t delimiter = line.find('=');
    const std::string option = line.substr(0, delimiter);
    std::string value = line.substr(delimiter + 1);
    const size_t openingQuote = value.find('"');
    if (openingQuote != std::string::npos) {
      const size_t closingQuote = value.find('"', openingQuote + 1);
      if (closingQuote != std::string::npos) {
        value = value.substr(openingQuote + 1,
                             closingQuote - openingQuote - 1);
      }
    }    
    options_[option].push_back(value);
  }
  file.close();
  error_ = false;
  error_message_.clear();
  return true;
}

Configuration::operator bool() const {
  return !error_;
}

const std::string& Configuration::error() const {
  return error_message_;
}

const std::string& Configuration::filename() const {
  return filename_;
}

const std::string Configuration::getString(const std::string option) const {
  auto option_itr = options_.find(option);
  if (option_itr == options_.end()) {
    return "";
  }
  return (option_itr->second)[0];
}

const std::vector<std::string>& Configuration::getStrings(
    const std::string option) const {
  auto option_itr = options_.find(option);
  return option_itr->second;
}

size_t Configuration::getNumber(const std::string option) const {
  auto option_itr = options_.find(option);
  if (option_itr == options_.end()) {
    return std::numeric_limits<size_t>::max();
  }
  return strtoul((option_itr->second)[0].c_str(), nullptr, 10);
}
