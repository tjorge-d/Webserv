#include "../includes/IOHandler.hpp"

// CONSTRUCTORS & DESTRUCTORS

IOHandler::IOHandler()
{
	//std::cout << "IOHandler default constructor called\n";
	_fd = epoll_create(1);
	if (_fd == -1)
		throw EPollCreationFailure();
}

IOHandler::IOHandler(const IOHandler &copy)
{
	//std::cout << "IOHandler copy constructor called\n";
	*this = copy;
}

IOHandler::~IOHandler()
{
	//std::cout << "IOHandler default destructor called\n";
	closeHandler();
}

// GETTERS


// SETTERS


// OPERATORS

IOHandler&	IOHandler::operator=(const IOHandler &copy)
{
	//std::cout << "IOHandler copy assignment operator called\n";
	if (this != &copy)
	{
		closeHandler();
		_fd = copy._fd;
	}
	return (*this);
}

// MEMBER FUNCTIONS

void	IOHandler::closeHandler()
{
	if (_fd >= 0)
	{
		if (close(_fd) != 0)
			throw EPollCloseFailure();
		_fd = -1;
	}
}

int		IOHandler::add()
{

}

int		IOHandler::mod()
{

}

int		IOHandler::del()
{

}


// EXCEPTIONS
IOHandler::EPollCreationFailure::EPollCreationFailure() :
runtime_error("Failed to create an epoll instance"){}

IOHandler::EPollCloseFailure::EPollCloseFailure() :
runtime_error("Failed to close an epoll instance"){}
