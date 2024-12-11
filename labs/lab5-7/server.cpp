#include "send_recv.hpp"

std::optional<std::uint32_t> GetPort(std::vector<std::pair<std::uint32_t, bool>>& ports) {
  std::uint32_t ans = 0;
  for (auto& item : ports) {
    if (item.second) {
      item.second = false;
      ans = item.first;
      break;
    }
  }
  if (ans == 0) {
    return std::nullopt;
  }
  return {ans};
}

bool InServer(ServerNode& node, std::uint32_t id) {
  return node.node_id_.contains(id);
}

void InitPorts(std::vector<std::pair<std::uint32_t, bool>>& ports, std::uint32_t start_port) {
  for (auto& item : ports) {
    item.first = start_port++;
    item.second = true;
  }
}

bool ExecParse(ServerNode& server_node, std::string message) {
  std::stringstream ss(message);
  std::string exec;
  ss >> exec;
  std::uint32_t node_id = 0;
  ss >> node_id;
  if (InServer(server_node, node_id)) {
    Send(server_node.socket_root_.value(), std::move(message));
    return true;
  }
  return false;
}


void Init(ServerNode& server_node) {
  server_node.context_root_.emplace();
  server_node.ports_.resize(100, {0, true});
  InitPorts(server_node.ports_, 1070);
  server_node.node_id_.emplace(0);
}


bool Create(ServerNode& server_node, std::string message) {
  std::stringstream ss(message);
  std::string create;
  ss >> create;
  std::uint32_t node_id = 0;
  ss >> node_id;
  if (InServer(server_node, node_id)) {
    return false;
  }
  std::optional<std::uint32_t> port_id = GetPort(server_node.ports_);
  if (!port_id.has_value()) {
    return false;
  }
  server_node.node_id_.emplace(node_id);
  if (server_node.socket_root_.has_value()) {
    if (!Send(server_node.socket_root_.value(), message + ' ' + std::to_string(port_id.value()))) {
      server_node.socket_root_.reset();
      return Create(server_node, std::move(message));
    }
  } else {
    server_node.socket_root_.emplace(server_node.context_root_.value(), zmq::socket_type::pair);
    server_node.socket_root_.value().bind("tcp://*:" + std::to_string(port_id.value()));
    server_node.child_id = node_id;
    pid_t pid = fork();
    if (pid == 0) {
      std::string child_id = std::to_string(node_id);
      std::string port = std::to_string(port_id.value());
      execl("./client", child_id.c_str(), port.c_str());
    }
  }
  return true;
}

void AnalizeSenders(ServerNode& server_node, std::string message);

void ReceivQueue(ServerNode& server_node, std::vector<std::uint32_t>& availible_nodes) {
  while (true) {
    std::optional<std::string> message = Recv(server_node.socket_root_);
    if (message.has_value()) {
      std::stringstream ss(message.value());
      std::string command;
      ss >> command;
      if (command != "node_ping:") {
        AnalizeSenders(server_node, std::move(message.value()));
      } else {
        std::uint32_t node_id;
        ss >> node_id;
        availible_nodes.push_back(node_id);
      }
    } else {
      break;
    }
  }
}

std::vector<std::uint32_t> GetUnavailibleNodes(ServerNode& server_node, const std::vector<std::uint32_t>& avail_nodes) {
  std::vector<std::uint32_t> ans_;
  for (auto item : server_node.node_id_) {
    if (item == 0) continue;
    if (std::find(avail_nodes.begin(), avail_nodes.end(), item) == avail_nodes.end()) {
      ans_.push_back(item);
    }
  }
  return ans_;
}

void Destroy(ServerNode& server_node, const std::vector<std::uint32_t>& destroy_nodes) {
  for (auto item : destroy_nodes) {
    server_node.node_id_.erase(item);
  }
}

void CallDestroyResponse(ServerNode& server_node, const std::vector<std::uint32_t>& destroy_nodes) {
  for (auto item : destroy_nodes) {
    std::cout << item << ' ';
    if (!Send(server_node.socket_root_.value(), std::string("destroy ") + std::to_string(item))) {
      server_node.socket_root_.reset();
      server_node.child_id = 0;
    }
  }
}



void AnalizeSenders(ServerNode& server_node, std::string message) {
  std::stringstream ss(message);
  std::string command;
  ss >> command;
  if (command == std::string("node_ping:")) {
    if (server_node.check_ping_ == true) {
      std::uint32_t node_id;
      ss >> node_id;
      std::vector<std::uint32_t> availible_nodes;
      availible_nodes.push_back(node_id);
      ReceivQueue(server_node, availible_nodes);
      std::vector<std::uint32_t> unavailible_nodes_ = GetUnavailibleNodes(server_node, availible_nodes);
      Destroy(server_node, unavailible_nodes_);
      CallDestroyResponse(server_node, unavailible_nodes_);
      server_node.check_ping_ = false;
      std::cout << "Availible nodes: ";
      for (auto item : availible_nodes) {
        std::cout << item << ' ';
      }
      std::cout << std::endl;
    } else {} // ничего не делаем мы опоздали...
  } else {
    std::cout << message << std::endl;
  }

}

void AnalizeUser(ServerNode& server_node, std::string message) {
  std::stringstream ss(message);
  std::string command;
  ss >> command;
  if (command == std::string("create")) {
    if (!Create(server_node, std::move(message))) {
      std::cout << "Max limit or node already in system" << std::endl;
    }
  } else if (command == "exec") {
    if (!ExecParse(server_node, std::move(message))) {
      std::cout << "Node not in system!" << std::endl;
    }
  } else if (message == "ping all") {
    server_node.check_ping_ = true;
    if (!Send(server_node.socket_root_.value(), message)) {
      server_node.socket_root_.reset();
      server_node.child_id = 0;
      server_node.check_ping_ = false;
      server_node.node_id_.clear();
      server_node.node_id_.emplace(0);
      std::cout << "All broken " << std::endl;
      return;
    }
    std::cout << "Please wait. Nodes checking..." << std::endl;
    std::this_thread::sleep_for(1s);
  } else {
    std::cout << "Incorrect command" << std::endl;
  }
}


int main() {
  ServerNode server_node;
  Init(server_node);
  std::string user_command;
  while (true) {
    while (true) {
      std::optional<std::string> message = Recv(server_node.socket_root_);
      if (message.has_value()) {
        AnalizeSenders(server_node, std::move(message.value()));
      } else {
        break;
      }
    }
    std::getline(std::cin, user_command);
    AnalizeUser(server_node, std::move(user_command));
  }
}
