#include "gcf/gcf.hpp"
#include "simple_numbers/simple_numbers.hpp"
#include <iostream>

enum class Commands {Change, First, Second, Exit};

std::istream& operator>>(std::istream& in, Commands& command) {
  std::int32_t number = 0;
  in >> number;
  command = static_cast<Commands>(number);
  return in;
}

enum class FunctionVersions {GSF, SimpleNumbers};

auto main() -> int {
  Commands current_command = Commands::Change;
  Commands current_version = Commands::First;
  FunctionVersions func = FunctionVersions::GSF;
  while (true) {
    std::cin >> current_command;
    std::int32_t first = 0, second = 0;
    if (current_command == Commands::Exit) {
      std::cout << "Program finished!" << std::endl;
      break;
    } else if (current_command == Commands::Change) {
      func = func == FunctionVersions::GSF ? FunctionVersions::SimpleNumbers : FunctionVersions::GSF;
    } else if (current_command == Commands::First) {
      std::cin >> first >> second;
      if (func == FunctionVersions::GSF) {
        std::cout << FastGCF(first, second) << std::endl;
      } else {
        std::cout << FastCountSimpleNumbers(first, second) << std::endl;
      }
    } else {
      std::cin >> first >> second;
      if (func == FunctionVersions::GSF) {
        std::cout << BadGCF(first, second) << std::endl;
      } else {
        std::cout << BadCountSimpleNumbers(first, second) << std::endl;
      }
    }
  }
  return 0;
}
