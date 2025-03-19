/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmiguel- <lmiguel-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:16:29 by lmiguel-          #+#    #+#             */
/*   Updated: 2025/03/19 15:22:52 by lmiguel-         ###   ########.fr       */
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


bool domainBlockSolver()
{
	
}

int main(int argc, char **argv){

	std::ifstream	config_file;
	std::size_t		start;
	std::string		line;
	bool			client_request_accquired = false;
	

	try{
		
		if (argc != 2) //only 2 arguments are required: ./webserv and [config_file]. anything else is invalid
			throw "Invalid number of arguments.";
		config_file.open (argv[1]);
		if (!config_file) //if the config file cannot be opened/doesn't exist, throw this error.
			throw std::exception();
		while (std::getline(config_file, line))//main loop, continue until text ends
		{
			if ()
		}
	}
	catch(std::string error_msg){
		std::cout << "The parser ran into a problem: " << error_msg << std::endl;
	}
	if (config_file.is_open())
		config_file.close();
}
