#include <iostream>
#include <sstream>
#include <algorithm>
#include "sha1.hpp"

std::string reduce(std::string &hash, size_t n){
  std::string digits_from_hash;
  std::copy_if(hash.begin(), hash.end(), std::back_inserter(digits_from_hash), isdigit);
  std::string result(digits_from_hash);
  while(result.length() < n){
    result += digits_from_hash;
  }
  return result.substr(0,n);
}

int main(int argc, char *argv[]) {
  if(argc != 4){
    throw std::invalid_argument("wrong number of argvs, expecting 3");
  }

  std::string mode(argv[1]);
  if(mode == "--create"){

  } else if(mode == "--search"){

  } else {
    throw std::invalid_argument(std::string("unknown switch option ") + argv[1]);
  }
  return 0;
}