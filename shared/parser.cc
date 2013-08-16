/*
 * Copyright 2013 Stanford University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the copyright holders nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /*
  * Parser for Nimbus scheduler protocol. 
  *
  * Author: Omid Mashayekhi <omidm@stanford.edu>
  */

#include "shared/parser.h"

namespace nimbus {
void parseCommand(const std::string& str, const CmSet& cms,
                  std::string& cm, std::vector<int>& args) {
  cm.clear();
  args.clear();
  std::set<std::string>::iterator it;
  for (it = cms.begin(); it != cms.end(); ++it) {
    if (str.find(*it) == 0) {
      cm = *it;
      int arg;
      std::stringstream ss;
      ss << str.substr(it->length(), std::string::npos);
      while (true) {
        ss >> arg;
        if (ss.fail()) {
          break;
        }
        args.push_back(arg);
      }
      break;
    }
  }
  if (cm == "") {
    std::cout << "wrong command! try again." << std::endl;
  }
}


int parseCommandFile(const std::string& fname, CmSet& cs) {
  return 0;
}

using boost::tokenizer;
using boost::char_separator;


bool parseCommandFromString(const std::string& input,
                            std::string& command,
                            std::vector<std::string>& parameters) {
  char_separator<char> separator(" \n\t\r");
  tokenizer<char_separator<char> > tokens(input, separator);
  tokenizer<char_separator<char> >::iterator iter = tokens.begin();
  if (iter == tokens.end()) {
    command = "";
    return false;
  }

  command = *iter;
  ++iter;
  for (; iter != tokens.end(); ++iter) {
    parameters.push_back(*iter);
  }
  return true;
}

void parseParameterFromString(const std::string& input, std::string& tag,
    std::string& args, std::string& string_set) {
  char_separator<char> separator(":");
  tokenizer<char_separator<char> > tokens(input, separator);
  tokenizer<char_separator<char> >::iterator iter = tokens.begin();
  if (iter == tokens.end()) {
    tag = "";
    return;
  }

  tag = *iter;
  iter++;
  if (isSet(*iter))
    string_set = *iter;
  else
    args = *iter;
}

void parseIDSetFromString(const std::string& input, std::set<uint64_t>& set) {
  int num;
  std::string str = input.substr(1, input.length() - 2);
  char_separator<char> separator(",");
  tokenizer<char_separator<char> > tokens(str, separator);
  tokenizer<char_separator<char> >::iterator iter = tokens.begin();
  for (; iter != tokens.end(); ++iter) {
    std::stringstream ss(*iter);
    ss >> num;
    set.insert(num);
  }
}

void parseIDSetFromString(const std::string& input, std::set<uint32_t>& set) {
  int num;
  std::string str = input.substr(1, input.length() - 2);
  char_separator<char> separator(",");
  tokenizer<char_separator<char> > tokens(str, separator);
  tokenizer<char_separator<char> >::iterator iter = tokens.begin();
  for (; iter != tokens.end(); ++iter) {
    std::stringstream ss(*iter);
    ss >> num;
    set.insert(num);
  }
}

int countOccurence(std::string str, std::string substr) {
  int count = 0;
  std::size_t pos = -1;

  while (true) {
    pos = str.find(substr, pos + 1);
    if (pos != std::string::npos)
      count++;
    else
      break;
  }
  return count;
}


bool isSet(const std::string& str) {
  if (str.find("{") == std::string::npos)
    return false;
  return true;
}

// ********************************************************

bool ParseSpawnJobCommand(const std::string& input,
    std::string& job_name,
    IDSet<job_id_t>& job_id,
    IDSet<data_id_t>& read, IDSet<data_id_t>& write,
    IDSet<job_id_t>& before, IDSet<job_id_t>& after,
    JobType& job_type, std::string& params) {
  int num = 8;
  std::set<job_id_t> job_id_set;
  std::set<data_id_t> data_id_set;

  char_separator<char> separator(" \n\t\r");
  tokenizer<char_separator<char> > tokens(input, separator);
  tokenizer<char_separator<char> >::iterator iter = tokens.begin();
  for (int i = 0; i < num; i++) {
    if (iter == tokens.end()) {
      std::cout << "ERROR: SpawnJobCommand has only " << i <<
        "parameters (expected " << num << ")." << std::endl;
      return false;
    }
    iter++;
  }
  if (iter != tokens.end()) {
    std::cout << "ERROR: SpawnJobCommand has more than "<<
      num << " parameters." << std::endl;
    return false;
  }

  iter = tokens.begin();
  job_name = *iter;

  iter++;
  if (ParseIDSet(*iter, job_id_set)) {
    IDSet<data_id_t> temp(job_id_set);
    job_id = temp;
  } else {
    std::cout << "ERROR: job id set is not well formatted" << std::endl;
    return false;
  }

  iter++;
  if (ParseIDSet(*iter, data_id_set)) {
    IDSet<data_id_t> temp(data_id_set);
    read = temp;
  } else {
    std::cout << "ERROR: read set is not well formatted" << std::endl;
    return false;
  }

  iter++;
  if (ParseIDSet(*iter, data_id_set)) {
    IDSet<data_id_t> temp(data_id_set);
    write = temp;
  } else {
    std::cout << "ERROR: write set is not well formatted" << std::endl;
    return false;
  }

  iter++;
  if (ParseIDSet(*iter, job_id_set)) {
    IDSet<data_id_t> temp(job_id_set);
    before = temp;
  } else {
    std::cout << "ERROR: before set is not well formatted" << std::endl;
    return false;
  }

  iter++;
  if (ParseIDSet(*iter, job_id_set)) {
    IDSet<data_id_t> temp(job_id_set);
    after = temp;
  } else {
    std::cout << "ERROR: after set is not well formatted" << std::endl;
    return false;
  }

  iter++;
  if (*iter == "COMP") {
    job_type = JOB_COMP;
  } else if (*iter == "SYNC") {
    job_type = JOB_SYNC;
  } else {
    std::cout << "ERROR: job type is not known!" << std::endl;
    return false;
  }

  iter++;
  params = *iter;

  return true;
}


bool ParseIDSet(const std::string& input, std::set<uint64_t>& set) {
  uint64_t num;
  set.clear();
  if (input[0] != '{' || input[input.length() - 1] != '}') {
    std::cout << "ERROR: wrong format for IDSet." << std::endl;
    return false;
  }
  std::string str = input.substr(1, input.length() - 2);
  char_separator<char> separator(",");
  tokenizer<char_separator<char> > tokens(str, separator);
  tokenizer<char_separator<char> >::iterator iter = tokens.begin();
  for (; iter != tokens.end(); ++iter) {
    std::stringstream ss(*iter);
    ss >> num;
    if (ss.fail()) {
      std::cout << "ERROR: wrong element in IDSet." << std::endl;
      return false;
    }
    set.insert(num);
  }
  return true;
}

bool ParseIDSet(const std::string& input, std::set<uint32_t>& set) {
  uint64_t num;
  set.clear();
  if (input[0] != '{' || input[input.length() - 1] != '}') {
    std::cout << "ERROR: wrong format for IDSet." << std::endl;
    return false;
  }
  std::string str = input.substr(1, input.length() - 2);
  char_separator<char> separator(",");
  tokenizer<char_separator<char> > tokens(str, separator);
  tokenizer<char_separator<char> >::iterator iter = tokens.begin();
  for (; iter != tokens.end(); ++iter) {
    std::stringstream ss(*iter);
    ss >> num;
    if (ss.fail()) {
      std::cout << "ERROR: wrong element in IDSet." << std::endl;
      return false;
    }
    set.insert(num);
  }
  return true;
}










}  // namespace nimbus
