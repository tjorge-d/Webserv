# include "../includes/BindingSocket.hpp"
# include "../includes/ListeningSocket.hpp"
# include "../includes/Socket.hpp"

int main()
{
    try
    {
        ListeningSocket a(INADDR_ANY, 8080, 10);
    }
    catch(const std::exception &e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
	}
}