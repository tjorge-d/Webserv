#include "../includes/Webserv.h"

// CONSTRUCTORS & DESTRUCTORS

BindingSocket::BindingSocket(int domain, int type, int protocol, u_long interface, int port) :
Socket(domain, type, protocol, interface, port)
{
	//std::cout << "Socket custom constructor called\n";
	configureSocket();
}

BindingSocket::BindingSocket(u_long interface, int port) :
Socket(AF_INET, SOCK_STREAM, 0, interface, port)
{
	//std::cout << "Socket custom constructor called\n";
	configureSocket();
}

BindingSocket::~BindingSocket()
{
	//std::cout << "BindingSocket default destructor called\n";
}

// MEMBER FUNCTIONS

void	BindingSocket::configureSocket()
{
	// Sets the options of the socket
	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
		throw BindingSocketException("Failed to set the socket options");

	// Initializes the address structure
	_address = IPv4AddressConvertion(_domain, _interface, _port);

	// Binds the socket to an address
	if(bind(_fd, (struct sockaddr *)&_address, sizeof(_address)) != 0)
		throw BindingSocketException("Failed to bind a socket");
}

// EXCEPTIONS

BindingSocket::BindingSocketException::BindingSocketException(std::string info) :
runtime_error(info + " (" + std::string(strerror(errno)) + ")"){}