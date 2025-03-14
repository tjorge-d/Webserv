#ifndef CONNECTINGSOCKET_HPP
# define CONNECTINGSOCKET_HPP
# include "Socket.hpp"

class ConnectingSocket : public Socket
{
	private:
		// CONSTRUCTORS/DESTRUCTORS
		ConnectingSocket();

	public:
		// CONSTRUCTORS/DESTRUCTORS
		ConnectingSocket(int domain, int type, int protocol, u_long interface, int port);
		ConnectingSocket(u_long interface, int port);
		ConnectingSocket(const ConnectingSocket &a);
		~ConnectingSocket();

		// OPERATORS
		ConnectingSocket&	operator=(const ConnectingSocket &copy);
		
		// MEMBER FUNCTIONS
		virtual void	configureSocket();

	// EXCEPTIONS
	class	SocketConnectingFailure : public std::runtime_error
	{
		public :
			SocketConnectingFailure();
	};
};

#endif