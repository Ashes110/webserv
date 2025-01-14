#include "ClientSocket.hpp"

static std::string convertUintToIpString(u_int32_t s_addr) {
	int byte;
	int bitshift(0);
	std::string ipAddr;
	std::stringstream ss;

	while (4 > bitshift) {
		byte = (s_addr >> (bitshift * 8)) & 0xFF;
		ss << byte;
		if (bitshift == 3) {
			ipAddr.append(ss.str());
		} else {
			ipAddr.append(ss.str() + ".");
		}
		ss.str("");
		ss.clear(std::stringstream::goodbit);
		bitshift++;
	}
	return ipAddr;
}

ClientSocket::ClientSocket() : _serverSocket(NULL) {}

ClientSocket::ClientSocket(const std::pair<int, sockaddr_in> &socketInfo,
						   ServerSocket *serverSocket)
	: _fd(socketInfo.first),
	  _remoteAddr(convertUintToIpString(socketInfo.second.sin_addr.s_addr)),
	  _revents(0),
	  _phase(ClientSocket::RECV),
	  _lastSendTimestamp(std::time(NULL)),
	  _serverSocket(serverSocket) {}

int ClientSocket::getFd() const { return this->_fd; }

std::string const &ClientSocket::getRemoteAddr() const {
	return this->_remoteAddr;
}

void ClientSocket::setRevents(short revents) { this->_revents = revents; }

short ClientSocket::getRevents() const { return this->_revents; }

ClientSocket::csphase ClientSocket::tryRecv() {
	char buf[BUFFERSIZE];
	ssize_t recvlen;

	if ((this->_revents & POLLIN) != POLLIN) {
		return this->_phase;
	}
	std::memset(&buf, 0, sizeof(buf));
	recvlen = recv(this->_fd, buf, BUFFERSIZE - 1, 0);
	if (recvlen == -1) {
		utils::putSysError("recv");
		return ClientSocket::CLOSE;
	}
	if (recvlen == 0) {
		return ClientSocket::CLOSE;
	}
	this->_lastSendTimestamp = std::time(NULL);
	this->buffer.write(buf, recvlen);
	return ClientSocket::RECV;
}

ssize_t ClientSocket::trySend(std::string const &msg) {
#if defined(_LINUX)
	const int flags = MSG_DONTWAIT | MSG_NOSIGNAL;
#elif defined(_DARWIN)
	const int flags = MSG_DONTWAIT | SO_NOSIGPIPE;
#else
	const int flags = MSG_DONTWAIT | MSG_NOSIGNAL;
#endif

	if ((this->_revents & POLLOUT) != POLLOUT) {
		return 0;
	}
	ssize_t bytes(0);
	bytes = send(this->_fd, msg.c_str(), msg.size(), flags);
	if (bytes == -1) {
		utils::putSysError("send");
		this->_phase = ClientSocket::CLOSE;
		return -1;
	}
	this->_phase = ClientSocket::RECV;
	return bytes;
}

void ClientSocket::close() {
	if (::close(this->_fd) == -1) {
		utils::putSysError("close");
	}
}

void ClientSocket::setPhase(ClientSocket::csphase const phase) {
	this->_phase = phase;
}

ClientSocket::csphase ClientSocket::getPhase() const { return this->_phase; }

std::time_t ClientSocket::getLastSendTimestamp() const {
	return this->_lastSendTimestamp;
}

void ClientSocket::setLastSendTimestamp(std::time_t const lastSendTimestamp) {
	this->_lastSendTimestamp = lastSendTimestamp;
}

ServerSocket *ClientSocket::getServerSocket() const {
	return this->_serverSocket;
}

bool ClientSocket::isOverBytesSize(const std::stringstream::pos_type bytes) {
	std::stringstream::pos_type posStart = this->buffer.tellg();
	this->buffer.seekg(0, std::ios::end);
	std::stringstream::pos_type posEnd = this->buffer.tellg();
	if (posEnd - posStart > bytes) {
		this->buffer.seekg(posStart);
		return true;
	}
	this->buffer.seekg(posStart);
	return false;
}