#include "../includes/Socket.hpp"

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
		throw SocketOpenFailure();
}

Socket::Socket(const Socket &copy)
{
	//std::cout << "Socket copy constructor called\n";
	*this = copy;
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

// OPERATORS

Socket&	Socket::operator=(const Socket &copy)
{
	//std::cout << "Socket copy assignment operator called\n";
	if(this != &copy)
	{
		closeSocket();
		_fd = copy._fd;
		_domain = copy._domain;
		_type = copy._type;
		_protocol = copy._protocol;
		_interface = copy._interface;
		_port = copy._port;
		_address = copy._address;
	}
	return (*this);
}

// MEMBER FUNCTIONS

void	Socket::closeSocket()
{
	// Safely closes the Socket fd
	if(_fd >= 0)
	{
		if(close(_fd) == -1)
			throw SocketCloseFailure();
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
Socket::SocketCloseFailure::SocketCloseFailure() :
runtime_error("A Socket failed to close"){}

Socket::SocketOpenFailure::SocketOpenFailure() :
runtime_error("A Socket failed to open"){}