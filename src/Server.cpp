#include "../includes/Server.hpp"
#include "../includes/Logger.hpp"
#include <signal.h>
#include <vector>

// Static member initialization
Server* Server::instance = NULL;

// CONSTRUCTORS & DESTRUCTORS
Server::Server(HttpInfo* config) :
    running(true),
    freeze(false),
    print(false),
    config(config),
    serverBlocks(),
    clients(),
    eventHandler(NULL)
{
    instance = this; // Set global instance for signal handling
    
    try {
        // Create server blocks
        serverBlocks = createServerBlocks(*config);
        
        // Create event handler
        eventHandler = new EventHandler(clients, serverBlocks, MAX_CONNECTIONS);
        
        // Setup signal handlers
        setupSignalHandlers();
        
    } catch (const std::exception& e) {
        cleanup();
        throw;
    }
}

Server::~Server()
{
    cleanup();
    instance = NULL;
}

// SIGNAL HANDLERS
void Server::stopSignalHandler(int signal)
{
    (void)signal;
    if (instance)
        instance->stop();
}

void Server::freezeSignalHandler(int signal)
{
    (void)signal;
    if (instance)
        instance->setFreeze(!instance->isFrozen());
}

void Server::printSignalHandler(int signal)
{
    (void)signal;
    if (instance)
        instance->setPrint(true);
}

void Server::setupSignalHandlers()
{
    signal(SIGINT, stopSignalHandler);
    signal(SIGQUIT, freezeSignalHandler);
    signal(SIGTSTP, printSignalHandler);
}

// HELPER METHODS
std::map<int, ServerBlock*> Server::createServerBlocks(HttpInfo &server_info)
{
    std::map<int, ServerBlock*> server_blocks;

    for (std::map<std::string, ServerBlockInfo>::iterator i = server_info.server_blocks.begin(); 
         i != server_info.server_blocks.end(); ++i)
    {
        ServerBlock *new_server_block = NULL;
        try {
            new_server_block = new ServerBlock(i->second, server_info.client_max_body_size);
            server_blocks[new_server_block->getListenerFD()] = new_server_block;

            // Create upload directories for each location
            for (std::map<std::string, LocationBlockInfo>::iterator loc_it = i->second.locations.begin();
                 loc_it != i->second.locations.end(); ++loc_it) {
                std::string uploadPath = i->second.server_root + loc_it->second.location + "upload";
                // Ensure no double slash
                size_t pos = uploadPath.find("//");
                while (pos != std::string::npos) {
                    uploadPath.erase(pos, 1);
                    pos = uploadPath.find("//", pos);
                }
                // Create directory if not exists
                struct stat st;
                if (stat(uploadPath.c_str(), &st) == -1) {
                    mkdir(uploadPath.c_str(), 0755);
                }
            }
        }
        catch (const std::exception& e) {
            // Clean up the current block if it was created
            delete new_server_block;
            
            // Clean up any previously created blocks
            for (std::map<int, ServerBlock*>::iterator j = server_blocks.begin(); j != server_blocks.end(); ++j) {
                delete j->second;
            }
            server_blocks.clear();
            
            throw;
        }
    }
    return server_blocks;
}

void Server::pendingClients()
{
    std::vector<int> clientsToDelete;

    for (std::map<int, Client *>::iterator i = clients.begin(); i != clients.end(); ++i)
    {
        try
        {
            // Check for timeout first
            if (i->second->isTimedOut())
            {
                Logger::log(INFO, "Client timed out: FD " + intToString(i->second->getFD()));
                clientsToDelete.push_back(i->first);
                continue;
            }

            switch (i->second->getState())
            {
            case RECIEVING_REQUEST:
                i->second->recieveRequestChunk();
                break;

            case SENDING_HEADER:
                i->second->sendHeaderChunk();
                break;

            case SENDING_BODY:
                i->second->sendBodyChunk();
                break;

            case DONE:
                clientsToDelete.push_back(i->first);
                break;

            default:;
            };
        }
        catch (const std::exception &e)
        {
            Logger::log(ERROR, "Client error: " + std::string(e.what()) + " for FD " + intToString(i->second->getFD()));
            clientsToDelete.push_back(i->first);
        };
    }

    // Delete clients after iteration
    for (std::vector<int>::iterator it = clientsToDelete.begin(); it != clientsToDelete.end(); ++it)
    {
        eventHandler->deleteClient(*it);
    }
}

void Server::cleanup()
{
    // Clean up clients
    for (std::map<int, Client *>::iterator i = clients.begin(); i != clients.end(); i++) {
        if (i->second)
            delete i->second;
    }
    clients.clear();
    
    // Clean up server blocks
    for (std::map<int, ServerBlock*>::iterator i = serverBlocks.begin(); i != serverBlocks.end(); i++) {
        if (i->second)
            delete i->second;
    }
    serverBlocks.clear();
    
    // Clean up event handler
    delete eventHandler;
    eventHandler = NULL;
    
    // Clean up config
    delete config;
    config = NULL;
}

// MAIN OPERATIONS
void Server::run()
{
    Logger::log(INFO, "Server starting...");
    
    while (running) {
        try {
            // Wait for events
            eventHandler->waitEvents(0);
            
            // Process events
            eventHandler->checkEvents();
            
            // Handle pending clients
            pendingClients();
            
            // Handle debugging tools
            while (freeze) {
                // Busy wait while frozen (could be improved with proper pause/resume)
            }
            
            if (print) {
                std::cout << "Clients connected: " << eventHandler->getConnections() << std::endl;
                std::cout << "Clients map size: " << clients.size() << std::endl;
                print = false;
            }
        }
        catch (const std::exception &e) {
            Logger::log(ERROR, "Server loop error: " + std::string(e.what()));
        }
    }

    Logger::log(INFO, "Server shutting down...");
}

void Server::stop()
{
    running = false;
}