#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP
# include <stdio.h>
# include <iostream>
# include <string.h>
# include <fstream>
# include "ListeningSocket.hpp"
# include "Webserv.h"

class ServerBlock
{
	private:
		// ATTRIBUTES
		ListeningSocket	listener;
		ServerBlockInfo	&info;
		int				maxBodySize;
		// HttpInfo		&httpInfo;
		
		public:
		// CONSTRUCTORS/DESTRUCTORS
		ServerBlock(ServerBlockInfo &info, int &size);
		~ServerBlock();

		// GETTERS
		int	const						&getListenerFD()const;
		int const						&getMaxBodySize()const;
		ServerBlockInfo					&getInfo() const;
		std::map<int, std::string>		&getErrorPages() const;
};

#endif