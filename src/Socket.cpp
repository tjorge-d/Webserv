#include "../includes/Socket.hpp"

// CONSTRUCTORS & DESTRUCTORS

Socket::Socket(int domain, int type, int protocol, u_long interface, int port) :
_domain(domain),
_type(type),
_protocol(protocol),
_interface(interface),
_port(port)
{
	//Creates a socket
	_fd = socket(_domain, _type, _protocol);
	if(_fd == -1)
		throw SocketException("Failed to create a socket");
}

Socket::~Socket()
{
	// Closes the socket safely
	closeSocket();
}

// GETTERS

int const	&Socket::getFD() const
{
	return(_fd);
}

int const	&Socket::getDomain() const
{
	return(_domain);
}

int const	&Socket::getType() const
{
	return(_type);
}

int const	&Socket::getProtocol() const
{
	return(_protocol);
}

u_long const	&Socket::getInterface() const
{
	return(_interface);
}

int const	&Socket::getPort() const
{
	return(_port);
}

struct sockaddr_in const	&Socket::getAddress() const
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
	//address.sin_addr.s_addr = htonl(interface);
	address.sin_addr.s_addr = interface;
	address.sin_port = htons(port);
	return (address);
}

// EXCEPTIONS
Socket::SocketException::SocketException(std::string info) :
runtime_error(info + " (" + std::string(strerror(errno)) + ")"){}