#include "server.h"

#include <iostream>
#include <random>

#include "client.h"

Server::Server() {}

namespace {
int32_t generateRandom() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1000, 9999);
    return dist(gen);
}
}  // namespace

std::shared_ptr<Client> Server::add_client(std::string id) {
    if (get_client(id) != nullptr) {
        id += std::to_string(generateRandom());
    }
    auto client = std::make_shared<Client>(id, *this);

    clients[client] = 5.0;
    return client;
}

std::shared_ptr<Client> Server::get_client(std::string id) {
    for (auto& pair : clients) {
        if (pair.first->get_id() == id) {
            return pair.first;
        }
    }
    return nullptr;
}

const std::map<std::shared_ptr<Client>, double>& Server::get_clients() const {
    return clients;
}

double Server::get_wallet(std::string id) { return clients[get_client(id)]; }
