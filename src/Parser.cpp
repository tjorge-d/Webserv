/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmiguel- <lmiguel-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:16:29 by lmiguel-          #+#    #+#             */
/*   Updated: 2025/04/22 15:57:03 by lmiguel-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*

	THINGS TO DO BEFORE PASSING INFORMATION:

	CHECK IF ARGUMENT NUMBER IS VALID
	CHECK IF ARGUMENT IS A CONFIG FILE
	CHECK IF CONFIG FILE IS VALID
	CHECK IF FILE CONTAINS EVERYTHING (could be saved for last? checkmark system?
	requires knowing what is essential for our webserver to function) (yes, it will be saved for last)

	THINGS TO PASS DOWN:

	SERVER BLOCKS done
	CLIENT REQUEST SIZE LIMIT done
	DOMAIN PORTS done
	DOMAIN ADDRESS done
	WEBSERVER DEFAULT LOCATION done
	AVAILABLE SERVICES done
	ROOT DIRECTORY done
	AUTOINDEX STATE done
	ERROR CODES AND CORRESPONDING ERROR PAGES done
	DEFAULT FILE FOR DIRECTORY REQUESTS done
*/

#include "../includes/Webserv.h"
/* 
bool locationBlockSolver()
{
	
} */


static void setupServices(LocationBlockInfo *locationBlock, std::string acquired_services)
{
	std::string						current_method;
	std::stringstream				stream(acquired_services);
	

	// check valid methods (also stringstream practice)
	while (stream >> current_method)
	{
		if (current_method != "GET" && current_method != "POST" && current_method != "DELETE" && current_method != "HEAD")
			throw ParserException("Attempt to configure invalid service. Allowed services are: GET POST DELETE HEAD");
		locationBlock->allowed_services.push_back(current_method);
	}
}

static void setupClientmaxbodysize(HttpInfo *webserver, std::string acquired_maxbodysize)
{
	// This function will check if everything is gucci, AKA:

	// Is the client_max_body_size different from 0? if not, convert to 1 Mb, or 1024^2 (Client max size will always be in bytes to make things easier)

	// Are the ServerBlock structs filled? if even one of them is empty, its invalid. Likewise for DomainBlock, except the autoindex boolean. The default value is false, therefore it can safely be removed from the checks.

	// Practice using stringstream for things with more than one string, such as all the methods and the client max size. (methods done)

	int								unconverted_maxbodysize;
	std::string						body_size;
	std::string						unit;
	std::stringstream				stream(acquired_maxbodysize);

	/*
		BYTE CONVERSION

		n Kb = n x (1024) Bytes
		n Mb = n x (1024^2) Bytes
		n Gb = n x (1024^3) Bytes
	 */

	stream >> body_size >> unit;
	std::cout << "SIZE: " << body_size <<std::endl;
	std::cout << "UNIT: " << unit <<std::endl;
	unconverted_maxbodysize = atoi(body_size.c_str());
	if (unit == "Mb" || unit == "mb" || unit == "mb;" || unit == "Mb;")
		unconverted_maxbodysize = unconverted_maxbodysize * 1024 * 1024;
	else if (unit == "Kb" || unit == "kb" || unit == "kb;" || unit == "Kb;")
		unconverted_maxbodysize = unconverted_maxbodysize * 1024;
	else
		throw ParserException("Invalid client request body size storage unit, please use Mb/mb or Kb/kb as the storage unit.");
	if (unconverted_maxbodysize <= 0 || unconverted_maxbodysize > INT_MAX)
		throw ParserException("Invalid maximum client request body size.");
	webserver->client_max_body_size = unconverted_maxbodysize;
}

