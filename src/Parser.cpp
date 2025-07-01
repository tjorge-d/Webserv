/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmiguel- <lmiguel-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:16:29 by lmiguel-          #+#    #+#             */
/*   Updated: 2025/07/01 14:07:31 by lmiguel-         ###   ########.fr       */
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
bool	errorCodeChecker()
{
	
} */

static void setupFailsafes(HttpInfo *Server)
{
	Server->failsafe_error_codes["200"] = "200 OK"; //The request succeeded.
	Server->failsafe_error_codes["204"] = "204 No Content"; //There is no content to send for this request, but the headers are useful. 
	Server->failsafe_error_codes["301"] = "301 Moved Permanently"; //The URL of the requested resource has been changed permanently. The new URL is given in the response (necessary?)
	Server->failsafe_error_codes["303"] = "303 See Other"; //The server sent this response to direct the client to get the requested resource at another URI with a GET request.
	Server->failsafe_error_codes["308"] = "308 Permanent Redirect"; //This means that the resource is now permanently located at another URI, specified by the Location response header.
	//This has the same semantics as the 301 Moved Permanently HTTP response code, with the exception that the user agent must not change the HTTP method used: if a POST was used in the first request, a POST must be used in the second request.
	Server->failsafe_error_codes["400"] = "400 Bad Request"; //The server cannot or will not process the request due to something that is perceived to be a client error (e.g., malformed request syntax, invalid request message framing, or deceptive request routing).
	Server->failsafe_error_codes["403"] = "403 Forbidden"; //The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource. Unlike 401 Unauthorized, the client's identity is known to the server.
	Server->failsafe_error_codes["404"] = "404 Not Found"; //The server cannot find the requested resource. In the browser, this means the URL is not recognized. In an API, this can also mean that the endpoint is valid but the resource itself does not exist.
	Server->failsafe_error_codes["405"] = "405 Method Not Allowed"; //The request method is known by the server but is not supported by the target resource. For example, an API may not allow DELETE on a resource, or the TRACE method entirely.
	Server->failsafe_error_codes["408"] = "408 Request Timeout"; //This response is sent on an idle connection by some servers, even without any previous request by the client. It means that the server would like to shut down this unused connection.
	Server->failsafe_error_codes["409"] = "409 Conflict"; //This response is sent when a request conflicts with the current state of the server. 
	Server->failsafe_error_codes["411"] = "411 Length Required"; //Server rejected the request because the Content-Length header field is not defined and the server requires it.
	Server->failsafe_error_codes["413"] = "413 Content Too Large"; //The request body is larger than limits defined by server. The server might close the connection or return an Retry-After header field.
	Server->failsafe_error_codes["429"] = "429 Too Many Requests"; //The user has sent too many requests in a given amount of time (rate limiting).
	Server->failsafe_error_codes["431"] = "431 Request Header Fields Too Large"; //The server is unwilling to process the request because its header fields are too large.
	Server->failsafe_error_codes["500"] = "500 Internal Server Error"; //The server has encountered a situation it does not know how to handle. This error is generic, indicating that the server cannot find a more appropriate 5XX status code to respond with.
	Server->failsafe_error_codes["503"] = "503 Service Unavailable"; //The server is not ready to handle the request.
	Server->failsafe_error_codes["504"] = "504 Gateway Timeout"; //This error response is given when the server is acting as a gateway and cannot get a response in time.
	Server->failsafe_error_codes["505"] = "505 HTTP Version Not Supported"; //The HTTP version used in the request is not supported by the server.
}

