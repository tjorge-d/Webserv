/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmiguel- <lmiguel-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 14:46:08 by lmiguel-          #+#    #+#             */
/*   Updated: 2025/03/24 16:17:38 by lmiguel-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <iomanip>

struct ServerInfo{

	int				client_request_max_size;
	/* Domain		domain;
	Location		location; */
};

/* struct ServerBlock{

	int				domain_port;
	std::string		server_name;
};

struct DomainBlock{

	bool			autoindex;
	std::string		allowed_services[];
	std::string		root_directory;
	std::string		index_file;
}; */

/* class Parser{

	public:
	
	Parser	( void );
	~Parser	( void );

	void	parseConfig( std::string filename );
	void	parseServerBlocks( std::ifstream file );
	void	parseLocationBlocks( std::ifstream file );
	void	parseDomainPort( std::ifstream file );
	void	parseServerName( std::ifstream file );
	
}; */

#endif