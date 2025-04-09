#ifndef SOCKET_HPP
# define SOCKET_HPP
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdexcept>
# include <unistd.h>

class Socket
{
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
		virtual ~Socket();

		// MEMBER FUNCTIONS
		// Safely closes the Socket
		void	closeSocket();
		
		// Returns a sockaddr_in struct (for IPv4)
		struct sockaddr_in	IPv4AddressConvertion(int socket_fd, u_long interface , int port);
				
		// Mandatory socket configuration in derived classes
		virtual void	configureSocket() = 0;
		
	public:
		// GETTERS
		int					getFD();
		int					getDomain();
		int					getType();
		int					getProtocol();
		u_long				getInterface();
		int					getPort();
		struct sockaddr_in	getAddress();

	// EXCEPTIONS
	class	SocketException : public std::runtime_error
	{
		public :
			SocketException(std::string info);
	};
};

#endif