static void setupServices(HttpInfo *Server, LocationBlockInfo *locationBlock, std::string acquired_services)
{
	std::string						current_method;
	std::stringstream				stream(acquired_services);
	

	// check valid methods (also stringstream practice)
	while (stream >> current_method)
	{
		if (current_method != "GET" && current_method != "POST" && current_method != "DELETE" && current_method != "HEAD" &&
			current_method != "OPTION" && current_method != "PUT" && current_method != "TRACE" && current_method != "CONNECT")
			throw ParserException(Server, "Attempt to configure invalid service. Allowed services are: GET POST DELETE HEAD OPTION PUT TRACE CONNECT");
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
/* 	std::cout << "SIZE: " << body_size <<std::endl;
	std::cout << "UNIT: " << unit <<std::endl; */
	unconverted_maxbodysize = atoi(body_size.c_str());
	if (unit == "Mb" || unit == "mb" || unit == "mb;" || unit == "Mb;")
		unconverted_maxbodysize = unconverted_maxbodysize * 1024 * 1024;
	else if (unit == "Kb" || unit == "kb" || unit == "kb;" || unit == "Kb;")
		unconverted_maxbodysize = unconverted_maxbodysize * 1024;
	else
		throw ParserException(webserver, "Invalid client request body size storage unit, please use Mb/mb or Kb/kb as the storage unit.");
	if (unconverted_maxbodysize <= 0 || unconverted_maxbodysize > INT_MAX)
		throw ParserException(webserver, "Invalid maximum client request body size.");
	webserver->client_max_body_size = unconverted_maxbodysize;
}

HttpInfo *config_parser(char *file_path, int argc)
{
	HttpInfo *Server = new HttpInfo();

	Server->parser_info.max_size_acquired = false;
	Server->parser_info.server_setup_mode = false;
	Server->parser_info.location_setup_mode = false;
	if (argc > 2)
		throw ParserException(Server, "Invalid number of arguments.");
	if (argc == 1)
		Server->parser_info.config_file.open("config_file_default.txt");
	else
		Server->parser_info.config_file.open(file_path);
	if (!Server->parser_info.config_file.is_open()) // if the config file cannot be opened/doesn't exist, throw this error.
		throw ParserException(Server, "Invalid config file.");
	if (!Server->parser_info.config_file.good())
	{
		Server->parser_info.config_file.close();
		throw ParserException(Server, "Error while openning file.");
	}
	if (Server->parser_info.config_file.is_open() && Server->parser_info.config_file.peek() == EOF)
		throw ParserException(Server, "Your config file is empty.");
	setupFailsafes(Server);
	while (std::getline(Server->parser_info.config_file, Server->parser_info.line))
	{ // main loop, continue until text ends
		if (Server->parser_info.line.size() > 0 && Server->parser_info.line[Server->parser_info.line.size() - 1] != ';')
		{
			std::cout << Server->parser_info.line << std::endl;
			throw ParserException(Server, "All lines must end in the ';' character.");
		}
		//----------------------------SERVER SPECIFIC INFO, DOES NOT REQUIRE A SERVERBLOCK TO EXIST----------------------------
		parseServerInfo(Server);
		//----------------------------SERVERBLOCK SPECIFIC INFO, REQUIRES A SERVERBLOCK TO EXIST----------------------------
		parseServerBlock(Server);
		//----------------------------LOCATION BLOCK SPECIFIC INFO, REQUIRES A LOCATION BLOCK TO EXIST (REMEMBER LOCATION BLOCKS REQUIRE SERVERBLOCKS TO EXIST)----------------------------
		parseLocationBlock(Server);
		Server->parser_info.newline += Server->parser_info.line;
		Server->parser_info.newline += '\n';
	}
	setupClientmaxbodysize(Server, Server->parser_info.acquired_max_body_size);


	//----------------------------IF ANYTHING GOES WRONG, UNCOMMENT THIS AND FIGURE OUT WHERE THE PARSING GOES WRONG----------------------------

	/* 
	std::cout << "Client max body size : " << Server->client_max_body_size << std::endl;
	for (std::map<std::string, ServerBlockInfo>::iterator i = Server->server_blocks.begin(); \
		i != Server->server_blocks.end(); ++i)
	{
		std::cout << "Server Block : " << i->second.server_name << std::endl;
		std::cout << "Server Port : " << i->second.port << std::endl;
		std::cout << "Redirect Directory : " << i->second.redirect_directory << std::endl;
		for (std::map<int, std::string>::iterator k = i->second.error_codes.begin(); \
		k != i->second.error_codes.end(); k++)
			std::cout << "Error code : " << k->first << " Error index file : " << k->second << std::endl;
		for (std::map<std::string, LocationBlockInfo>::iterator j = i->second.locations.begin(); \
			j != i->second.locations.end(); j++)
		{
			std::cout << "location Block : " << j->first << std::endl;
			std::cout << "location Autoindex State : " << j->second.autoindex << std::endl;
			std::cout << "Root Directory : " << j->second.root_directory << std::endl;
			std::cout << "Index File : " << j->second.index_file << std::endl;
			std::cout << "Allowed Services : " << std::endl;
			for (std::vector<std::string>::iterator m = j->second.allowed_services.begin(); \
			m != j->second.allowed_services.end(); m++)
				std::cout << *m << std::endl;
		}
	}
	for (std::map<std::string, std::string>::iterator i = Server->failsafe_error_codes.begin();
		i != Server->failsafe_error_codes.end(); ++i)
	{
		std::cout << "Error code : " << i->first << std::endl;
		std::cout << "Error message : " << i->second << std::endl;
	}
	*/

	//----------------------------PARSER TROUBLESHOOTER ENDS HERE----------------------------


	Server->parser_info.config_file.close();
	return (Server);
}

void parseServerInfo(HttpInfo *Server){

	if ((Server->parser_info.line.find("client_max_body_size")) != std::string::npos)
	{
		if (Server->parser_info.max_size_acquired == false)
		{
			Server->parser_info.acquired_max_body_size = Server->parser_info.line.substr((Server->parser_info.line.find(' ') + 1), Server->parser_info.line.rfind(' '));
			if (atoi(Server->parser_info.acquired_max_body_size.c_str()) <= 0)
				throw ParserException(Server, "Your client_max_body_size is invalid.");
			Server->parser_info.max_size_acquired = true;
		}
		else
			throw ParserException(Server, "Multiple client_max_body_size detected.");
	}
	if ((Server->parser_info.line.find("server_block start;")) != std::string::npos)
	{
		if (Server->parser_info.server_setup_mode == false)
		{
			Server->parser_info.server_setup_mode = true;
		}
		else
			throw ParserException(Server, "Attempt to create server block inside another server block.");
	}
	if ((Server->parser_info.line.find("server_block end;")) != std::string::npos)
	{
		if (Server->parser_info.server_setup_mode == true)
		{
			Server->parser_info.server_setup_mode = false;
			//Server->server_blocks.push_back(current_server_block);
			Server->server_blocks[Server->parser_info.current_server_block.server_name] = Server->parser_info.current_server_block;
			Server->parser_info.current_server_block = ServerBlockInfo();
		}
		else
			throw ParserException(Server, "Attempt to finish nonexistent server block.");
	}
}

void	parseServerBlock(HttpInfo *Server){
	
	if ((Server->parser_info.line.find("location_block start;")) != std::string::npos)
		{
			if (Server->parser_info.server_setup_mode == true && Server->parser_info.location_setup_mode == false)
				Server->parser_info.location_setup_mode = true;
			else if (Server->parser_info.location_setup_mode == true)
				throw ParserException(Server, "Attempt to create location block inside another location block.");
			else
				throw ParserException(Server, "Attempt to create location block outside of server block.");
		}
		if ((Server->parser_info.line.find("location_block end;")) != std::string::npos)
		{
			if (Server->parser_info.location_setup_mode == true)
			{
				Server->parser_info.location_setup_mode = false;
				//current_server_block.locations.push_back(current_location_block);
				Server->parser_info.current_server_block.locations[Server->parser_info.current_location_block.location] = Server->parser_info.current_location_block;
				Server->parser_info.current_location_block = LocationBlockInfo();
			}
			else
				throw ParserException(Server, "Attempt to finish nonexistent location block.");
		}
		if ((Server->parser_info.line.find("domain_port")) != std::string::npos)
		{
			if (Server->parser_info.server_setup_mode == true)
				Server->parser_info.current_server_block.port = atoi((Server->parser_info.line.substr((Server->parser_info.line.rfind(' ') + 1),
					Server->parser_info.line.size() - Server->parser_info.line.rfind(' ') - 2)).c_str());
			else
				throw ParserException(Server, "Attempting to set up ports on nonexistent server block.");
		}
		if ((Server->parser_info.line.find("server_name")) != std::string::npos)
		{
			if (Server->parser_info.server_setup_mode == true)
			{
				Server->parser_info.current_server_block.server_name = Server->parser_info.line.substr((Server->parser_info.line.rfind(' ') + 1),
					Server->parser_info.line.size() - Server->parser_info.line.rfind(' ') - 2);
				if (Server->parser_info.current_server_block.server_name.empty())
					throw ParserException(Server, "Your server name is empty.");
			}
			else
				throw ParserException(Server, "Attempting to set up server name on nonexistent server block.");
		}
		if ((Server->parser_info.line.find("directory_request_redirect")) != std::string::npos)
		{
			if (Server->parser_info.server_setup_mode == true)
			{
				Server->parser_info.current_server_block.redirect_directory = Server->parser_info.line.substr((Server->parser_info.line.rfind(' ') + 1), 
					Server->parser_info.line.size() - Server->parser_info.line.rfind(' ') - 2);
				if (Server->parser_info.current_server_block.redirect_directory.empty())
					throw ParserException(Server, "Your directory request redirection file is invalid/nonexistent.");
			}
			else
				throw ParserException(Server, "Attempting to set up directory request redirection file on nonexistent server block.");
		}
		if ((Server->parser_info.line.find("error_page")) != std::string::npos)
		{
			if (Server->parser_info.server_setup_mode == true)
			{
				Server->parser_info.current_server_block.error_codes.insert(std::pair<int, std::string>(atoi(Server->parser_info.line.substr((Server->parser_info.line.find(' ') + 1),
					Server->parser_info.line.size() - Server->parser_info.line.rfind(' ') - 2).c_str()),
					Server->parser_info.line.substr((Server->parser_info.line.rfind(' ') + 1), Server->parser_info.line.size() - Server->parser_info.line.rfind(' ') - 2)));
				for (std::map<int, std::string>::iterator it = Server->parser_info.current_server_block.error_codes.lower_bound(0); it != Server->parser_info.current_server_block.error_codes.upper_bound(1000); ++it)
				{
					if (it->first <= 0 || it->second.empty())
						throw ParserException(Server, "Your error code or coresponding page is invalid/nonexistent.");
					std::string blame98 = "./var/www/dev" + it->second;
					std::ifstream error_file(blame98.c_str());
					if (!error_file.good()){
						//delete Server;
						throw ParserException(Server, "One of your provided HTML code error pages does not exist or is invalid.");
					}
				}
			}
			else
				throw ParserException(Server, "Attempting to set up error codes and error pages on nonexistent server block.");
		}
}

void parseLocationBlock(HttpInfo *Server){

	if ((Server->parser_info.line.find("autoindex")) != std::string::npos)
		{
			if (Server->parser_info.location_setup_mode == true)
			{
				if (Server->parser_info.line.substr((Server->parser_info.line.rfind(' ') + 1), Server->parser_info.line.size() - Server->parser_info.line.rfind(' ') - 2) == "on")
					Server->parser_info.current_location_block.autoindex = true;
				else if (Server->parser_info.line.substr((Server->parser_info.line.rfind(' ') + 1), Server->parser_info.line.size() - Server->parser_info.line.rfind(' ') - 2) == "off")
					Server->parser_info.current_location_block.autoindex = false;
				else
					throw ParserException(Server, "Invalid autoindexing options, please set it to on or off.");
			}
			else
				throw ParserException(Server, "Attempting to set up autoindexing on nonexistent location block.");
		}
		if ((Server->parser_info.line.find("services_available")) != std::string::npos)
		{
			if (Server->parser_info.location_setup_mode == true)
			{
				Server->parser_info.acquired_services = Server->parser_info.line.substr((Server->parser_info.line.find(' ') + 1), 
					Server->parser_info.line.size() - Server->parser_info.line.find(' ') - 2);
				if (Server->parser_info.acquired_services.empty())
					throw ParserException(Server, "Your allowed services are empty.");
				setupServices(Server, &Server->parser_info.current_location_block, Server->parser_info.acquired_services);
			}
			else
				throw ParserException(Server, "Attempting to set up allowed services on nonexistent domain block.");
		}
		if ((Server->parser_info.line.find("index_file")) != std::string::npos)
		{
			if (Server->parser_info.location_setup_mode == true)
			{
				Server->parser_info.current_location_block.index_file = Server->parser_info.line.substr((Server->parser_info.line.find(' ') + 1),
					Server->parser_info.line.size() - Server->parser_info.line.find(' ') - 2);
				if (Server->parser_info.current_location_block.index_file.empty())
					throw ParserException(Server, "Your location index file is invalid/empty.");
			}
			else
				throw ParserException(Server, "Attempting to set up location index file on nonexistent location block.");
		}
		if ((Server->parser_info.line.find("default_location")) != std::string::npos)
		{
			if (Server->parser_info.location_setup_mode == true)
			{
				Server->parser_info.current_location_block.location = Server->parser_info.line.substr((Server->parser_info.line.find(' ') + 1),
					Server->parser_info.line.size() - Server->parser_info.line.find(' ') - 2);
				if (Server->parser_info.current_location_block.location.empty())
					throw ParserException(Server, "Your location location is empty.");
			}
			else
				throw ParserException(Server, "Attempting to set up a location on nonexistent location block.");
		}
		if ((Server->parser_info.line.find("root")) != std::string::npos)
		{
			if (Server->parser_info.location_setup_mode == true)
			{
				Server->parser_info.current_location_block.root_directory = Server->parser_info.line.substr((Server->parser_info.line.find(' ') + 1),
					Server->parser_info.line.size() - Server->parser_info.line.find(' ') - 2);
				if (Server->parser_info.current_location_block.root_directory.empty())
					throw ParserException(Server, "Your root directory is empty.");
			}
			else
				throw ParserException(Server, "Attempting to set up root directory on nonexistent location block.");
		}
}

std::string trim(const std::string &s) {
	size_t start = 0;
	while (start < s.size() && std::isspace(s[start])){
		++start;
	}

	size_t end = s.size();
	while (end > start && std::isspace(s[end - 1])){
		--end;
	}

	return (s.substr(start, end - start));
}

ParserException::ParserException(HttpInfo *server, std::string error) :
std::runtime_error("The parser ran into a problem: " + error){
	delete server;
};
