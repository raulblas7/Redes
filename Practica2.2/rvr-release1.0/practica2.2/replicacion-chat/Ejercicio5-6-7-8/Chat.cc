#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data
    char* tmp = _data;

    memcpy(tmp, &type, sizeof(uint8_t));
    tmp += sizeof(uint8_t);

    memcpy(tmp, nick.c_str(), 8 * sizeof(char));
    tmp += 8 * sizeof(char);

    memcpy(tmp, message.c_str(), 80 * sizeof(char));
    tmp += 80 * sizeof(char);

}

int ChatMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data

    char* buffer = _data;

    memcpy(type, buffer, sizeof(uint8_t));
    buffer += sizeof(uint8_t);

    nick = buffer;
    buffer += 8 * sizeof(char);

    message = buffer;
    buffer += 80 * sizeof(char);

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{
    while (true)
    {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */

        //Recibir Mensajes en y en función del tipo de mensaje
        // - LOGIN: Añadir al vector clients
        // - LOGOUT: Eliminar del vector clients
        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)

		ChatMessage messg;
		Socket* client = &socket;
		socket.recv(messg, client);

		std::unique_ptr<Socket> clientSocket(client);

		switch (messg.type)
		{
			case 0:
				//Si el tipo de mensaje es cero, es un mensaje de login, asi que se añade a la lista de clientes que tiene el servidor
				clients.push_back(std::move(clientSocket));
				std::cout << "Usuario " << messg.nick << " se ha unido" << std::endl;
				break;
			case 1:
				//Si el tipo es uno, entonces es que un cliente ha enviado un mensaje al chat y el servidor avisa a todos los demas clientes del mensaje 
				for (auto it = clients.begin(); it != clients.end(); it++)
				{
					if (!(**it == *clientSocket))
					{
						socket.send(messg, **it);
					}
				}
				break;
			case 2:
			
				//Si el tipo es dos, entonces es que un cliente se ha desconectado del server, lo busco y lo elimino del vector de clientes
				bool encontrado = false;
				auto it = clients.begin();
				while (!encontrado && it != clients.end())
				{
					if (**it == *clientSocket)
					{
						it = clients.erase(it);
						encontrado = true;
					}
					else
					{
						it++;
					}
				}
				std::cout << "Usuario " << messg.nick << " se ha desconectado." << std::endl;
				break;
			
			default:
				break;
			
		}
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatClient::login()
{
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGIN;

    socket.send(em, socket);
}

void ChatClient::logout()
{
    // Completar
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGOUT;

    socket.send(em, socket);
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        // Enviar al servidor usando socket
        std::string msg;
        std::getline(std::cin, msg);

        ChatMessage em(nick, msg);
        em.type = ChatMessage::MESSAGE;

        socket.send(em, socket);

        std::cout << "Mensaje -" << em.message << "- enviado" << std::endl;
    }
}

void ChatClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
        ChatMessage mssge;
        socket.recv(mssge);

        std::cout << mssge.nick << ": " << mssge.message << std::endl;

    }
}

