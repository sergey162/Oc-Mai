#include <sys/wait.h>
#include <iostream>

auto main() -> int {
  int pipe1[2], pipe2[2];
  if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
    std::cerr << "Error" << std::endl;
    exit(1);
  }
  pid_t pid = fork();
  if (pid == 0) {
    //child
    (close(pipe1[1]), close(pipe2[0])); // closing channels
    (dup2(pipe1[0], STDIN_FILENO), dup2(pipe2[1], STDOUT_FILENO));
    execl("./b", "child", nullptr); // replace current process
    exit(1);
  } else if (pid == -1) {
    // Error 
    std::cerr << "Error" << std::endl;
    exit(1);
  } else {
    // parent
    (close(pipe1[0]), close(pipe2[1])); // closing channels
    std::string name_file;
    std::getline(std::cin, name_file);
    name_file += '\n'; // for correct childs reading
    size_t str_size = name_file.size();

    write(pipe1[1], name_file.c_str(), name_file.size());
    std::string numbers;
    std::getline(std::cin, numbers);
    numbers += '\n';
    write(pipe1[1], numbers.c_str(), numbers.size());
    close(pipe1[1]);
    std::string status_msg = "1"; // 1 - it s ok 0 - was divided zero
    read(pipe2[0], status_msg.data(), sizeof(char));
    if (status_msg[0] - '0' == 0) {
      std::cerr << "Devided zero" << std::endl;
    }
    close(pipe2[0]);
    wait(nullptr); // wait child
  }
  return 0;
}
