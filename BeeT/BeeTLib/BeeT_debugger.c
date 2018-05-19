#include "BeeT_debugger.h"

#include "BeeT_NW_packet.h"

#define BEET_DB_MAX_NUM_SOCKETS 3 //TODO: Change this

BEET_bool BeeT_Debugger_Init(BeeT_debugger * debugger, int port) // TODO: Do clean up of socketList
{
	debugger->initialized = BEET_TRUE;
	debugger->port = port;

	if (SDLNet_Init() < 0)
	{
		printf("%s\n", SDLNet_GetError());
		return BEET_FALSE;
	}
	if (SDLNet_ResolveHost(&debugger->ip, "localhost", port) == -1)
	{
		printf("%s\n", SDLNet_GetError());
		return BEET_FALSE;
	}

	debugger->socketSet = SDLNet_AllocSocketSet(BEET_DB_MAX_NUM_SOCKETS); 
	if (debugger->socketSet == NULL)
	{
		printf("SDLNet create socket set: %s\n", SDLNet_GetError());
		return BEET_FALSE;
	}
	
	debugger->socketList = InitDequeue();

	for (int i = 0; i < BEET_DB_MAX_NUM_SOCKETS; i++)
	{
		BeeT_socket* sc = (BeeT_socket*)BEET_malloc(sizeof(BeeT_socket));
		sc->state = OPEN;
		sc->socket = NULL;
		dequeue_push_back(debugger->socketList, sc);
	}
	
	return BEET_TRUE;
}

void BeeT_Debugger_Tick(BeeT_debugger * debugger)
{
	SDLNet_CheckSockets(debugger->socketSet, 0);
	node_deq* it = dequeue_head(debugger->socketList);
	while (it != NULL)
	{
		BeeT_socket* sc = (BeeT_socket*)it->data;

		if (sc->state == NOT_INIT)
		{
			it = it->next;
			continue;
		}

		if (sc->state == OPEN)
		{
			TCPsocket tcpsock = SDLNet_TCP_Open(&debugger->ip);
			if (tcpsock == NULL)
			{
				// PRINT ERROR
			}
			sc->socket = tcpsock;
			SDLNet_TCP_AddSocket(debugger->socketSet, sc->socket);
			sc->state++;
		}

		if (sc->state == CONNECTION_ACK)
		{
			if (SDLNet_SocketReady(sc->socket))
			{
				BeeT_packet* packet = (BeeT_packet*)BEET_malloc(sizeof(BeeT_packet));
				BEET_bool readRet = BeeT_packet_Read(packet, &sc->socket);
				if (readRet)
				{				
					if (packet->type == PT_CONNECTION_ACK)
						sc->state++;	
				}
				else 
				{
					sc->state = CLEANUP;// Close connection
				}
				BeeT_packet_Cleanup(packet);
				BEET_free(packet);
			}
		}

		if (sc->state == CLEANUP)
		{
			SDLNet_TCP_DelSocket(debugger->socketSet, sc->socket);
			SDLNet_TCP_Close(sc->socket);
			sc->state = NOT_INIT;
		}

		it = it->next;
	}
}