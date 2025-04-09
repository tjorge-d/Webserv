#ifndef BINDINGSOCKET_HPP
# define BINDINGSOCKET_HPP
# include "Socket.hpp"

class BindingSocket : public Socket
{
	protected:
		// MEMBER FUNCTIONS
		// Configures the socket
		virtual void	configureSocket();

	public:
		// CONSTRUCTORS/DESTRUCTORS
		BindingSocket(int domain, int type, int protocol, u_long interface, int port);
		BindingSocket(u_long interface, int port);
		~BindingSocket();

	// EXCEPTIONS
	class	BindingSocketException : public std::runtime_error
	{
		public :
			BindingSocketException(std::string info);
	};
};

#endif