#include "../includes/Webserv.h"

// CONSTRUCTORS & DESTRUCTORS

ListeningSocket::ListeningSocket(int domain, int type, int protocol, u_long interface, int port, int backlog) :
BindingSocket(domain, type, protocol, interface, port),
_backlog(backlog)
{
	//std::cout << "ListeningSocket custom constructor called\n";
	configureSocket();
}

ListeningSocket::ListeningSocket(u_long interface, int port, int backlog) :
BindingSocket(AF_INET, SOCK_STREAM, 0, interface, port),
_backlog(backlog)
{
	//std::cout << "ListeningSocket custom constructor called\n";
	configureSocket();
}

ListeningSocket::~ListeningSocket()
{
	//std::cout << "ListeningSocket default destructor called\n";
}


// GETTERS

int		ListeningSocket::getBacklog()
{
	return (_backlog);
}


// MEMBER FUNCTIONS

void	ListeningSocket::configureSocket()
{
	// Makes the socket passive and ready to accept incoming connection requests
    if(listen(_fd, _backlog) != 0)
        throw ListeningSocketException("Failed to make the socket listen");
}

// EXCEPTIONS

ListeningSocket::ListeningSocketException::ListeningSocketException(std::string info) :
runtime_error(info + " (" + std::string(strerror(errno)) + ")"){}