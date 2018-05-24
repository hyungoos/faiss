/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <vector>


class Args {
  protected:
    std::string boolToString(bool) const;

  public:
    Args();
    std::string input;
    std::string output;
    std::string query;
    std::string indexKey;
    std::string indexPath;
    int K;
    size_t batchSize;
    // these are specific to hsfw
    int linksPerVector; 
    int efConstruction; 
    int efSearch; 


    void parseArgs(const std::vector<std::string>& args);
    void dumpArgs();
    void printHelp();
};
