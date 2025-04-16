#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "Socket.hpp"
# include "BindingSocket.hpp"
# include "ListeningSocket.hpp"
# include "ConnectingSocket.hpp"
# include "EventHandler.hpp"
# include "Client.hpp"
# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "Parser.hpp"

# include <errno.h>
# include <string.h>
# include <signal.h>
# include <map>

# define SOCKET_BACKLOG 5
# define MAX_CONNECTIONS 10

#endif