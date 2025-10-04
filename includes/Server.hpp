#ifndef SERVER_HPP
# define SERVER_HPP

# include "Webserv.h"
# include "EventHandler.hpp"
# include "Client.hpp"
# include "ServerBlock.hpp"

class Server {
private:
    // Server state
    bool running;
    bool freeze;
    bool print;
    
    // Server data
    HttpInfo* config;
    std::map<int, ServerBlock*> serverBlocks;
    std::map<int, Client*> clients;
    EventHandler* eventHandler;

    // Signal handling methods
    static Server* instance; // For signal handling
    static void stopSignalHandler(int signal);
    static void freezeSignalHandler(int signal);
    static void printSignalHandler(int signal);

    // Helper methods
    void setupSignalHandlers();
    void cleanup();
    std::map<int, ServerBlock*> createServerBlocks(HttpInfo &server_info);
    void pendingClients();

public:
    Server(HttpInfo* config);
    ~Server();

    // Main server operations
    void run();
    void stop();

    // State management
    bool isRunning() const { return running; }
    void setFreeze(bool state) { freeze = state; }
    void setPrint(bool state) { print = state; }
    bool isFrozen() const { return freeze; }
    bool shouldPrint() const { return print; }
};

#endif