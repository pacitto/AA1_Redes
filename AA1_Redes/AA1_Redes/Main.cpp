#include <SFML\Network.hpp>
#include <iostream>
#include <list>

/*
* ENUMS
*/

//Conexión
enum Commands {LOG_IN, CREATE_ROOM, JOIN_ROOM, ASK_ROOM, CHOOSE_ROOM, FIRST_PEER, CONNECT_TO }; //comandos para comunicación
enum Headers {PEER_NUM, READY, PLAYER_ORD, PLAYER_AN, PLAYER_RES};

/*
* TO DO: ENUM COLOR, ENUM TYPE, ENUM STATE
*/
enum COLOR {RED, BLUE, YELLOW, GREEN, RAINBOW};
enum TYPE {ORGAN, VIRUS, VACCINE, TREATMENT};
enum STATE {HEALTHY, INFECTED, INMUNE, PROTECTED};
enum TREATMENT_TYPE {INFECTION, ORGAN_THIEF, TRANSPLANT, LATEX_GLOVE, MEDICAL_ERROR};

struct ConnectionInfo {
	int port;
	std::string ip;
	sf::TcpSocket* socket; 
};

//Struct para carta con constructor
struct Card {

	COLOR color; //COLOR
	TYPE type; //TIPO
	STATE state; //ESTAT
	TREATMENT_TYPE treatment_type; //TRATAMIENTO
	std::string name;

	Card(COLOR _color, TYPE _type, TREATMENT_TYPE _tt)
	{
		color = _color;
		type = _type;
		treatment_type = _tt;

		setType();
	}

