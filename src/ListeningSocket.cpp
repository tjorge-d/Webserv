#include "../includes/ListeningSocket.hpp"

// CONSTRUCTORS & DESTRUCTORS

ListeningSocket::ListeningSocket(int domain, int type, int protocol, u_long interface, int port, int backlog) :
Socket(domain, type, protocol, interface, port),
_backlog(backlog)
{
	configureSocket();
}

ListeningSocket::ListeningSocket(u_long interface, int port, int backlog) :
Socket(AF_INET, SOCK_STREAM, 0, interface, port),
_backlog(backlog)
{
	configureSocket();
}

ListeningSocket::~ListeningSocket()
{}


// GETTERS

int		ListeningSocket::getBacklog()
{
	return (_backlog);
}


// MEMBER FUNCTIONS

void	ListeningSocket::configureSocket()
{
	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
		throw ListeningSocketException("Failed to set the socket options");

	// Initializes the address structure
	_address = IPv4AddressConvertion(_domain, _interface, _port);

	// Binds the socket to an address
	if(bind(_fd, (struct sockaddr *)&_address, sizeof(_address)) != 0)
		throw ListeningSocketException("Failed to bind a socket");

	// Makes the socket passive and ready to accept incoming connection requests
	if(listen(_fd, _backlog) != 0)
		throw ListeningSocketException("Failed to make the socket listen");
}

// EXCEPTIONS

ListeningSocket::ListeningSocketException::ListeningSocketException(std::string info) :
runtime_error(info + " (" + std::string(strerror(errno)) + ")"){}