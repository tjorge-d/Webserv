#include "../includes/BindingSocket.hpp"

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

BindingSocket::BindingSocket(const BindingSocket &copy) :
Socket(0,0,0,0,0)
{
	//std::cout << "BindingSocket copy constructor called\n";
	*this = copy;
}

BindingSocket::~BindingSocket()
{
	//std::cout << "BindingSocket default destructor called\n";
}

// OPERATORS

BindingSocket&	BindingSocket::operator=(const BindingSocket &copy)
{
	//std::cout << "BindingSocket copy assignment operator called\n";
	if(this != &copy)
		Socket::operator=(copy);
	return (*this);
}

// MEMBER FUNCTIONS

void	BindingSocket::configureSocket()
{
	// Initializes the address structure
	_address = IPv4AddressConvertion(_domain, _interface, _port);

	// Binds the socket to an address
	if(bind(_fd, (struct sockaddr *)&_address, sizeof(_address)) != 0)
		throw SocketBindingFailure();
}

// EXCEPTIONS

BindingSocket::SocketBindingFailure::SocketBindingFailure() :
runtime_error("A Socket failed to bind"){}