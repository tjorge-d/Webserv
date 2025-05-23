/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmiguel- <lmiguel-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 14:46:08 by lmiguel-          #+#    #+#             */
/*   Updated: 2025/04/16 14:37:34 by lmiguel-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include <iostream>
# include <string>
# include <fstream>
# include <cstring>
# include <iomanip>
# include <cstdlib>
# include <map>
# include <vector>
# include <sstream>
# include <climits>

//# define BODY_SIZE_MAX 2147483647; //2 Gb max

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
	std::vector<DomainBlock>		domain;
};

struct ServerInfo{

	int								client_max_body_size;
	std::vector<ServerBlock>		location;
};

class ParserException : public std::runtime_error{

	public:

	ParserException (std::string error);
};

#endif