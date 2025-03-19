/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmiguel- <lmiguel-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 14:46:08 by lmiguel-          #+#    #+#             */
/*   Updated: 2025/03/19 14:35:50 by lmiguel-         ###   ########.fr       */
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

	int			client_request_max_size;
	Domain		domain;
	Location	location;
};

struct Domain{

	int			domain_port;
	std::string	domain_name;
};

struct Location{

	bool		autoindex_state;
	std::string	allowed_services;
	std::string	root_directory;
	std::string	index_file;
};

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