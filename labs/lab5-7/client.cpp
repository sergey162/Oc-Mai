#include "send_recv.hpp"


void Init(std::uint32_t node_id, std::uint32_t port_id, ClientNode& client_node) {
  client_node.id = node_id;
  client_node.context_client_.emplace();
  client_node.socket_parent_.emplace(client_node.context_client_.value(), zmq::socket_type::pair);
  client_node.socket_parent_.value().connect(std::string("tcp://localhost:") + std::to_string(port_id));
}

void Append(std::optional<zmq::socket_t>& node, zmq::context_t& context, std::optional<std::uint32_t>& setid, std::string message) {
  std::stringstream ss(message);
  std::string create;
  ss >> create;
  std::uint32_t node_id;
  ss >> node_id;
  std::uint32_t port_id;
  ss >> port_id;
  if (node.has_value()) {
    if (!Send(node.value(), message)) {
      node.reset();
      Append(node, context, setid, message);
    }
  } else {
    node.emplace(context, zmq::socket_type::pair);
    node.value().bind("tcp://*:" + std::to_string(port_id));
    setid.emplace(node_id);
    pid_t pid = fork();
    if (pid == 0) {
      std::string child_id = std::to_string(node_id);
      std::string port = std::to_string(port_id);
      execl("./client", child_id.c_str(), port.c_str());
    }
  }
}

void Create(ClientNode& client_node, std::string message) {
  std::stringstream ss(message);
  std::string create;
  ss >> create;
  std::uint32_t node_id;
  ss >> node_id;
  if (node_id < client_node.id) {
    Append(client_node.socket_left_, client_node.context_client_.value(), client_node.left_id_, std::move(message));
  } else {
    Append(client_node.socket_right_, client_node.context_client_.value(), client_node.right_id_, std::move(message));
  }
}

void Calculate(ClientNode& client_node, std::string message) {
  std::stringstream ss(message);
  std::string exec;
  ss >> exec;
  std::uint32_t node_id;
  ss >> node_id;
  if (node_id < client_node.id) {
    Send(client_node.socket_left_.value(), std::move(message));
  } else if (node_id > client_node.id) {
    Send(client_node.socket_right_.value(), std::move(message));
  } else {
    std::int32_t ans = 0;
    std::int32_t number = 0;
    while (ss >> number) {
      ans += number;
    }
    Send(client_node.socket_parent_.value(), std::string("OK:") + std::to_string(node_id) + std::string(": ") + std::to_string(ans));
  }
}

void DestroyNode(ClientNode& client_node, std::string message) {
  std::stringstream ss(message);
  std::string destroy;
  ss >> destroy;
  std::uint32_t node_id;
  ss >> node_id;
  if (node_id < client_node.id) {
    if (client_node.left_id_.has_value()) {
      if (client_node.left_id_.value() == node_id) {
        client_node.left_id_.reset();
        client_node.socket_left_.reset();
      } else {
        if (!Send(client_node.socket_left_.value(), std::move(message))) {
          client_node.left_id_.reset();
          client_node.socket_left_.reset();
        }
      }
    } else {} // уже была удалена
  } else if (node_id > client_node.id) {
    if (client_node.right_id_.has_value()) {
      if (client_node.right_id_.value() == node_id) {
        client_node.right_id_.reset();
        client_node.socket_right_.reset();
      } else {
        if (!Send(client_node.socket_right_.value(), std::move(message))) {
          client_node.right_id_.reset();
          client_node.socket_right_.reset();
        }
      }
    } else {} // значит была кем то удалена
  }
}



void ParseAndMakeAction(ClientNode& client_node, std::string message) {
  std::stringstream ss(message);
  std::string command;
  ss >> command;
  if (command == "create") {
    Create(client_node, std::move(message));
  } else if (command == "exec") {
    Calculate(client_node, std::move(message));
  } else if (message == "ping all") {
    Send(client_node.socket_parent_.value(), std::string("node_ping: ") + std::to_string(client_node.id));
    if (client_node.socket_left_.has_value()) {
      if (!Send(client_node.socket_left_.value(), message)) {
        client_node.socket_left_.reset();
        client_node.socket_right_.reset();
      }
    } 
    if (client_node.socket_right_.has_value()) {
      if (!Send(client_node.socket_right_.value(), message)) {
        client_node.socket_right_.reset();
        client_node.right_id_.reset();
      }
    }
  } else if (command == "destroy") {
    DestroyNode(client_node, std::move(message));
  } else {
    Send(client_node.socket_parent_.value(), std::move(message));
  }
}


int main(int argc, char* argv[]) {
  ClientNode client_node;
  pid_t pid = getpid();
  std::uint32_t node_id = std::stoul(argv[0]);
  std::uint32_t port_id = std::stoul(argv[1]);
  Init(node_id, port_id, client_node);
  Send(client_node.socket_parent_.value(), std::string("make node with id: ") + std::to_string(node_id) + std::string(", pid: ") + std::to_string(pid));
  while (true) {
    while (true) {
      std::optional<std::string> message = Recv(client_node.socket_parent_);
      if (message.has_value()) {
        ParseAndMakeAction(client_node, std::move(message.value()));
      } else {
        break;
      }
    }
    while (true) {
      std::optional<std::string> message = Recv(client_node.socket_left_);
      if (message.has_value()) {
        ParseAndMakeAction(client_node, std::move(message.value()));
      } else {
        break;
      }
    }
    while (true) {
      std::optional<std::string> message = Recv(client_node.socket_right_);
      if (message.has_value()) {
        ParseAndMakeAction(client_node, std::move(message.value()));
      } else {
        break;
      }
    }
    std::this_thread::sleep_for(0.1s);
  }
}
