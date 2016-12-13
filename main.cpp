#include <iostream>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <vector>
#include <thread>
#include <fstream>
#include "sha1.hpp"

bool exit_flag = false;
constexpr const size_t brute_force = 7;
constexpr const size_t chain_size = 1000;

std::string reduce(std::string &hash, size_t n) {
  std::string digits_from_hash;
  std::copy_if(hash.begin(), hash.end(), std::back_inserter(digits_from_hash), isdigit);
  std::string result(digits_from_hash);
  while (result.length() < n) {
    result += digits_from_hash;
  }
  return result.substr(0, n);
}

void rainbows_and_unicorns(uint64_t n, std::vector<std::pair<std::string, std::string>> &vec){
  SHA1 sha;
  if (n<brute_force){
    uint64_t n_max = pow(10, n);
    for(uint64_t i = (n==1) ? 0 : pow(10, n-1); i< n_max && !exit_flag; i++){
      std::string i_str = std::to_string(i);
      sha.update(i_str);
      vec.push_back({sha.final(), i_str});
    }
  } else {

  }
  std::cout << "done " << n << std::endl;
}

void create_m(int n, std::string filename) {
  std::vector<std::vector<std::pair<std::string, std::string> >> top_map;
  for (int i = 0; i < n; i++) {
    top_map.push_back(std::vector<std::pair<std::string, std::string>>());
  }

  std::vector<std::thread> threads;
  for (int i = 0; i < n; i++){
    threads.push_back(std::thread(rainbows_and_unicorns, i+1, std::ref(top_map[i])));
  }

  while(getchar() != 'q');
  exit_flag = true;

  for(auto &thread : threads){
    thread.join();
  }

  std::ofstream file(filename);
  if(!file.is_open()){
    throw std::runtime_error("cannot open file with such name");
  }

  file << n << '\n' << '\n';
  for(auto &vec : top_map){
    std::sort(vec.begin(), vec.end());
    for(auto &pair : vec){
      file << pair.first << ',' << pair.second << '\n';
    }
    file << '\n';
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    throw std::invalid_argument("wrong number of argvs, expecting 3");
  }
  std::string mode(argv[1]);
  if (mode == "--create") {
    int num = std::stoi(argv[2]);
    if (num < 1) {
      throw std::invalid_argument("shortest password size must be greater than or equal to 1");
    }
    create_m(num, argv[3]);
  } else if (mode == "--search") {

  } else {
    throw std::invalid_argument(std::string("unknown switch option ") + argv[1]);
  }
  return 0;
}