#include <sys/wait.h>
#include <iostream>
#include <deque>
#include <vector>
#include <fstream>

void child_process(int read_fd) {
  std::vector<int> numbers = {1};
  int i = 0;
  while (read(read_fd, &numbers[i++], sizeof(int))) {
    numbers.push_back(1);
  }
  std::fstream file("results.txt", std::ios::out | std::ios::trunc);
  if (!file) {
    std::cerr << "cannot open file" << std::endl;
  }
  int result = numbers[0];
  for (int i = 1; i < numbers.size(); ++i) {
    if (numbers[i] == 0) {
      std::cerr << "Zero error" << std::endl;
      file.close();
      exit(1);
    } else {
      result /= numbers[i];
    }
  }
  file << result;
  file.close();
  exit(0);
}

int main() {
  int pipe_fd[2];
  pid_t pid;
  if (pipe(pipe_fd) == -1) {
    std::cerr << "Error creating" << std::endl;
    return -1;
  }
  pid = fork();
  if (pid < 0) {
    std::cerr << "Fork error" << std::endl;
    exit(1);
  } else if (pid > 0) {
    close(pipe_fd[0]);
    std::deque<int> dq;
    int n = 0;
    while (true) {
      std::cin >> n;
      if (std::cin.fail()) {
        break;
      }
      dq.push_back(n);
      write(pipe_fd[1], &(dq.back()), sizeof(int));
    }
    close(pipe_fd[1]);
    wait(NULL);
    
  } else {
    close(pipe_fd[1]);
    child_process(pipe_fd[0]);
  }
  return 0;
}
