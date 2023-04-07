# include <iostream>
# include "network/server.hpp"




# if defined(__NETV) 
int main(int argc, char* argv[])
{

    // =>> Check if a port number is provided as a command line argument <<=
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " port\n\n" 
            << "Commands: \r\n" << "  /hello   \t\t\tprints welcome message to all connected sockets|\r\n" 
            << "  /info    \t\t\tget info|\r\n"
            << "  /socks   \t\t\tget connected sockets|\r\n"
            << "  /online     \t\t\tget online users count|\r\n"
            << "  /send `recipient` `message`\tsend private message to recipient|\r\n" << std::endl;
        exit(1);
    }

    // =>> Convert the command line argument to an integer 
    int port = std::stoi(argv[1]); // ^^ string .. to .. integer
     
    try
    {
        Server server(port);
        server.run();
        

    }
    catch (std::exception& ex)
    {
        spdlog::info("{}", ex.what());
    }
    catch (netv::system_error& error)
    {
        spdlog::warn("{}", error.what());
    }

    return 0;
}
# endif 

