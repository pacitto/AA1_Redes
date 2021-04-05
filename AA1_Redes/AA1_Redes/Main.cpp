#include <SFML\Network.hpp>
#include <iostream>
#include <list>

enum Commands {LOG_IN, CREATE_ROOM, JOIN_ROOM, ASK_ROOM, CHOOSE_ROOM, FIRST_PEER, CONNECT_TO }; //comandos para comunicación
enum Headers {PEER_NUM, READY, PLAYER_ORD};
enum Culture {ARAB, BANTU, CHINA, ESQUIMAL, INDIA, MEXICAN, TIROLESE};
enum Family {GRANDFATHER, GRANDMOTHER, FATHER, MOTHER, SON, DAUGHTER};

struct ConnectionInfo {
	int port;
	std::string ip;
	sf::TcpSocket* socket; 
};
//Struct para carta con constructor
struct Card {
	int id;
	Culture culture;
	Family family;

	Card(int _id, Culture _culture, Family _family)
	{
		id = _id;
		culture = _culture;
		family = _family; 
	}
};
//struct de player con un vector de cartas para guardar su mano
struct Player {
	sf::TcpSocket* socket;
	int id;
	std::vector<Card> hand; 
};
struct Room {
	std::string id;
	int numPlayers;
	bool hasPass;
	std::string password; 
	std::vector<ConnectionInfo> connections; 

	Room(std::string _id, int _numPlayers, bool _hasPass, std::string _password)
	{
		id = _id;
		numPlayers = _numPlayers;
		hasPass = _hasPass;
		password = _password; 
	}
};

//Incialización de la baraja de cartas
void InitializeDeck(std::vector<Card>* deck)
{
	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			deck->push_back(Card{(j + 1)+(i * 6), (Culture)i, (Family)j});
		}
	}
}

//asks players if ready and sends info back and forth
bool Ready() {

	char c = 'n';
	while (c != 'y') {

		std::cout << "Ready? (y/n)" << std::endl;
		std::cin >> c;

	}
	return true;
}

//Crear función para dar un número de cartas aleatorias del deck a los jugadores
void DealCards(std::vector<Card> deck, std::vector<Card>* hand, int numPlayers, int playerId)
{
	int i = 1; 
	int random; 
	while (!deck.empty())
	{
		random = rand() % deck.size(); 
		Card newCard = deck[random]; 

		if (i == playerId)
		{
			hand->push_back(newCard); 
		}
		
		deck.erase(deck.begin() + random);

		i++; 
		if (i > numPlayers)
		{
			i = 1; 
		}

	}
}

//enseñar cartas por familias y si hay una familia completa dar el punto al jugador
void ShowHand(std::vector<Card> hand)
{
	std::string culture, family; 
	for (int i = 0; i < hand.size(); i++)
	{
		switch (hand[i].culture) {
		case Culture::ARAB:
			culture = "Arab";
			break;
		case Culture::BANTU:
			culture = "Bantu";
			break;
		case Culture::CHINA:
			culture = "China";
			break;
		case Culture::ESQUIMAL:
			culture = "Esquimal";
			break;
		case Culture::INDIA:
			culture = "India";
			break;
		case Culture::MEXICAN:
			culture = "Mexican";
			break;
		case Culture::TIROLESE:
			culture = "Tirolese";
			break;
		default: break;
		}

		switch (hand[i].family) {
		case Family::DAUGHTER:
			family = "Daughter";
			break;
		case Family::FATHER:
			family = "Father";
			break;
		case Family::GRANDFATHER:
			family = "Grandfather";
			break;
		case Family::GRANDMOTHER:
			family = "Grandmother";
			break;
		case Family::MOTHER:
			family = "Mother";
			break;
		case Family::SON:
			family = "Son";
			break;
		default: break;
		}
		
		std::cout << i + 1 << ". " << family << " from " << culture << std::endl; 
	}
}

