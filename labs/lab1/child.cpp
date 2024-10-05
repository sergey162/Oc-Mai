#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <sstream>


auto main() -> int {
  std::string file_name, numbers;
  std::getline(std::cin, file_name, '\n');
  std::fstream file(file_name, std::ios::out | std::ios::trunc);
  std::getline(std::cin, numbers, '\n');
  std::istringstream iss(numbers);
  int result = 0, num = 0;
  iss >> result;
  bool ok = true;
  while (iss >> num) {
    if (num == 0) {
      ok = false;
      std::cout << ok << std::endl;
      file.close();
      exit(1);
    }
    result /= num;
  }
  file << result; // write in file 
  std::cout << ok << std::endl;
  file.close();
  exit(0);
}
