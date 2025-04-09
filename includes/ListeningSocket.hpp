#ifndef LISTENINGSOCKET_HPP
# define LISTENINGSOCKET_HPP
# include "BindingSocket.hpp"

class ListeningSocket : public BindingSocket
{
	private:
		// ATTRIBUTES
		int	_backlog;
		
		// MEMBER FUNCTIONS
		// Configures the Socket
		virtual void	configureSocket();

	public:
		// CONSTRUCTORS/DESTRUCTORS
		ListeningSocket(int domain, int type, int protocol, u_long interface, int port,int backlog);
		ListeningSocket(u_long interface, int port, int backlog);
		~ListeningSocket();

		// GETTERS
		int	getBacklog();

	// EXCEPTIONS
	class	ListeningSocketException : public std::runtime_error
	{
		public :
			ListeningSocketException(std::string info);
	};
};

#endif