#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

static inline constexpr size_t kBufferSize = 1024u; 

int main(int argc, char* argv[]) {
  std::string file_name(static_cast<char*>(argv[0]));
  int fd1 = shm_open("/shared_memory", O_RDWR, 0666);
  char* shared_memory = static_cast<char*>(mmap64(nullptr, kBufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0));
  std::string numbers(shared_memory);
  std::istringstream iss(numbers);
  int result = 0, num = 0;
  iss >> result;
  while (iss >> num) {
    if (num == 0) {
      shared_memory[0] = '0';
      close(fd1);
      munmap(shared_memory, kBufferSize);
      return 1;
    }
    result /= num;
  }
  std::fstream file(file_name, std::ios::out | std::ios::trunc);
  file << result; // write in file
  shared_memory[0] = '1';
  file.close();
  close(fd1);
  munmap(shared_memory, kBufferSize);
  return 0;
}