void ControlServidor()   
{
	bool running = true;
	int aux; 
	std::string name;
	std::string password;
	bool hasPass;
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
	std::vector<Room> rooms; 
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
					packet.clear();
					packet << static_cast<int32_t>(Commands::LOG_IN);
					if (rooms.empty()) packet << false;
					else packet << true; 
					client->send(packet); 
					connections.push_back(client); 
					selector.add(*client); 
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
							packet >> aux; 
							Commands command = (Commands)aux; 
							switch (command) {
							case Commands::CREATE_ROOM:
							{
								packet >> name >> aux >> hasPass;
								if (hasPass) packet >> password;
								else password = ""; 
								Room newRoom(name, aux, hasPass, password);
								ConnectionInfo newInfo;
								newInfo.socket = *it; 
								newInfo.ip = newInfo.socket->getRemoteAddress().toString(); 
								newInfo.port = (int)newInfo.socket->getRemotePort();  
								newRoom.connections.push_back(newInfo); 
								rooms.push_back(newRoom); 
								packet.clear(); 
								packet << static_cast<int32_t>(Commands::FIRST_PEER) << newRoom.numPlayers; 
								client.send(packet); 
								connections.erase(it); 
								break;
							}
							case Commands::JOIN_ROOM:
							{
								packet.clear(); 
								aux = rooms.size();
								packet << static_cast<int32_t>(Commands::ASK_ROOM) << false << aux;
								int aux2;
								for (int i = 0; i < aux; i++)
								{
									aux2 = rooms[i].connections.size(); 
									packet << rooms[i].id << rooms[i].numPlayers << rooms[i].hasPass << aux2; 
								}
								client.send(packet); 
								break;
							}
							case Commands::CHOOSE_ROOM:
							{
								packet >> name >> hasPass;
								if (hasPass) packet >> password;
								packet.clear(); 
								for (int i = 0; i < rooms.size(); i++)
								{
									if (rooms[i].id == name)
									{
										if (!rooms[i].hasPass || rooms[i].password == password)
										{
											aux = rooms[i].connections.size(); 
											packet << static_cast<int32_t>(Commands::CONNECT_TO) << rooms[i].numPlayers << " " << aux;
											for (int j = 0; j < aux; j++)
											{
												packet << rooms[i].connections[j].ip << rooms[i].connections[j].port; 
											}
											client.send(packet); 
											ConnectionInfo newInfo;
											newInfo.socket = *it;
											newInfo.ip = newInfo.socket->getRemoteAddress().toString();
											newInfo.port = (int)newInfo.socket->getRemotePort();
											rooms[i].connections.push_back(newInfo); 
											if (rooms[i].connections.size() >= rooms[i].numPlayers)
											{
												while (!rooms[i].connections.empty())
												{
													rooms[i].connections.back().socket->disconnect();
													rooms[i].connections.pop_back();
												}
												rooms.erase(rooms.begin() + i); 
											}
											connections.erase(it);
										}
										else if(rooms[i].hasPass)
										{
											aux = rooms.size();
											packet << static_cast<int32_t>(Commands::ASK_ROOM) << true << aux;
											int aux2;
											for (int i = 0; i < aux; i++)
											{
												aux2 = rooms[i].connections.size();
												packet << rooms[i].id << rooms[i].numPlayers << rooms[i].hasPass << aux2;
											}
											client.send(packet);
										}		
									}
								} 
								break;
							}
							default: break;
							}
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
	std::vector<Card> deck;
	InitializeDeck(&deck); 
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
	std::string info; 
	bool checkStatus; 
	Commands command;
	int clientPort;
	int playerNum = 0; 
	int numPlayers = 0; 
	std::vector<Player> players;
	Player playerInfo; 
	sf::Packet packet;
	//Mientras se tenga una conexión con el servidor se compartiran los datos de login para entrar a la partida deseada
	while (waiting)
	{
		if (socket.receive(packet) == sf::Socket::Done)
		{
			packet >> aux;
			command = (Commands)aux;
			switch (command) {
			//Deja escoger al jugador si quiere crear una partida o unirse a una existente
			case Commands::LOG_IN:
			{
				packet >> checkStatus;
				packet.clear();
				if (checkStatus)
				{
					std::cout << "Would you like to create a new game or join an existing one?(1/2)" << std::endl;
					std::cin >> info;
					while (info != "1" && info != "2")
					{
						std::cout << "Invalid input" << std::endl;
						std::cin >> info;
					}
					if (info == "1")
					{
						std::cout << "Enter the room name" << std::endl;
						std::cin >> info;
						packet << static_cast<int32_t> (Commands::CREATE_ROOM) << info;
						std::cout << "Enter the number of players for the game (3 - 6)" << std::endl;
						std::cin >> aux;
						while (aux < 3 || aux > 6)
						{
							std::cout << "Invalid input" << std::endl;
							std::cin >> aux;
						}
						packet << aux;
						std::cout << "Do you want to set a password for the game (y/n)" << std::endl;
						std::cin >> info;
						while (info != "y" && info != "n")
						{
							std::cout << "Invalid input" << std::endl;
							std::cin >> info;
						}
						if (info == "y")
						{
							std::cout << "Enter the game password" << std::endl;
							std::cin >> info;
							packet << true << info;
						}
						else packet << false;
					}
					else packet << static_cast<int32_t> (Commands::JOIN_ROOM);
				}
				else
				{
					std::cout << "There are no available rooms, create a new one" << std::endl; 
					std::cout << "Enter the room name" << std::endl; 
					std::cin >> info; 
					packet << static_cast<int32_t> (Commands::CREATE_ROOM) << info; 
					std::cout << "Enter the number of players for the game (3 - 6)" << std::endl;
					std::cin >> aux; 
					while (aux < 3 || aux > 6)
					{
						std::cout << "Invalid input" << std::endl;
						std::cin >> aux;
					}
					packet << aux; 
					std::cout << "Do you want to set a password for the game (y/n)" << std::endl; 
					std::cin >> info;
					while (info != "y" && info != "n")
					{
						std::cout << "Invalid input" << std::endl; 
						std::cin >> info; 
					}
					if (info == "y")
					{
						std::cout << "Enter the game password" << std::endl;
						std::cin >> info;
						packet << true << info;
					}
					else packet << false; 
				}
				socket.send(packet); 
				break;
			}
			//En caso de querer unirse a una existente, se le muestran las salas activas y puedo escoger a cual unirse
			case Commands::ASK_ROOM: {
				int size, aux2;
				packet >> checkStatus >> size;
				if (checkStatus) std::cout << "Wrong password" << std::endl; 
				std::vector<std::string> roomName; 
				std::vector<bool> hasPass; 
				std::cout << "Choose one of the following games" << std::endl; 
				for (int i = 0; i < size; i++)
				{
					packet >> info >> aux >> checkStatus >> aux2;
					std::cout << i + 1 << ". " << info << " " << aux2 << "/" << aux << " ";
					if (checkStatus) std::cout << "Has password" << std::endl;
					else std::cout << "Has no password" << std::endl;
					roomName.push_back(info);
					hasPass.push_back(checkStatus);
				}
				std::cin >> aux;
				packet.clear();
				packet << static_cast<int32_t> (Commands::CHOOSE_ROOM) << roomName[aux - 1] << hasPass[aux - 1];
				if (hasPass[aux - 1])
				{
					std::cout << "Enter the password" << std::endl;
					std::cin >> info; 
					packet << info; 
				}
				socket.send(packet); 
				break;
			}
			//Se comunica al peer que crea una partida, que es el primero en entrar y por lo tanto el de turno 1
			case Commands::FIRST_PEER: {
				packet >> aux; 
				numPlayers = aux;  
				std::cout << "You are the first player to join. Waiting for other players" << std::endl;
				playerNum = 1; 
				clientPort = socket.getLocalPort();
				socket.disconnect();
				waiting = false;
				break;
			}
			//Se comunica al peer que ha entrado en una partida existente, y se le pasan la información de los otros peers de la partida para conectarse a ellos (ip y port)
			case Commands::CONNECT_TO: {
				packet >> aux >> info;
				numPlayers = aux; 
				packet >> aux; 
				playerNum = aux + 1; 
				int port;
				std::string ip;
				for (int i = 0; i < aux; i++)
				{
					packet >> ip >> port;
					sf::TcpSocket* sock = new sf::TcpSocket;
					sf::Socket::Status st = sock->connect(ip, port, sf::seconds(5.f));
					Player p; 
					p.socket = sock; 
					p.id = i + 1; 
					players.push_back(p);
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
	sf::TcpListener listener;
	sf::Socket::Status newStatus = listener.listen(clientPort);
	listener.setBlocking(false);
	selector.add(listener); 
	//Esperar a que la sala se llene de jugadores
	if (players.size() < numPlayers - 1) {
		std::cout << "Waiting for players" << std::endl;
		if (newStatus == sf::Socket::Done)
		{
			while (players.size() < numPlayers - 1)
			{
				sf::TcpSocket* newPlayer = new sf::TcpSocket;
				sf::Socket::Status newPlayerStatus = listener.accept(*newPlayer);
				if (newPlayerStatus == sf::Socket::Done)
				{
					std::cout << "A new player has joined" << std::endl;
					Player p;
					p.socket = newPlayer;
					p.id = players.size() + 2; 
					players.push_back(p);
				}
			}
		}

	}

	DealCards(deck, &playerInfo.hand, numPlayers, playerNum);
	ShowHand(playerInfo.hand);
	
	if (Ready()) {
		packet.clear();
		for (auto it = players.begin(); it != players.end(); it++) {
			Player p = *it;
			packet << static_cast<int32_t>(Headers::READY) << 1;
			for (int i = 0; i < players.size(); i++)
			{
				players[i].socket->send(packet);
			}
		}
	}

	bool ready = false;
	int numready = 0;
	while (!ready) {
		packet.clear();
		for (auto it = players.begin(); it != players.end(); it++)
		{
			Player p = *it;
			status = p.socket->receive(packet);
			if (status == sf::Socket::Done) {
				Headers header;
				packet >> aux;
				header = (Headers)aux;
				if (aux == Headers::READY) {
					numready++;
				}
				if (numready == players.size() - 1) {
					ready = true;
				}
			}
			else if (status == sf::Socket::Disconnected)
			{
				std::cout << "A player has disconnected" << std::endl;
				p.socket->disconnect();
			}
		}
	}


	//Si eres el que crea la sala, envia a los otros peers su numero (indicando su orden en la partida)
	if (playerNum == 1) 
	{
		std::cout << "You are player number " << playerNum << std::endl;
		packet.clear(); 
		packet << static_cast<int32_t>(Headers::PEER_NUM) << playerNum;
		for(int i = 0; i < players.size(); i++)
		{
			players[i].socket->send(packet); 
		}
	}

	bool connected = true;
	//Loop de juego
	while (connected)
	{
		Headers header;
		packet.clear(); 
		for (auto it = players.begin(); it != players.end(); it++)
		{
			Player p = *it;
			status = p.socket->receive(packet); 
			if (status == sf::Socket::Done)
			{
				
				packet >> aux; 
				header = (Headers)aux; 
				//Añadir los cases necesarios para conseguir la comunicacion entre peers durante la partida
				switch (header) {
				case Headers::PEER_NUM: {
					packet >> aux; 
					std::cout << "Player number " << aux << std::endl;
					if (playerNum == aux + 1)
					{
						std::cout << "You are player number " << playerNum << std::endl;
						packet.clear();
						packet << static_cast<int32_t>(Headers::PEER_NUM) << playerNum;
						for (int i = 0; i < players.size(); i++)
						{
							players[i].socket->send(packet);
						}
						
					}
					//if (playerNum == players.size() + 1) {
						//std::cout << "Current turn: Player number 1" << std::endl;
						packet.clear();
						packet << static_cast<int32_t>(Headers::PLAYER_ORD) << 0;
						for (int i = 0; i < players.size(); i++)
						{
							players[i].socket->send(packet);
						}
					//}
				
					break;
				}
				case Headers::PLAYER_ORD:{
				packet >> aux;
				if (playerNum == aux + 1) {
					std::cout << "Your turn" << std::endl;
					std::cout << "Which player to ask for card: " << std::endl;

					std::cout << "Which culture?" << std::endl;

					std::cout << "Which family member?" << std::endl;
				}
				else {
					std::cout << "Current turn: Player number " << aux + 1<< std::endl;
				}
				break;
			    }
				default: break;
				}
			}
			else if (status == sf::Socket::Disconnected)
			{
				std::cout << "A player has disconnected" <<std::endl; 
				p.socket->disconnect(); 
			}
		}

		/*if (status == sf::Socket::Done)
		{
			if (header == Headers::PLAYER_ORD) {
				packet >> aux;
				if (playerNum == aux + 1) {
					std::cout << "Your turn" << std::endl;
				}
				else {
					std::cout << "Current turn: Player number " << aux << std::endl;
				}
			}
		}
		else {
			std::cout << "A player has disconnected" << std::endl;
			socket.disconnect();
		}*/
		

	}

	
		
	/*if (status == sf::Socket::Done) {
		Headers header;
		packet >> aux;
		std::cout << aux << std::endl;
		if (playerNum == aux + 1) {
			std::cout << "Your turn" << std::endl;
		}
		else {
			std::cout << "Current turn: Player number " << aux << std::endl;
		}
	}
	else if (status == sf::Socket::Disconnected)
	{
		std::cout << "A player has disconnected" << std::endl;
		socket.disconnect();
	}*/
	

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