	void setType() {
		switch (type) {
			case TYPE::ORGAN:
				name = "Organ ";
				setColor();
				break;
			case TYPE::TREATMENT:
				name = "Treatment ";
				switch (treatment_type)
				{
				case TREATMENT_TYPE::INFECTION:
					name += "infection";
					break;
				case TREATMENT_TYPE::LATEX_GLOVE:
					name += "latex glove";
					break;
				case TREATMENT_TYPE::MEDICAL_ERROR:
					name += "medical error";
					break;
				case TREATMENT_TYPE::ORGAN_THIEF:
					name += "organ thief";
					break;
				case TREATMENT_TYPE::TRANSPLANT:
					name += "transplant";
					break;
				default:
					break;
				}
				break;
			case TYPE::VACCINE:
				name = "Vaccine ";
				setColor();
				break;
			case TYPE::VIRUS:
				name = "Virus ";
				setColor();
				break;
			default: 
				break;
		}
	}
	void setColor() {
		switch (color) {
			case COLOR::BLUE:
				name += "blue ";
				break;
			case COLOR::GREEN:
				name += "green ";
				break;
			case COLOR::RED:
				name += "red ";
				break;
			case COLOR::YELLOW:
				name += "yellow ";
				break;
			case COLOR::RAINBOW:
				name += "rainbow ";
				break;
			default:
				break;
		}
	}
};
//struct de player con un vector de cartas para guardar su mano
struct Player {
	sf::TcpSocket* socket;
	int id;
	std::vector<Card> hand;
	std::vector<Card> table;
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
//TO DO: INICIALIZAR DECK CON TODAS LAS CARTAS
void InitializeDeck(std::vector<Card>* deck)
{
	//Rainbow
	deck->push_back(Card{ COLOR::RAINBOW, TYPE::ORGAN, (TREATMENT_TYPE)0 });
	deck->push_back(Card{ COLOR::RAINBOW, TYPE::VIRUS, (TREATMENT_TYPE)0 });
	for(int i = 0; i < 4; i++)
		deck->push_back(Card{ COLOR::RAINBOW, TYPE::VACCINE, (TREATMENT_TYPE)0 });
	//Virus y Vacunas
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			deck->push_back(Card{ (COLOR)j, TYPE::VACCINE, (TREATMENT_TYPE)0 });
			deck->push_back(Card{ (COLOR)j, TYPE::VIRUS, (TREATMENT_TYPE)0 });
		}
	}
	//Organos
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			deck->push_back(Card{ (COLOR)j, TYPE::ORGAN, (TREATMENT_TYPE)0 });
		}
	}
	//Tratamientos
	for (int i = 0; i < 2; i++)
		deck->push_back(Card{ COLOR::RAINBOW, TYPE::TREATMENT, TREATMENT_TYPE::INFECTION });
	for (int i = 0; i < 3; i++) {
		deck->push_back(Card{ COLOR::RAINBOW, TYPE::TREATMENT, TREATMENT_TYPE::ORGAN_THIEF });
		deck->push_back(Card{ COLOR::RAINBOW, TYPE::TREATMENT, TREATMENT_TYPE::TRANSPLANT });
	}
	for (int i = 0; i < 1; i++) {
		deck->push_back(Card{ COLOR::RAINBOW, TYPE::TREATMENT, TREATMENT_TYPE::LATEX_GLOVE });
		deck->push_back(Card{ COLOR::RAINBOW, TYPE::TREATMENT, TREATMENT_TYPE::MEDICAL_ERROR });
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

/*
* TO DO: UTILIZAR SEED PARA EL RANDOM Y CAMBIAR LÓGICA DE REPARTO
*/
void DealCards(std::vector<Card> deck, std::vector<Card>* hand, int numPlayers, int playerId)
{
	int i = 1; 
	int random;

	while (hand->size() < 3)
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

//returns score based on number of cards held
int GetScore(std::vector<Card> deck) {

	int score = 0;
	int numArab = 0;
	int numBantu = 0;
	int numChina = 0;
	int numEsquimal = 0;
	int numIndia = 0;
	int numMexican = 0;
	int numTirolese = 0;

	//for (auto it = deck.begin(); it != deck.end(); it++) {
	//	Card c = *it;
	//	if (c.culture == Culture::ARAB) {
	//		numArab++;
	//	}
	//	if (c.culture == Culture::BANTU) {
	//		numBantu++;
	//	}
	//	if (c.culture == Culture::CHINA) {
	//		numChina++;
	//	}
	//	if (c.culture == Culture::ESQUIMAL) {
	//		numEsquimal++;
	//	}
	//	if (c.culture == Culture::INDIA) {
	//		numIndia++;
	//	}
	//	if (c.culture == Culture::MEXICAN) {
	//		numMexican++;
	//	}
	//	if (c.culture == Culture::TIROLESE) {
	//		numTirolese++;
	//	}
	//}

	if (numArab == 6) {
		score++;
	}
	if (numBantu == 6) {
		score++;
	}
	if (numChina == 6) {
		score++;
	}
	if (numEsquimal == 6) {
		score++;
	}
	if (numIndia == 6) {
		score++;
	}
	if (numMexican == 6) {
		score++;
	}
	if (numTirolese == 6) {
		score++;
	}

	return score;
}


//Adds card to deck
std::vector<Card> AddCard(std::vector<Card> deck, int cult, int fam) {
	//deck.push_back(Card{});
	return deck;
}

//Removes card from deck
//std::vector<Card> RemoveCard(std::vector<Card> deck, int cult, int fam) {
//
//	bool foundCard = false;
//	for (auto it = deck.begin(); it != deck.end(); it++) {
//		
//		Card c = *it;
//		if (c.culture == (Culture)cult and c.family == (Family)fam) {
//			// erase this element, and get an iterator to the new next one
//			it = deck.erase(it);
//
//		}
//
//	}
//	return deck;
//}

/// <summary>
/// Imprimir por pantalla las cartas que tiene el jugador
/// </summary>
/// <param name="hand"> Mano del jugador </param>
void ShowHand(std::vector<Card> hand)
{
	std::string color, type, treatment; 
	for (int i = 0; i < hand.size(); i++)
	{
		
		std::cout << i + 1 << ". " << hand[i].name << std::endl; 
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
						std::cout << "Enter the number of players for the game (2 - 4)" << std::endl;
						std::cin >> aux;
						while (aux < 2 || aux > 4)
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
					std::cout << "Enter the number of players for the game (2 - 4)" << std::endl;
					std::cin >> aux; 
					while (aux < 2 || aux > 4)
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
	if(!players[playerNum - 1].hand.empty())
		players[playerNum - 1].hand.clear();
	for (auto var : playerInfo.hand) {
		players[playerNum - 1].hand.push_back(var);
	}
	
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
				if (numready == players.size()) {
					ready = true;
					std::cout << "READY" << std::endl;
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
					if (playerNum == aux + 1)
					{
						std::cout << "You are player number " << playerNum << std::endl;
						if (playerNum > players.size()) {
							//std::cout << "Current turn: Player number 1" << std::endl;
							packet.clear();
							packet << static_cast<int32_t>(Headers::PLAYER_ORD) << 0;
							for (int i = 0; i < players.size(); i++)
							{
								players[i].socket->send(packet);
							}
							std::cout << "Current turn: Player number 1" << std::endl;
						}
						else
						{
							packet.clear();
							packet << static_cast<int32_t>(Headers::PEER_NUM) << playerNum;
							for (int i = 0; i < players.size(); i++)
							{
								players[i].socket->send(packet);
							}
						}
					}

					break;
				}
				case Headers::PLAYER_ORD: {
					packet >> aux;
					if (playerNum == aux + 1) {
						int score = GetScore(p.hand);
						int whichPlayer = -1;
						int whichCulture = -1;
						int whichMember = -1;
						if (score == 7) {
							//end game, player wins
						}

						std::cout << "Your turn" << std::endl;
						std::cout << "Your score is: " << score << std::endl;

						while (whichPlayer <= 0 || whichPlayer > numPlayers || whichPlayer==playerNum) {
							std::cout << "Which player to ask for card? (introduce player number)" << std::endl;
							std::cin >> whichPlayer;
						}

						while (whichCulture <= 0 || whichCulture > 7) {
							std::cout << "Which culture? (1.ARAB  2.BANTU  3.CHINA  4.ESQUIMAL  5.INDIA  6.MEXICAN  7.TIROLESE)" << std::endl;
							std::cin >> whichCulture;
						}

						while (whichMember <= 0 || whichMember > 6) {
							std::cout << "Which family member? (1.GRANDFATHER  2.GRANDMOTHER  3.FATHER  4.MOTHER  5.SON  6.DAUGHTER)" << std::endl;
							std::cin >> whichMember;
						}
						packet.clear();
						packet << static_cast<int32_t>(Headers::PLAYER_AN) << whichPlayer << whichCulture << whichMember << playerNum;
						players[whichPlayer-2].socket->send(packet);

					}
					else {
						std::cout << "Current turn: Player number " << aux + 1 << std::endl;
					}
					break;
				}
				case Headers::PLAYER_AN: {

					packet >> aux;
					//int whichPlayer = -1;
					int whichCulture = -1;
					int whichMember = -1;
					int answer = -1;
					int playerResponding = -1;

					packet >> whichCulture;
					packet >> whichMember;
					packet >> playerResponding;

					while (answer < 0 || answer>1) {
						std::cout << "Player " << playerResponding << " asks:" << std::endl;
						std::cout << "Do you have this card? First number: (1.ARAB  2.BANTU  3.CHINA  4.ESQUIMAL  5.INDIA  6.MEXICAN  7.TIROLESE)" << std::endl;
						std::cout << "Second number:(1.GRANDFATHER  2.GRANDMOTHER  3.FATHER  4.MOTHER  5.SON  6.DAUGHTER) Write 0 for no or 1 for yes" << std::endl;
						std::cout << "First number: " << whichCulture << " second number: " << whichMember << std::endl;
						std::cin >> answer;
					}

					if (answer == 1) {
						std::cout << "Your hand is now:" << std::endl;
						//players[playerNum - 1].hand = RemoveCard(players[playerNum-1].hand, whichCulture-1, whichMember-1);
						ShowHand(players[playerNum-1].hand);
					}

					packet.clear();
					packet << static_cast<int32_t>(Headers::PLAYER_RES) << answer << whichCulture << whichMember;
					players[playerResponding-1].socket->send(packet);

					break;
				}
				case Headers::PLAYER_RES: {

					int whichCulture = -1;
					int whichMember = -1;
					int answer = -1;

					packet >> answer;
					packet >> whichCulture;
					packet >> whichMember;

					std::cout << "Your hand is now:" << std::endl;
					players[playerNum - 1].hand = AddCard(players[playerNum - 1].hand, whichCulture - 1, whichMember - 1);
					ShowHand(players[playerNum - 1].hand);

					packet.clear();
					packet << static_cast<int32_t>(Headers::PLAYER_ORD) << playerNum;
					for (int i = 0; i < players.size(); i++)
					{
						players[i].socket->send(packet);
					}
					

					break;
				}
				default: break;
				}
			}
			else if (status == sf::Socket::Disconnected)
			{
				numPlayers--;
				std::cout << "A player has disconnected" <<std::endl; 
				p.socket->disconnect(); 
				if (numPlayers <= 2) {
					std::cout << "Less than 3 players, match is over" << std::endl;
					exit(3);
				}
			}
		}
	}

	socket.disconnect();
}

void GameLoop() 
{

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