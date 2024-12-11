#pragma once

#include <zmq.hpp>
#include <iostream>
#include <vector>
#include <cstdint>
#include <optional>
#include <sstream>
#include <set>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <algorithm>

using namespace std::chrono_literals;


struct ClientNode {
  std::uint32_t id = 0;
  std::optional<std::uint32_t> left_id_;
  std::optional<std::uint32_t> right_id_;
  std::optional<zmq::context_t> context_client_;
  std::optional<zmq::socket_t> socket_parent_;
  std::optional<zmq::socket_t> socket_left_;
  std::optional<zmq::socket_t> socket_right_;
};


struct ServerNode {
  bool check_ping_ = false;
  std::uint32_t id = 0;
  std::uint32_t child_id = 0;
  std::optional<zmq::socket_t> socket_root_;
  std::optional<zmq::context_t> context_root_;
  std::vector<std::pair<std::uint32_t, bool>> ports_;
  std::set<std::uint32_t> node_id_;
};

bool Send(zmq::socket_t& receiver, std::string message) {
  zmq::message_t request(message.size());
  std::memcpy(request.data(), message.c_str(), message.size());
  if (receiver.send(request, zmq::send_flags::dontwait).has_value()) {
    return true;
  }
  return false;
}


std::optional<std::string> Recv(std::optional<zmq::socket_t>& sender) {
  if (sender.has_value()) {
    zmq::message_t message;
    std::optional<size_t> count_bytes = sender.value().recv(message, zmq::recv_flags::dontwait);
    if (count_bytes.has_value()) {
      return {std::string(static_cast<char*>(message.data()), message.size())};
    }
  }
  return std::nullopt;
}
