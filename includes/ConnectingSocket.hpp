#ifndef CONNECTINGSOCKET_HPP
# define CONNECTINGSOCKET_HPP
# include "Socket.hpp"

class ConnectingSocket : public Socket
{
	private:
		// MEMBER FUNCTIONS
		// Configures the socket
		virtual void	configureSocket();

	public:
		// CONSTRUCTORS/DESTRUCTORS
		ConnectingSocket(int domain, int type, int protocol, u_long interface, int port);
		ConnectingSocket(u_long interface, int port);
		~ConnectingSocket();

	// EXCEPTIONS
	class	ConnectingSocketException : public std::runtime_error
	{
		public :
			ConnectingSocketException(std::string info);
	};
};

#endif