#include "client.h"

#include "crypto.h"
#include "server.h"

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
