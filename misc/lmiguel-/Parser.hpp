/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmiguel- <lmiguel-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 14:46:08 by lmiguel-          #+#    #+#             */
/*   Updated: 2025/04/10 17:55:43 by lmiguel-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <cstdlib>
#include <map>
#include <vector>
#include <sstream>

struct DomainBlock{
	
	bool							autoindex;
	std::vector<std::string>		allowed_services;
	std::string						root_directory;
	std::string						index_file;
};

struct ServerBlock{
	
	int								domain_port;
	std::string						server_name;
	std::string						redirect_directory;
	std::map<int, std::string>		error_codes;
	DomainBlock						domain;
};

struct ServerInfo{

	unsigned int					client_max_body_size;
	ServerBlock						location;
};

class Parser{

	public:
	
	Parser	( void );
	~Parser	( void );

	/* void Parser::parserOKChecker( ServerInfo *webserver ); */
	/* void	parseConfig( std::string filename );
	void	parseServerBlocks( std::ifstream file );
	void	parseLocationBlocks( std::ifstream file );
	void	parseDomainPort( std::ifstream file );
	void	parseServerName( std::ifstream file ); */
	
};

#endif