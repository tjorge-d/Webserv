#ifndef SOCKET_HPP
# define SOCKET_HPP
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <stdexcept>
# include <unistd.h>
# include <iostream>

class Socket
{
	private:
		// CONSTRUCTORS/DESTRUCTORS
		Socket();

	protected:
		// ATTRIBUTES
		int					_fd;
		int					_domain;
		int 				_type;
		int					_protocol;
		u_long				_interface;
		int					_port;
		struct sockaddr_in	_address;
		
		// CONSTRUCTORS/DESTRUCTORS
		Socket(int domain, int type, int protocol, u_long interface, int port);
		Socket(const Socket &a);
		~Socket();

		// OPERATORS
		Socket&	operator=(const Socket &copy);
		
	public:
		// GETTERS
		int					getFD();
		int					getDomain();
		int					getType();
		int					getProtocol();
		u_long				getInterface();
		int					getPort();
		struct sockaddr_in	getAddress();

		// MEMBER FUNCTIONS
		void				closeSocket();
		struct sockaddr_in	IPv4AddressConvertion(int socket_fd, u_long interface , int port);
		virtual void		configureSocket() = 0;

	// EXCEPTIONS
	class	SocketCloseFailure : public std::runtime_error
	{
		public :
			SocketCloseFailure();
	};

	class	SocketOpenFailure : public std::runtime_error
	{
		public :
			SocketOpenFailure();
	};
};

#endif