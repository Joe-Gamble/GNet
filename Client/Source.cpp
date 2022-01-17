//Client Code

#include <GNet/IncludeMe.h>
#include <iostream>

using namespace GNet;

int main()
{
	if (Network::Initialize())
	{
		std::cout << "Winsock api successfully initialized." << std::endl;

		Socket socket;
		if (socket.Create() == GResult::G_SUCCESS)
		{
			std::cout << "Socket successfully created." << std::endl;
			socket.Close();
		}
		else
		{
			std::cerr << "Socket failed to create." << std::endl;
		}
	}
	Network::Shutdown();
	system("Pause");
	return 0;
}