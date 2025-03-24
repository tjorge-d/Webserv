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

ConnectingSocket::ConnectingSocket(const ConnectingSocket &copy) :
Socket(0,0,0,0,0)
{
	//std::cout << "ConnectingSocket copy constructor called\n";
	*this = copy;
}

ConnectingSocket::~ConnectingSocket()
{
	//std::cout << "ConnectingSocket default destructor called\n";
}

// OPERATORS

ConnectingSocket&	ConnectingSocket::operator=(const ConnectingSocket &copy)
{
	//std::cout << "ConnectingSocket copy assignment operator called\n";
	if(this != &copy)
		Socket::operator=(copy);
	return (*this);
}

// MEMBER FUNCTIONS

void	ConnectingSocket::configureSocket()
{
	// Initializes the address structure
	_address = IPv4AddressConvertion(_fd, _interface, _port);
	
	// Connects the socket to an address
	if(connect(_fd, (struct sockaddr *)&_address, sizeof(_address)) != 0)
		throw SocketConnectingFailure();
}

// EXCEPTIONS

ConnectingSocket::SocketConnectingFailure::SocketConnectingFailure() :
runtime_error("A Socket failed to connect"){}