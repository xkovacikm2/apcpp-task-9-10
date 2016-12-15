#include <iostream>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <vector>
#include <thread>
#include <fstream>
#include <atomic>
#include <cstdlib>
#include <mutex>
#include <iterator>
#include "sha1.hpp"

bool exit_flag = false;
constexpr const size_t brute_force = 8;
constexpr const size_t chain_size = 1000;

std::mutex mtx;

std::atomic_bool found(false);
std::string found_unicorn;

std::string reduce(std::string hash, uint64_t n) {
  std::string digits_from_hash;
  std::copy_if(hash.begin(), hash.end(), std::back_inserter(digits_from_hash), isdigit);
  std::string result(digits_from_hash);
  while (result.length() < n) {
    result += digits_from_hash;
  }
  return result.substr(0, static_cast<unsigned int>(n));
}

void rainbows_and_unicorns(uint64_t n, std::vector<std::pair<std::string, std::string>> &vec) {
  if (n < brute_force) {
    uint64_t n_max = static_cast<uint64_t>(pow(10, n));
    for (uint64_t i = (n == 1) ? 0 : static_cast<uint64_t>(pow(10, n - 1)); i < n_max && !exit_flag; i++) {
      auto i_str = std::to_string(i);
      SHA1 sha;
      sha.update(i_str);
      vec.push_back({sha.final(), i_str});
    }
  } else {
    uint64_t repeats = static_cast<uint64_t>(pow(10, n)) / chain_size;
    for (uint64_t i = 0; i < repeats && !exit_flag; i++) {
      std::string pass;
      for (size_t j = 0; j < n; j++) {
        pass += std::to_string(rand() % 10);
      }
      auto tmp = pass;
      std::string hash;
      for (uint64_t j = 0; j < chain_size; j++) {
        SHA1 sha;
        sha.update(tmp);
        hash = sha.final();
        tmp = reduce(hash, n);
      }
      vec.push_back({hash, pass});
    }
  }
  std::cout << "done " << n << std::endl;
}

void quit_on_q() {
  while (getchar() != 'q');
  exit_flag = true;
  std::cout << "acknowledged, initiating serialization" << std::endl;
}

void create_m(size_t n, std::string filename) {
  std::vector<std::vector<std::pair<std::string, std::string> >> top_map;
  for (size_t i = 0; i < n; i++) {
    top_map.push_back(std::vector<std::pair<std::string, std::string>>());
  }

  std::vector<std::thread> threads;
  for (size_t i = 0; i < n; i++) {
    threads.push_back(std::thread(rainbows_and_unicorns, i + 1, std::ref(top_map[i])));
  }

  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("cannot open file with such name");
  }

  std::thread kek(quit_on_q);

  file << n << '\n' << '\n';

  for (size_t i = 0; i<n; i++) {
    threads[i].join();
    std::sort(top_map[i].begin(), top_map[i].end());
    for (auto &pair : top_map[i]) {
      file << pair.first << ',' << pair.second << '\n';
    }
    file << '\n';
  }
  kek.join();
}

void set_unicorn(std::string &unicorn){
  std::lock_guard<std::mutex> guard(mtx);
  if(!found) {
    found_unicorn = unicorn;
    found = true;
  }
}

void find_unicorn_in_rainbows(size_t n, std::vector<std::pair<std::string, std::string>> &rainbows, std::string &hash){
  if(n < brute_force){
    auto iter = std::lower_bound(rainbows.begin(), rainbows.end(), std::pair<std::string, std::string>(hash, ""));
    if(iter != rainbows.end() && iter->first == hash){
      set_unicorn(iter->second);
    }
  }
  else{
    std::pair<std::string, std::string> hash_pair(hash, "");
    for(size_t i = 0; i < chain_size && !found; i++){
      auto iter = std::lower_bound(rainbows.begin(), rainbows.end(), hash_pair);
      if(iter != rainbows.end() && iter->first == hash_pair.first){
        std::string number = iter->second;
        SHA1 sha1;
        sha1.update(number);
        std::string chain_hash = sha1.final();
        for(size_t j = 0; j < chain_size - (i + 1); j++){
          number = reduce(chain_hash, n);
          sha1.update(number);
          chain_hash = sha1.final();
        }
        if(chain_hash == hash){
          set_unicorn(number);
        }
      }
      SHA1 sha;
      sha.update(reduce(hash_pair.first, n));
      hash_pair.first = sha.final();
    }
  }
}

void find_hash_b(std::ifstream &input_file, std::string hash){
  std::string line;
  std::getline(input_file, line);
  int n = std::stoi(line);
  if(n <= 0)
    throw std::runtime_error("Bad number in file");
  std::getline(input_file, line);
  std::vector<std::vector<std::pair<std::string, std::string>>> rainbow;
  for(int i = 0; i < n; i++)
    rainbow.push_back(std::vector<std::pair<std::string, std::string>>());
  int digit_count = 1;
  while(std::getline(input_file, line)){
    if(line == std::string()){
      digit_count++;
      continue;
    }
    auto comma_position = line.find(",");
    rainbow[digit_count-1].push_back({line.substr(0, comma_position), line.substr(comma_position + 1, line.length() - comma_position)});
  }

  std::vector<std::thread> threads;
  for (int i = 0; i < n; i++){
    threads.push_back(std::thread(find_unicorn_in_rainbows, i+1, std::ref(rainbow[i]), std::ref(hash)));
  }

  for(auto &thread : threads){
    thread.join();
  }

  if(found)
    std::cout << found_unicorn << std::endl;
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
    std::ifstream input_file(argv[3]);
    if(!input_file.is_open()){
      throw std::runtime_error(std::string("Could not open file ") + argv[3]);
    }
    find_hash_b(input_file, argv[2]);
  } else {
    throw std::invalid_argument(std::string("unknown switch option ") + argv[1]);
  }
  return 0;
}