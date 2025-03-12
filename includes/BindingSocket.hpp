#ifndef BINDINGSOCKET_HPP
# define BINDINGSOCKET_HPP
# include "Socket.hpp"

class BindingSocket : public Socket
{
	private:
		BindingSocket();

	public:
		// CONSTRUCTORS/DESTRUCTORS
		BindingSocket(int domain, int type, int protocol, u_long interface, int port);
		BindingSocket(u_long interface, int port);
		BindingSocket(const BindingSocket &a);
		~BindingSocket();

		// OPERATORS
		BindingSocket&	operator=(const BindingSocket &copy);
		
		// MEMBER FUNCTIONS
		virtual void	configureSocket();

	class	SocketBindingFailure : public std::runtime_error
	{
		public :
			SocketBindingFailure();
	};
};

#endif