#include <SFML\Network.hpp>
#include <iostream>
#include <list>

enum Commands { FIRST_PEER, CONNECT_TO };
struct ConnectionInfo {
	int port;
	std::string ip;
};


struct Datos {
	int edad;
	int altura;
};


void ControlServidor()
{
	bool running = true;
	sf::Packet packet;
	// Create a socket to listen to new connections
	sf::TcpListener listener;
	sf::Socket::Status status = listener.listen(50000);
	if (status != sf::Socket::Done)
	{
		std::cout << "Error al abrir listener\n";
		exit(0);
	}
	// Create a list to store the future connections
	std::list<sf::TcpSocket*> connections;
	//vector to save the peer's info struct 
	std::vector<ConnectionInfo> connectionsInfo;
	// Create a selector
	sf::SocketSelector selector;
	// Add the listener to the selector
	selector.add(listener);
	// Endless loop that waits for new connections
	while (running)
	{
		// Make the selector wait for data on any socket
		if (selector.wait())
		{
			// Test the listener
			if (selector.isReady(listener))
			{
				// The listener is ready: there is a pending connection
				sf::TcpSocket* client = new sf::TcpSocket;
				if (listener.accept(*client) == sf::Socket::Done)
				{
					// Add the new client to the clients list
					std::cout << "Llega el cliente con puerto: " << client->getRemotePort() << std::endl;
					packet.clear();
					if (connections.empty())
					{
						packet << static_cast<int32_t>(Commands::FIRST_PEER);
						client->send(packet);
					}
					else
					{
						int size = connectionsInfo.size();
						packet << static_cast<int32_t>(Commands::CONNECT_TO) << size;
						for (int i = 0; i < size; i++)
						{
							packet << connectionsInfo[i].ip << connectionsInfo[i].port;
						}
						client->send(packet);
					}
					connections.push_back(client);
					// Add the new client to the selector so that we will
					// be notified when he sends something
					selector.add(*client);
					ConnectionInfo aux;
					aux.ip = client->getRemoteAddress().toString();
					aux.port = (int)client->getRemotePort();
					connectionsInfo.push_back(aux);
					if (connections.size() >= 3) {
						while (!connections.empty())
						{
							connections.front()->disconnect();
							connections.pop_front();
							connectionsInfo.pop_back();

						}
					}
				}
				else
				{
					// Error, we won't get a new connection, delete the socket
					std::cout << "Error al recoger conexión nueva\n";
					delete client;
				}
			}
			else
			{
				// The listener socket is not ready, test all other sockets (the clients)
				for (std::list<sf::TcpSocket*>::iterator it = connections.begin(); it != connections.end(); ++it)
				{
					sf::TcpSocket& client = **it;
					if (selector.isReady(client))
					{
						// The client has sent some data, we can receive it
						status = client.receive(packet);
						if (status == sf::Socket::Done)
						{
							std::string strRec;
							packet >> strRec;
							std::cout << "He recibido " << strRec << " del puerto " << client.getRemotePort() << std::endl;
						}
						else if (status == sf::Socket::Disconnected)
						{
							selector.remove(client);
							std::cout << "Elimino el socket que se ha desconectado\n";
						}
						else
						{
							std::cout << "Error al recibir de " << client.getRemotePort() << std::endl;
						}
					}
				}
			}
		}
	}

}

void ControlClient() {
	sf::TcpSocket socket;
	sf::Socket::Status status = socket.connect("localhost", 50000, sf::milliseconds(15.f));
	sf::SocketSelector selector;
	if (status != sf::Socket::Done)
	{
		std::cout << "Error al establecer conexion\n";
		exit(0);
	}
	else
	{
		std::cout << "Se ha establecido conexion\n";
	}
	bool waiting = true;
	int aux;
	Commands command;
	int clientPort;
	std::vector<sf::TcpSocket*> connections;
	sf::Packet packet;
	while (waiting)
	{
		if (socket.receive(packet) == sf::Socket::Done)
		{
			packet >> aux;
			command = (Commands)aux;
			switch (command) {
			case Commands::FIRST_PEER: {
				std::cout << "You are the first player to join. Waiting for other players" << std::endl;
				clientPort = socket.getLocalPort();
				socket.disconnect();
				waiting = false;
				break;
			}
			case Commands::CONNECT_TO: {
				packet >> aux;
				int port;
				std::string ip;
				for (int i = 0; i < aux; i++)
				{
					packet >> ip >> port;
					sf::TcpSocket* sock = new sf::TcpSocket;
					sf::Socket::Status st = sock->connect(ip, port, sf::seconds(5.f));
					connections.push_back(sock);
				}
				clientPort = socket.getLocalPort();
				socket.disconnect();
				waiting = false;
				break;
			}
			default:
				break;
			}
		}
	}
	if (connections.size() < 2) {
		std::cout << "Waiting for players" << std::endl;
		sf::TcpListener listener;
		sf::Socket::Status newStatus = listener.listen(clientPort);
		//selector.add(listener); 
		listener.setBlocking(false);
		if (newStatus == sf::Socket::Done)
		{
			while (connections.size() < 2)
			{
				sf::TcpSocket* newPlayer = new sf::TcpSocket;
				sf::Socket::Status newPlayerStatus = listener.accept(*newPlayer);
				if (newPlayerStatus == sf::Socket::Done)
				{
					std::cout << "A new player has joined" << std::endl;
					connections.push_back(newPlayer);
				}
			}
		}

	}
	std::cout << "Hola" << std::endl;
	bool connected = true;
	while (connected)
	{
		if (1 == 2) connected = false;
	}
	//system("pause");
	socket.disconnect();
}
int main()
{
	std::cout << "¿Seras servidor (s) o cliente (c)? ... ";
	char c;
	std::cin >> c;

	if (c == 's')
	{
		ControlServidor();
	}
	else if (c == 'c')
	{
		ControlClient();
	}
	else
	{
		exit(0);
	}


}