/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include "args.h"

#include <stdlib.h>

#include <iostream>
#include <stdexcept>

Args::Args() {
    batchSize = 10000; 
    K = 300; 
    indexKey = "hnswcustom";
    linksPerVector = 8; 
    efConstruction = 26; 
    efSearch = 64; 
}

std::string Args::boolToString(bool b) const {
  if (b) {
    return "true";
  } else {
    return "false";
  }
}

void Args::parseArgs(const std::vector<std::string>& args) {
  for (int ai = 1; ai < args.size(); ai += 2) {
    if (args[ai][0] != '-') {
      std::cerr << "Provided argument without a dash! Usage:" << std::endl;
      printHelp();
      exit(EXIT_FAILURE);
    }
    try {
      if (args[ai] == "-h") {
        std::cerr << "Here is the help! Usage:" << std::endl;
        printHelp();
        exit(EXIT_FAILURE);
      } else if (args[ai] == "-input") {
        input = std::string(args.at(ai + 1));
      } else if (args[ai] == "-query") {
        query = std::string(args.at(ai + 1));
      } else if (args[ai] == "-indexKey") {
        indexKey = std::string(args.at(ai + 1));
      } else if (args[ai] == "-indexPath") {
        indexPath = std::string(args.at(ai + 1));
      } else if (args[ai] == "-linksPerVector") {
        linksPerVector = std::stoi(args.at(ai + 1));
      } else if (args[ai] == "-efConstruction") {
        efConstruction = std::stoi(args.at(ai + 1));
      } else if (args[ai] == "-efSearch") {
        efSearch = std::stoi(args.at(ai + 1));
      } else if (args[ai] == "-batchSize") {
        batchSize = std::stoi(args.at(ai + 1));
      } else if (args[ai] == "-K") {
        K = std::stoi(args.at(ai + 1));
      } else if (args[ai] == "-output") {
        output = std::string(args.at(ai + 1));
      } 
  } catch (std::out_of_range) {
     std::cerr << args[ai] << " is missing an argument" << std::endl;
     printHelp();
     exit(EXIT_FAILURE);
   }
  }
  if (input.empty() || output.empty() || query.empty() || indexPath.empty()) {
    std::cerr << "Empty input, output, query, index" << std::endl;
    printHelp();
    exit(EXIT_FAILURE);
  }
}

void Args::dumpArgs() {
  std::cerr
    << "Initialized args:" << std::endl
    << "input: " << input << std::endl
    << "output: " << output << std::endl
    << "query: " << query << std::endl
    << "indexPath: " << indexPath << std::endl
    << "K: " << K << std::endl
    << "batchSize: " << batchSize << std::endl
    << "indexKey: " << indexKey << std::endl
    << "linksPerVector: " << linksPerVector << std::endl
    << "efConstruction: " << efConstruction << std::endl
    << "efSearch: " << efSearch << std::endl;
}

void Args::printHelp() {
  std::cerr
    << "\nThe following arguments are mandatory:\n"
    << "  -input              vecs in word2vec format\n"
    << "  -output             output nn path\n"
    << "  -query              query file path\n"
    << "  -indexPath          path to store index\n";

  std::cerr
    << "\nThe following arguments are optional:\n"
    << "  -K  number of neighbours to search [" << K << "]\n"
    << "  -batchSize size of query batch [" << batchSize << "]\n"
    << "  -indexKey  IndexFactory string or [" << indexKey << "]\n"
    << "  -linksPerVector  4 <= x <= 64 is the number of links per vector, higher is more accurate but uses more RAM [" << linksPerVector << "]\n"
    << "  -efConstruction  [" << efConstruction << "]\n"
    << "  -efSearch  speed accuracy tradeoff [" << efSearch << "]\n";
}

