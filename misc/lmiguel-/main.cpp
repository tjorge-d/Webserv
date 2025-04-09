/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmiguel- <lmiguel-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:16:29 by lmiguel-          #+#    #+#             */
/*   Updated: 2025/04/09 18:50:11 by lmiguel-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*

	THINGS TO DO BEFORE PASSING INFORMATION:

	CHECK IF ARGUMENT NUMBER IS VALID
	CHECK IF ARGUMENT IS A CONFIG FILE
	CHECK IF CONFIG FILE IS VALID
	CHECK IF FILE CONTAINS EVERYTHING (could be saved for last? checkmark system?
	requires knowing what is essential for our webserver to function)

	THINGS TO PASS DOWN:

	SERVER BLOCKS
	CLIENT REQUEST SIZE LIMIT
	DOMAIN PORTS
	DOMAIN ADDRESS
	WEBSERVER DEFAULT LOCATION
	AVAILABLE SERVICES
	ROOT DIRECTORY
	AUTOINDEX STATE
	ERROR CODES AND CORRESPONDING ERROR PAGES
	DEFAULT FILE FOR DIRECTORY REQUESTS
*/

#include "Parser.hpp"
#include "Exception.hpp"

/* bool domainBlockSolver()
{
	
} */

int main(int argc, char **argv){

	std::string		start;
	std::string		line;
	std::string		newline;
	std::string		current_start;
	std::ifstream	config_file;
	//std::string		info;
	//int				serverblocknum = 0;
	//int				server_block_num;
	bool			max_size_acquired = false;
	bool			server_setup_mode = false;
	bool			domain_setup_mode = false;

	ServerInfo *Server = new ServerInfo();
	if (!Server)
		return (0);
	try{

		if (argc != 2) //only 2 arguments are required: ./webserv and [config_file]. anything else is invalid
			throw ParserException("Invalid number of arguments.");
		config_file.open (argv[1]);
		if (!config_file) //if the config file cannot be opened/doesn't exist, throw this error.
			throw ParserException("Invalid config file");
		while (std::getline(config_file, line)) //main loop, continue until text ends
		{
			//----------------------------SERVER SPECIFIC INFO, DOES NOT REQUIRE A SERVERBLOCK TO EXIST----------------------------
			if ((line.find("client_request_max_size")) != std::string::npos){
				if (max_size_acquired == false){
					Server->client_request_max_size = line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2);
					if (atoi(Server->client_request_max_size.c_str()) == 0)
						throw ParserException ("Your client_request_max_size is zero.");
					max_size_acquired = true;
				}
				else
					throw ParserException("Multiple client_request_max_size detected.");
			}
			if ((line.find("server_block start;")) != std::string::npos){
				if (server_setup_mode == false)
					server_setup_mode = true;
				else
					throw ParserException("Attempt to create server block inside another server block.");
			}
			//----------------------------SERVERBLOCK SPECIFIC INFO, REQUIRES A SERVERBLOCK TO EXIST----------------------------
			if ((line.find("server_block end;")) != std::string::npos){
				if (server_setup_mode == true)
					server_setup_mode = false;
				else
					throw ParserException("Attempt to finish unexistent server block.");
			}
			if ((line.find("domain_block start;")) != std::string::npos){
				if (server_setup_mode == true && domain_setup_mode == false)
					domain_setup_mode = true;
				else if (domain_setup_mode == true)
					throw ParserException("Attempt to create domain block inside another domain block.");
				else
					throw ParserException("Attempt to create domain block outside of server block.");
			}
			if ((line.find("domain_block end;")) != std::string::npos){
				if (domain_setup_mode == true)
					domain_setup_mode = false;
				else
					throw ParserException("Attempt to finish unexistent domain block.");
			}
			if ((line.find("domain_port")) != std::string::npos){
				if (server_setup_mode == true)
					Server->location.domain_port = atoi((line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2)).c_str());
				else
					throw ParserException("Attempting to set up ports on nonexistent server block.");
			}
			if ((line.find("server_name")) != std::string::npos){
				if (server_setup_mode == true){
					Server->location.server_name = line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2);
					if (Server->location.server_name.empty())
						throw ParserException("Your server name is empty.");
				}
				else
					throw ParserException("Attempting to set up server name on nonexistent server block.");
			}
			if ((line.find("directory_request_redirect")) != std::string::npos){
				if (server_setup_mode == true){
					Server->location.redirect_directory = line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2);
					if (Server->location.redirect_directory.empty())
						throw ParserException("Your directory request redirection file is invalid/nonexistent.");
				}
				else
					throw ParserException("Attempting to set up directory request redirection file on nonexistent server block.");
			}
			if ((line.find("error_page")) != std::string::npos){
				if (server_setup_mode == true){
					Server->location.error_codes.insert( std::pair<int, std::string>
					(atoi(line.substr((line.find(' ') + 1), line.size() - line.rfind(' ') - 2).c_str()), line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2)));
					for (std::map<int, std::string>::iterator it = Server->location.error_codes.lower_bound(0); it != Server->location.error_codes.upper_bound(1000); ++it) {
						if (it->first == 0 || it->second.empty())
							throw ParserException("Your error code or coresponding page is invalid/nonexistent.");
					}
				}
				else
					throw ParserException("Attempting to set up error codes and error pages on nonexistent server block.");
			}
			//----------------------------DOMAINBLOCK SPECIFIC INFO, REQUIRES A DOMAINBLOCK TO EXIST (REMEMBER DOMAINBLOCKS REQUIRE SERVERBLOCKS TO EXIST)----------------------------
			if ((line.find("autoindex")) != std::string::npos){
				if (domain_setup_mode == true){
					if (line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2) == "on")
						Server->location.domain.autoindex = true;
					else if (line.substr((line.rfind(' ') + 1), line.size() - line.rfind(' ') - 2) == "off")
						Server->location.domain.autoindex = false;
					else
						throw ParserException("Invalid autoindexing options, please set it to on or off.");
				}
				else
					throw ParserException("Attempting to set up autoindexing on nonexistent domain block.");
			}
			if ((line.find("services_available")) != std::string::npos){
				if (domain_setup_mode == true){
					Server->location.domain.allowed_services = line.substr((line.find(' ') + 1), line.size() - line.find(' ') - 2);
					if (Server->location.domain.allowed_services.empty())
						throw ParserException("Your allowed services are empty.");
				}
				else
					throw ParserException("Attempting to set up allowed services on nonexistent domain block.");
			}
			if ((line.find("root")) != std::string::npos){
				if (domain_setup_mode == true){
					Server->location.domain.root_directory = line.substr((line.find(' ') + 1), line.size() - line.find(' ') - 2);
					if (Server->location.domain.root_directory.empty())
						throw ParserException("Your root directory is empty.");
				}
				else
					throw ParserException("Attempting to set up root directory on nonexistent domain block.");
			}
			newline += line;
			newline += '\n';
		}
		std::cout << newline << std::endl; //EXPERIMENTAL: print the entire file. (WORKING)
		std::cout << Server->client_request_max_size << std::endl; //correct;
		std::cout << Server->location.domain_port << std::endl; //correct, figure out how to send to corresponding server;
		std::cout << Server->location.server_name << std::endl; //correct, need to cover "server_name;"? (YES)
		std::cout << Server->location.domain.autoindex << std::endl; //correct, perfect as it is.
		std::cout << Server->location.domain.allowed_services << std::endl; //correct;
		std::cout << Server->location.redirect_directory << std::endl; //correct
		for (std::map<int, std::string>::iterator it = Server->location.error_codes.lower_bound(0); it != Server->location.error_codes.upper_bound(1000); ++it) {
			std::cout << "Error " << it->first << ": " << it->second << std::endl;
	}
	}
	catch(std::exception &e){
		std::cerr << e.what() << std::endl; //catch and print all exceptions thrown during code execution.
	}
	delete Server;
	if (config_file.is_open())
		config_file.close();
	return (1);
}
