#ifndef IOHANDLER_HPP
# define IOHANDLER_HPP
# include <stdio.h>
# include <unistd.h>
# include <iostream>
# include <string.h>
# include <fstream>
# include <sys/epoll.h>
# include <fcntl.h>

class IOHandler
{
	private:
		// ATTRIBUTES
		int	_fd;
		// CONSTRUCTORS/DESTRUCTORS
		// GETTERS
		// SETTERS
		// OPERATORS
		// MEMBER FUNCTIONS

	public:
		// ATTRIBUTES
		// CONSTRUCTORS/DESTRUCTORS
		IOHandler();
		IOHandler(const IOHandler &a);
		~IOHandler() ;
		// GETTERS
		// SETTERS
		// OPERATORS
		IOHandler&	operator=(const IOHandler &copy);
		// MEMBER FUNCTIONS
		void	closeHandler();
		int		add();
		int		mod();
		int		del();

	// EXCEPTIONS
	class	EPollCreationFailure : public std::runtime_error
	{
		public :
			EPollCreationFailure();
	};

	class	EPollCloseFailure : public std::runtime_error
	{
		public :
			EPollCloseFailure();
	};
};

#endif