HttpInfo *config_parser(char *file_path)
{
	bool max_size_acquired = false;
	bool server_setup_mode = false;
	bool location_setup_mode = false;
	ServerBlockInfo		current_server_block;
	LocationBlockInfo	current_location_block;
	std::string		start;
	std::string		line;
	std::string		newline;
	std::string		current_start;
	std::string		acquired_services;
	std::string		acquired_max_body_size;
	std::ifstream	config_file;

	HttpInfo *Server = new HttpInfo();
	config_file.open(file_path);
	if (!config_file.is_open()) // if the config file cannot be opened/doesn't exist, throw this error.
		throw ParserException("Invalid config file.");
	if (!config_file.good())
	{
		config_file.close();
		throw ParserException("Error while openning file.");
	}
	if (config_file.is_open() && config_file.peek() == EOF)
		throw ParserException("Your config file is empty.");

	while (std::getline(config_file, line))
	{ // main loop, continue until text ends
		if (line.size() > 0 && line[line.size() - 1] != ';')
			throw ParserException("All lines must end in the ';' character.");
		//----------------------------SERVER SPECIFIC INFO, DOES NOT REQUIRE A SERVERBLOCK TO EXIST----------------------------
		if ((line.find("client_max_body_size")) != std::string::npos)
		{
			if (max_size_acquired == false)
			{
				acquired_max_body_size = line.substr((line.find(' ') + 1), line.rfind(' '));
				if (atoi(acquired_max_body_size.c_str()) <= 0)
					throw ParserException("Your client_max_body_size is invalid.");
				max_size_acquired = true;
			}
			else
				throw ParserException("Multiple client_max_body_size detected.");
		}
		if ((line.find("server_block start;")) != std::string::npos)
		{
			if (server_setup_mode == false)
			{
				server_setup_mode = true;
			}
			else
				throw ParserException("Attempt to create server block inside another server block.");
		}
		if ((line.find("server_block end;")) != std::string::npos)
		{
			if (server_setup_mode == true)
			{
				server_setup_mode = false;
				Server->server_blocks.push_back(current_server_block);
				current_server_block = ServerBlockInfo();
			}
			else
				throw ParserException("Attempt to finish nonexistent server block.");
		}
		//----------------------------SERVERBLOCK SPECIFIC INFO, REQUIRES A SERVERBLOCK TO EXIST----------------------------
		if ((line.find("location_block start;")) != std::string::npos)
		{
			if (server_setup_mode == true && location_setup_mode == false)
				location_setup_mode = true;
			else if (location_setup_mode == true)
				throw ParserException("Attempt to create location block inside another location block.");
			else
				throw ParserException("Attempt to create location block outside of server block.");
		}
		if ((line.find("location_block end;")) != std::string::npos)
		{
			if (location_setup_mode == true)
			{
				location_setup_mode = false;
				current_server_block.locations.push_back(current_location_block);
				current_location_block = LocationBlockInfo();
			}
			else
				throw ParserException("Attempt to finish nonexistent location block.");
		}
		if ((line.find("domain_port")) != std::string::npos)
		{
			if (server_setup_mode == true)
				current_server_block.port = atoi((line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2)).c_str());
			else
				throw ParserException("Attempting to set up ports on nonexistent server block.");
		}
		if ((line.find("server_name")) != std::string::npos)
		{
			if (server_setup_mode == true)
			{
				current_server_block.server_name = line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2);
				if (current_server_block.server_name.empty())
					throw ParserException("Your server name is empty.");
			}
			else
				throw ParserException("Attempting to set up server name on nonexistent server block.");
		}
		if ((line.find("directory_request_redirect")) != std::string::npos)
		{
			if (server_setup_mode == true)
			{
				current_server_block.redirect_directory = line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2);
				if (current_server_block.redirect_directory.empty())
					throw ParserException("Your directory request redirection file is invalid/nonexistent.");
			}
			else
				throw ParserException("Attempting to set up directory request redirection file on nonexistent server block.");
		}
		if ((line.find("error_page")) != std::string::npos)
		{
			if (server_setup_mode == true)
			{
				current_server_block.error_codes.insert(std::pair<int, std::string>(atoi(line.substr((line.find(' ') + 1), line.size() - line.rfind(' ') - 2).c_str()), line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2)));
				for (std::map<int, std::string>::iterator it = current_server_block.error_codes.lower_bound(0); it != current_server_block.error_codes.upper_bound(1000); ++it)
				{
					if (it->first <= 0 || it->second.empty())
						throw ParserException("Your error code or coresponding page is invalid/nonexistent.");
				}
			}
			else
				throw ParserException("Attempting to set up error codes and error pages on nonexistent server block.");
		}
		//----------------------------LOCATION BLOCK SPECIFIC INFO, REQUIRES A LOCATION BLOCK TO EXIST (REMEMBER LOCATION BLOCKS REQUIRE SERVERBLOCKS TO EXIST)----------------------------
		if ((line.find("autoindex")) != std::string::npos)
		{
			if (location_setup_mode == true)
			{
				if (line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2) == "on")
					current_location_block.autoindex = true;
				else if (line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2) == "off")
					current_location_block.autoindex = false;
				else
					throw ParserException("Invalid autoindexing options, please set it to on or off.");
			}
			else
				throw ParserException("Attempting to set up autoindexing on nonexistent location block.");
		}
		if ((line.find("services_available")) != std::string::npos)
		{
			if (location_setup_mode == true)
			{
				acquired_services = line.substr((line.find(' ') + 1), line.size() - line.find(' ') - 2);
				if (acquired_services.empty())
					throw ParserException("Your allowed services are empty.");
				setupServices(&current_location_block, acquired_services);
			}
			else
				throw ParserException("Attempting to set up allowed services on nonexistent domain block.");
		}
		if ((line.find("index_file")) != std::string::npos)
		{
			if (location_setup_mode == true)
			{
				current_location_block.index_file = line.substr((line.find(' ') + 1), line.size() - line.find(' ') - 2);
				if (current_location_block.index_file.empty())
					throw ParserException("Your location index file is invalid/empty.");
			}
			else
				throw ParserException("Attempting to set up location index file on nonexistent location block.");
		}
		if ((line.find("default_location")) != std::string::npos)
		{
			if (location_setup_mode == true)
			{
				current_location_block.location = line.substr((line.find(' ') + 1), line.size() - line.find(' ') - 2);
				if (current_location_block.location.empty())
					throw ParserException("Your location location is empty.");
			}
			else
				throw ParserException("Attempting to set up a location on nonexistent location block.");
		}
		if ((line.find("root")) != std::string::npos)
		{
			if (location_setup_mode == true)
			{
				current_location_block.root_directory = line.substr((line.find(' ') + 1), line.size() - line.find(' ') - 2);
				if (current_location_block.root_directory.empty())
					throw ParserException("Your root directory is empty.");
			}
			else
				throw ParserException("Attempting to set up root directory on nonexistent location block.");
		}
		newline += line;
		newline += '\n';
	}
	setupClientmaxbodysize(Server, acquired_max_body_size);
	std::cout << "Client max body size : " << Server->client_max_body_size << std::endl;
	for (std::vector<ServerBlockInfo>::iterator i = Server->server_blocks.begin(); \
		i != Server->server_blocks.end(); i++)
	{
		std::cout << "Server Block : " << i->server_name << std::endl;
		std::cout << "Server Port : " << i->port << std::endl;
		std::cout << "Redirect Directory : " << i->redirect_directory << std::endl;
		for (std::map<int, std::string>::iterator k = i->error_codes.begin(); \
		k != i->error_codes.end(); k++)
			std::cout << "Error code : " << k->first << " Error index file : " << k->second << std::endl;
		for (std::vector<LocationBlockInfo>::iterator j = i->locations.begin(); \
			j != i->locations.end(); j++)
		{
			std::cout << "location Block : " << j->location << std::endl;
			std::cout << "location Autoindex State : " << j->autoindex << std::endl;
			std::cout << "Root Directory : " << j->root_directory << std::endl;
			std::cout << "Index File : " << j->index_file << std::endl;
			std::cout << "Allowed Services : " << std::endl;
			for (std::vector<std::string>::iterator m = j->allowed_services.begin(); \
			m != j->allowed_services.end(); m++)
				std::cout << *m << std::endl;
		}
	}
	config_file.close();
	return (Server);
}

ParserException::ParserException(std::string error) :
std::runtime_error("The parser ran into a problem: " + error){};
