/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmiguel- <lmiguel-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:16:29 by lmiguel-          #+#    #+#             */
/*   Updated: 2025/03/24 16:28:14 by lmiguel-         ###   ########.fr       */
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

	std::ifstream	config_file;
	//std::size_t		start;
	std::string		line;
	std::string		newline;
	std::string		current_start;

	ServerInfo *Server = new ServerInfo();
	try{
		
		if (argc != 2) //only 2 arguments are required: ./webserv and [config_file]. anything else is invalid
			throw ParserException("Invalid number of arguments.");
		config_file.open (argv[1]);
		if (!config_file) //if the config file cannot be opened/doesn't exist, throw this error.
			throw ParserException("Invalid config file");
		while (std::getline(config_file, line)) //main loop, continue until text ends
		{
			newline += line;
			newline += '\n';
		}
		std::cout << newline << std::endl; //EXPERIMENTAL: print the entire file.
		while ((newline.find("client_request_max_size")) == std::string::npos)
		{
			throw ParserException("Could not find client_request_max_size.");
		}
	}
	catch(std::exception &e){
		std::cerr << e.what() << std::endl; //catch and print all exceptions thrown during code execution.
	}
	delete Server;
	if (config_file.is_open())
		config_file.close();
}
