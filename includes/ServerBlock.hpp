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
		
		public:
		// CONSTRUCTORS/DESTRUCTORS
		std::map<int, std::string> error_codes;
		HttpInfo		&httpInfo;
		ServerBlock(ServerBlockInfo &info, HttpInfo &httpInfo);
		~ServerBlock();

		// GETTERS
		int	const	&getListenerFD()const;
		int const	&getMaxBodySize()const;
		//make a getter for failsafes / error pages
};

#endif