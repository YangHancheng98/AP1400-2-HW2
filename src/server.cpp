#include "server.h"

#include <iostream>
#include <random>
#include <string>

#include "client.h"
#include "crypto.h"

Server::Server() {}

namespace {
int32_t generateRandom() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1000, 9999);
    return dist(gen);
}

std::vector<std::string> split(std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}

bool is_valid_hash(const std::string& hash) {
    for (int i = 0; i < 7; i++) {
        if (hash.substr(i, i + 3) == "000") {
            return true;
        }
    }
    return false;
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

bool Server::parse_trx(std::string trx, std::string& sender,
                       std::string& receiver, double& value) {
    auto result = split(trx, "-");
    if (result.size() != 3) {
        throw std::runtime_error("input trx is invalid.");
    }
    sender = std::move(result[0]);
    receiver = std::move(result[1]);
    value = std::stod(result[2]);
    return true;
}

std::vector<std::string> Server::pending_trxs;
bool Server::add_pending_trx(std::string trx, std::string signature) {
    std::string sender{}, receiver{};
    double value = 0.0;
    parse_trx(trx, sender, receiver, value);
    auto sender_ptr = get_client(sender);
    auto receiver_ptr = get_client(receiver);
    if (sender_ptr == nullptr || receiver_ptr == nullptr) {
        return false;
    }
    if (!crypto::verifySignature(sender_ptr->get_publickey(), trx, signature)) {
        std::cout << "verify signature failed." << std::endl;
        return false;
    }
    if (value > sender_ptr->get_wallet()) {
        std::cout << "sender do not have enough balance." << std::endl;
        return false;
    }
    pending_trxs.emplace_back(trx);

    return true;
}

size_t Server::mine() {
    std::string mempool{};
    for (const auto& trx : Server::pending_trxs) mempool += trx;
    bool mineSuccess = false;
    size_t ret_nonce;
    while (!mineSuccess) {
        for (auto& [client, balance] : clients) {
            size_t nonce = client->generate_nonce();
            auto hash = crypto::sha256(mempool + std::to_string(nonce));
            if (is_valid_hash(hash)) {
                ret_nonce = nonce;
                clients[client] += 6.25;
                mineSuccess = true;
            }
        }
    }
    for (const auto& trx : Server::pending_trxs) {
        std::string sender{}, receiver{};
        double value;
        Server::parse_trx(trx, sender, receiver, value);
        clients[get_client(sender)] -= value;
        clients[get_client(receiver)] += value;
    }

    return ret_nonce;
}