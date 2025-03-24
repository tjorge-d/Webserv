#ifndef LISTENINGSOCKET_HPP
# define LISTENINGSOCKET_HPP
# include "BindingSocket.hpp"

class ListeningSocket : public BindingSocket
{
	private:
		// ATTRIBUTES
		int	_backlog;

	public:
		// CONSTRUCTORS/DESTRUCTORS
		ListeningSocket(int domain, int type, int protocol, u_long interface, int port,int backlog);
		ListeningSocket(u_long interface, int port, int backlog);
		ListeningSocket(const ListeningSocket &a);
		~ListeningSocket();

		// OPERATORS
		ListeningSocket&	operator=(const ListeningSocket &copy);

		// MEMBER FUNCTIONS
		// Configures the Socket
		virtual void	configureSocket();

	// EXCEPTIONS
	class	SocketListeningFailure : public std::runtime_error
	{
		public :
			SocketListeningFailure();
	};
};

#endif