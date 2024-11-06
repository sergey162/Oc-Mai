#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <sys/wait.h>

static inline constexpr size_t kBufferSize = 1024u; 

int main() {
  int fd1 = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
  if (fd1 == -1) {
    std::cerr << "Error open shared memory" << std::endl;
    return 1;
  }
  ftruncate(fd1, kBufferSize);
  char* shared_memory = static_cast<char*>(mmap64(nullptr, kBufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0));
  std::string file_name, numbers;
  std::getline(std::cin, file_name); // get file_name
  std::getline(std::cin, numbers); // get numbers
  std::memcpy(shared_memory, numbers.c_str(), numbers.size() + 1); // write in shared memory numbers array
  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "Error creating child process" << std::endl;
    close(fd1);
    munmap(shared_memory, kBufferSize);
    return 1;
  }
  if (pid == 0) { // child
    execl("./child", file_name.c_str());
    std::cerr << "Error executing child process" << std::endl;
    return 1;
  } else { // parent
    wait(nullptr);
    if (shared_memory[0] == '0') {
      std::cerr << "Devided zero" << std::endl;
    }
    close(fd1);
    munmap(shared_memory, kBufferSize);
  }
  return 0;
}
