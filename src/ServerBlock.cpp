#include "../includes/ServerBlock.hpp"

// CONSTRUCTORS & DESTRUCTORS

ServerBlock::ServerBlock(ServerBlockInfo &info, int &size) :
listener(INADDR_ANY, info.port, SOCKET_BACKLOG),
info(info),
maxBodySize(size){}
// httpInfo(httpInfo)
// {
// 	//std::cout << "ServerBlock default constructor called\n";
// }


ServerBlock::~ServerBlock()
{
	//std::cout << "ServerBlock default destructor called\n";
}

// GETTERS

int	const &ServerBlock::getListenerFD()const
{
	return(listener.getFD());
}

int	const &ServerBlock::getMaxBodySize()const
{
	return(maxBodySize);
}

ServerBlockInfo	&ServerBlock::getInfo()const
{
	return(info);
}

std::map<int, std::string>	&ServerBlock::getErrorPages()const
{
	return(this->info.error_codes);
}
