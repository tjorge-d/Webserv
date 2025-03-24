#ifndef WEBSERV_HPP
# define WEBSERV_HPP
# include "Socket.hpp"
# include "BindingSocket.hpp"
# include "ListeningSocket.hpp"
# include "ConnectingSocket.hpp"
# include "EventHandler.hpp"
# include "Client.hpp"
# include <errno.h>
# include <string.h>
# include <signal.h>
# include <map>

bool	running;
bool    debug;

/*typedef struct s_server_data
{
}	t_server_data;*/

#endif