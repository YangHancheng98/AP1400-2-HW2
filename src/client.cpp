#include "client.h"

#include <random>

#include "crypto.h"
#include "server.h"

namespace {
std::string generate_trx(const std::string& sender, const std::string& receiver,
                         double value) {
    return sender + "-" + receiver + "-" + std::to_string(value);
}
}  // namespace

Client::Client(std::string id, const Server& server)
    : id(std::move(id)), server(&server) {
    crypto::generate_key(this->public_key, this->private_key);
}

std::string Client::get_id() { return this->id; }

double Client::get_wallet() {
    return const_cast<Server*>(server)->get_wallet(id);
}

std::string Client::get_publickey() const { return this->public_key; }

std::string Client::sign(std::string txt) const {
    return crypto::signMessage(this->private_key, txt);
}

bool Client::transfer_money(std::string receiver, double value) {
    auto trx = generate_trx(this->get_id(), receiver, value);
    auto signature = sign(trx);
    return const_cast<Server*>(server)->add_pending_trx(trx, signature);
}

size_t Client::generate_nonce() {
    std::random_device rd;
    size_t value = 0;

    // 逐字节填充，避免 size_t 比 random_device::result_type 大
    for (size_t i = 0; i < sizeof(size_t); i++) {
        value = (value << 8) | (rd() & 0xFF);
    }
    return value;
}