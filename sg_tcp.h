/*****************************************************************

	--> sg_tcp.h

	Module d'utilisation d'un socket TCP sur Unix

	Stevens Gagnon
	Departement Informatique
	College Shawinigan

******************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define MAX_BUFFER 100

//---------------------------------------------------------------- prt_IP ( Unit_ID )

void prt_IP(int Socket_ID)
{
	struct sockaddr_in addr;
	char IP[30];
	int dump;

	socklen_t addr_size = sizeof(struct sockaddr_in);

	dump = getpeername(Socket_ID, (struct sockaddr *)&addr, &addr_size);
	strcpy(IP, inet_ntoa(addr.sin_addr));

	printf("%s", IP );
}

//-------------------------------------------------------------------------------------- setup_client

int setup_tcp_client(char adresse[20], int port, int max_buffer, int Tx_TimeOut, int Rx_TimeOut )
{
	int value = 1;
	int serveur;
	struct timeval tmr;

	struct sockaddr_in addr_serveur;
	struct hostent *nom_serveur;
	struct protoent *tcp_proto;


	if ( (tcp_proto = getprotobyname("tcp")) == 0 )
	{
		perror("\n\nERREUR getprotobyname\n\n");
		return -1;
	}
	else
	{
		serveur = socket(PF_INET, SOCK_STREAM, tcp_proto->p_proto);

		if (serveur < 0)
		{
			perror("\n\nERREUR socket\n\n");
			return -1;
		}
		else
		{
			nom_serveur = gethostbyname(adresse);

			if (nom_serveur == NULL)
			{
				perror("\n\nERREUR, adresse invalide\n\n");
				return -1;
			}
			else
			{
				if( max_buffer > 0 ){ value = max_buffer; }
				else{ value = 1000; }
				setsockopt(serveur, SOL_SOCKET, SO_SNDBUF, &value, sizeof(value));

				value = MAX_BUFFER;
				setsockopt(serveur, SOL_SOCKET, SO_RCVBUF, &value, sizeof(value));

				value = 1;
				setsockopt(serveur, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

				value = 1;
				setsockopt(serveur, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));

				if( Rx_TimeOut > 0 )
				{
					tmr.tv_sec = Rx_TimeOut;
					tmr.tv_usec = 0;
					setsockopt(serveur, SOL_SOCKET, SO_RCVTIMEO, &tmr, sizeof(tmr));
				}

				if( Tx_TimeOut > 0 )
				{
					tmr.tv_sec = Tx_TimeOut;
					tmr.tv_usec = 0;
					setsockopt(serveur, SOL_SOCKET, SO_SNDTIMEO, &tmr, sizeof(tmr));
				}

				value = 1;
				setsockopt(serveur, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

				memset(&addr_serveur, 0, sizeof(struct sockaddr_in));

				addr_serveur.sin_family = AF_INET;
				bcopy((char *)nom_serveur->h_addr, (char *)&addr_serveur.sin_addr.s_addr, nom_serveur->h_length);
				addr_serveur.sin_port = htons(port);

				if (connect(serveur, (struct sockaddr *) &addr_serveur,sizeof(addr_serveur)) < 0)
				{
					perror("\n\nERREUR connect\n\n");
					return -1;
				}
				else
				{
					return serveur;
				}
			}
		}
	}
}

//---------------------------------------------------------------- setup_serveur

int setup_tcp_serveur(char ip_addr[20], int port, int max_buffer, int Tx_TimeOut, int Rx_TimeOut )
{
	struct sockaddr_in adresse;
	struct protoent *tcp_proto;
	struct hostent *nom_serveur; /// <<-- new
	struct timeval tmr;
	int serveur;
	int value=1;


	if ( (tcp_proto = getprotobyname("tcp")) == 0 )
	{
		perror("\n\nERREUR getprotobyname\n\n");
		return -1;
	}
	else
	{
		serveur = socket(PF_INET, SOCK_STREAM, tcp_proto->p_proto);
		if (serveur == -1)
		{
			perror("\n\nERREUR socket\n\n");
			return -1;
		}
		else
		{
			if( max_buffer > 0 ){ value = max_buffer; }
			else{ value = 1000; }
			setsockopt(serveur, SOL_SOCKET, SO_SNDBUF, &value, sizeof(value));

			value = MAX_BUFFER;
			setsockopt(serveur, SOL_SOCKET, SO_RCVBUF, &value, sizeof(value));

			value = 1;
			setsockopt(serveur, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

			value = 1;
			setsockopt(serveur, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));

			if( Rx_TimeOut > 0 )
			{
				tmr.tv_sec = Rx_TimeOut;
				tmr.tv_usec = 0;
				setsockopt(serveur, SOL_SOCKET, SO_RCVTIMEO, &tmr, sizeof(tmr));
			}

			if( Tx_TimeOut > 0 )
			{
				tmr.tv_sec = Tx_TimeOut;
				tmr.tv_usec = 0;
				setsockopt(serveur, SOL_SOCKET, SO_SNDTIMEO, &tmr, sizeof(tmr));
			}

			value = 1;
			setsockopt(serveur, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

			memset(&adresse, 0, sizeof(struct sockaddr_in));

			if( !strcmp( ip_addr, "INADDR_ANY") )
					adresse.sin_addr.s_addr = INADDR_ANY;
			else
			{
				nom_serveur = gethostbyname(ip_addr);
				bcopy((char *)nom_serveur->h_addr, (char *)&adresse.sin_addr.s_addr, nom_serveur->h_length);
			}

			adresse.sin_family = AF_INET;
			adresse.sin_port = htons(port);

			if ( bind(serveur, (struct sockaddr *) &adresse, sizeof(struct sockaddr_in)) == -1 )
			{
				perror("\n\nERREUR bind\n\n");
				return -1;
			}
			else
			{
				if ( listen(serveur,1) == -1 )
				{
					perror("\n\nERREUR listen\n\n");
					return -1;
				}
				else
				{
					return serveur;
				}
			}
		}
	}
}

//---------------------------------------------------------------- client_accept

int accept_tcp_client(int serveur)
{
	int connexion;

	if ((connexion = accept(serveur, NULL, NULL)) < 0)
	{
		perror("\n\nERREUR accept\n\n");
		return -1;
	}
	else
	{
		return connexion;
	}
}

//----------------------------------------------------------------

