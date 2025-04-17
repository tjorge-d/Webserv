#include "../includes/ConnectingSocket.hpp"

// CONSTRUCTORS & DESTRUCTORS

ConnectingSocket::ConnectingSocket(int domain, int type, int protocol, u_long interface, int port) :
Socket(domain, type, protocol, interface, port)
{
	//std::cout << "Socket custom constructor called\n";
	configureSocket();
}

ConnectingSocket::ConnectingSocket(u_long interface, int port) :
Socket(AF_INET, SOCK_STREAM, 0, interface, port)
{
	//std::cout << "Socket custom constructor called\n";
	configureSocket();
}

ConnectingSocket::~ConnectingSocket()
{
	//std::cout << "ConnectingSocket default destructor called\n";
}

// MEMBER FUNCTIONS

void	ConnectingSocket::configureSocket()
{
	// Initializes the address structure
	_address = IPv4AddressConvertion(_fd, _interface, _port);
	
	// Connects the socket to an address
	if(connect(_fd, (struct sockaddr *)&_address, sizeof(_address)) != 0)
		throw ConnectingSocketException("Failed to connect a socket");
}

// EXCEPTIONS

ConnectingSocket::ConnectingSocketException::ConnectingSocketException(std::string info) :
runtime_error(info + " (" + std::string(strerror(errno)) + ")"){}