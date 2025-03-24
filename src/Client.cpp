#include "../includes/Client.hpp"

// CONSTRUCTORS & DESTRUCTORS

Client::Client(int fd) : _fd(fd)
{
	std::cout << "Client custom constructor called\n";
}

Client::Client(const Client &copy)
{
	//std::cout << "Client copy constructor called\n";
	*this = copy;
}

Client::~Client()
{
	std::cout << "Client default destructor called\n";
	closeClient();
}

// GETTERS

int	Client::getFD()
{
	return (_fd);
}

// OPERATORS

Client&	Client::operator=(const Client &copy)
{
	//std::cout << "Client copy assignment operator called\n";
	if(this != &copy)
	{
		if (_fd != copy._fd)
			closeClient();
		_fd = copy._fd;
	}
	return (*this);
}

// MEMBER FUNCTIONS

void	Client::closeClient()
{
	// Safely closes the Client fd
	if(_fd >= 0)
	{
		if(close(_fd) == -1)
			throw ClientCloseFailure();
		_fd = -1;
	}
}

Client::ClientCloseFailure::ClientCloseFailure() :
runtime_error("A Client failed to Close"){}