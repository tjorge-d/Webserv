#include "../includes/ListeningSocket.hpp"

// CONSTRUCTORS & DESTRUCTORS

ListeningSocket::ListeningSocket(int domain, int type, int protocol, u_long interface, int port, int backlog) :
BindingSocket(domain, type, protocol, interface, port),
_backlog(backlog)
{
	//std::cout << "Socket custom constructor called\n";
	configureSocket();
}

ListeningSocket::ListeningSocket(u_long interface, int port, int backlog) :
BindingSocket(AF_INET, SOCK_STREAM, 0, interface, port),
_backlog(backlog)
{
	//std::cout << "Socket custom constructor called\n";
	configureSocket();
}

ListeningSocket::ListeningSocket(const ListeningSocket &copy) :
BindingSocket(0,0,0,0,0)
{
	//std::cout << "ListeningSocket copy constructor called\n";
	*this = copy;
}

ListeningSocket::~ListeningSocket()
{
	//std::cout << "ListeningSocket default destructor called\n";
}

// OPERATORS

ListeningSocket&	ListeningSocket::operator=(const ListeningSocket &copy)
{
	//std::cout << "ListeningSocket copy assignment operator called\n";
	if (this != &copy)
		BindingSocket::operator=(copy);
	return (*this);
}

// MEMBER FUNCTIONS

void	ListeningSocket::configureSocket()
{
	// Makes the socket passive and ready to accept incoming connection requests
    if (listen(_fd, _backlog) != 0)
        throw SocketListeningFailure();
}

// EXCEPTIONS

ListeningSocket::SocketListeningFailure::SocketListeningFailure() :
runtime_error("A Socket failed to start listening"){}