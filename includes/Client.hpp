#ifndef CLIENT_HPP
# define CLIENT_HPP
# include <stdio.h>
# include <iostream>
# include <string.h>
# include <fstream>
# include <unistd.h>

class Client
{
	private:
		// ATTRIBUTES
		int		_fd;

	public:
		// CONSTRUCTORS/DESTRUCTORS
		Client(int fd);
		Client(const Client &copy);
		~Client() ;

		// GETTERS
		int	getFD();

		// OPERATORS
		Client&	operator=(const Client &copy);

		// MEMBER FUNCTIONS
		// Safely closes the Client
		void	closeClient();
	
	class	ClientCloseFailure : public std::runtime_error
	{
		public :
			ClientCloseFailure();
	};
};

#endif