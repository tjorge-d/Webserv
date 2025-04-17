#include "../includes/ServerBlock.hpp"

// CONSTRUCTORS & DESTRUCTORS

ServerBlock::ServerBlock(ServerBlockInfo &info, HttpInfo &httpInfo) :
listener(INADDR_ANY, info.port, SOCKET_BACKLOG),
info(info),
httpInfo(httpInfo)
{
	//std::cout << "ServerBlock default constructor called\n";
}


ServerBlock::~ServerBlock()
{
	//std::cout << "ServerBlock default destructor called\n";
}

// GETTERS

int	const &ServerBlock::getListenerFD()const
{
	(void)info;
	(void)httpInfo;
	return(listener.getFD());
}

