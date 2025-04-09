#include "../includes/Webserv.h"

// CONSTRUCTORS & DESTRUCTORS

Socket::Socket(int domain, int type, int protocol, u_long interface, int port) :
_domain(domain),
_type(type),
_protocol(protocol),
_interface(interface),
_port(port)
{
	//std::cout << "Socket custom constructor called\n";
	//Creates a socket
	_fd = socket(_domain, _type, _protocol);
	if(_fd == -1)
		throw SocketException("Failed to create a socket");
}

Socket::~Socket()
{
	//std::cout << "Socket default destructor called\n";
	// Closes the socket safely
	closeSocket();
}

// GETTERS

int	Socket::getFD()
{
	return(_fd);
}

int	Socket::getDomain()
{
	return(_domain);
}

int Socket::getType()
{
	return(_type);
}

int	Socket::getProtocol()
{
	return(_protocol);
}

u_long	Socket::getInterface()
{
	return(_interface);
}

int	Socket::getPort()
{
	return(_port);
}

struct sockaddr_in	Socket::getAddress()
{
	return(_address);
}


// MEMBER FUNCTIONS

void	Socket::closeSocket()
{
	// Safely closes the Socket fd
	if(_fd >= 0)
	{
		if(close(_fd) == -1)
			throw SocketException("Failed to close a socket");
		_fd = -1;
	}
}

struct sockaddr_in	Socket::IPv4AddressConvertion(int domain, u_long interface , int port)
{
	// Fills and returns a sockaddr_in struct (for IPv4)
	struct sockaddr_in	address;
	address.sin_family = domain;
	address.sin_addr.s_addr = htonl(interface);
	address.sin_port = htons(port);
	return (address);
}

// EXCEPTIONS
Socket::SocketException::SocketException(std::string info) :
runtime_error(info + " (" + std::string(strerror(errno)) + ")"){}