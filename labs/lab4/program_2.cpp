#include <iostream>
#include <dlfcn.h>
#include <cstdint>
#include <cstddef>

static std::string gcf_lib_path = "./libs/libgcf.so";
static std::string simple_numbers_lib_path = "./libs/libsimplenumbers.so";
using FunctionType = std::int32_t(*)(std::int32_t, std::int32_t);
enum class Commands {Change, First, Second, Exit};
std::istream& operator>>(std::istream& in, Commands& command) {
  std::int32_t number = 0;
  in >> number;
  command = static_cast<Commands>(number);
  return in;
}
enum class FunctionVersions {GSF, SimpleNumbers};
auto main() -> int {
  void* gcf_lib = dlopen(gcf_lib_path.c_str(), RTLD_LAZY);
  void* simple_numbers_lib = dlopen(simple_numbers_lib_path.c_str(), RTLD_LAZY);
  if (gcf_lib == nullptr || simple_numbers_lib == nullptr) {
    std::cerr << "Cant load library" << std::endl;
    return 1;
  }
  Commands current_command = Commands::Change;
  Commands current_version = Commands::First;
  FunctionVersions func = FunctionVersions::GSF;
  while (true) {
    std::cin >> current_command;
    std::int32_t first = 0, second = 0;
    if (current_command == Commands::Exit) {
      std::cout << "Program finished!" << std::endl;
      dlclose(gcf_lib);
      dlclose(simple_numbers_lib);
      break;
    } else if (current_command == Commands::Change) {
      func = func == FunctionVersions::GSF ? FunctionVersions::SimpleNumbers : FunctionVersions::GSF;
    } else if (current_command == Commands::First) {
      std::cin >> first >> second;
      if (func == FunctionVersions::GSF) {
        std::cout << ((FunctionType)(dlsym(gcf_lib, "FastGCF")))(first, second) << std::endl;
      } else {
        std::cout << ((FunctionType)(dlsym(simple_numbers_lib, "FastCountSimpleNumbers")))(first, second) << std::endl;
      }
    } else {
      std::cin >> first >> second;
      if (func == FunctionVersions::GSF) {
        std::cout << ((FunctionType)(dlsym(gcf_lib, "BadGCF")))(first, second) << std::endl;
      } else {
        std::cout << ((FunctionType)(dlsym(simple_numbers_lib, "BadCountSimpleNumbers")))(first, second) << std::endl;
      }
    }
  }
  return 0;
}
