/*
 * MIT License
 *
 * Copyright (c) 2020 Alexandre Bédard
 * Copyright (c) 2020 Jérémy Bernard
 * Copyright (c) 2020 Félix Carle-Milette
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * bss.c
 * This is the server side code for a semi-battleship game
 */

#include <stdio.h>
#include <string.h>

/* Sources a Stevens Gagnon */
#include "./sg_rs232.h"
#include "./sg_tcp.h"

#define TX_TIMEOUT 0
#define RX_TIMEOUT 0

#define ADR_LISTEN "INADDR_ANY" /* "INADDR_ANY" ou "192.168.2.2" */
#define PORT_NET 32030 /* Port sur lequel le serveur attend */

#define CLIENTS_NUMBER 10
#define BUFFER_MAX 100
#define BOAT_MAX 3

/*
 * Pour le type
 *   s = serial
 *   t = tcp
 *
 * Pour le board
 *   0 = rien
 *   1 = un bateau
 */
struct T_client {
	int name;
	int id;
	char type;
	int board[100];
	struct T_client *next;
};


static char w_receive(struct T_client *con);
static void w_send(struct T_client *con, unsigned char c);
static void w_send_all(struct T_client *anchor, unsigned char c);
static struct T_client * w_connect(unsigned char c);
static void draw_interface(struct T_client *anchor);
static void init_client(struct T_client *anchor);
static void init_game(struct T_client *anchor);
static void end_game(struct T_client *anchor);
static int game_check(struct T_client *anchor);
static void game(struct T_client *anchor);


int player_count = 0;
int main_board[100];

/*
 * Communication
 */

static char
w_receive(struct T_client *con)
{
	unsigned char c;
	unsigned char buffer[BUFFER_MAX + 1];

	if (con->type == 't') {
		bzero(buffer, BUFFER_MAX);
		int lg = recv(con->id, buffer, BUFFER_MAX, 0); // reception
		if (lg < 0) {
			printf("Error w_receive :: con.type = tcp\n");
		} else {
			c = buffer[0];
		}
	} else if (con->type == 's') {
		if(!Rx(con->id, &c, 0)) {
			printf("Error w_receive :: con.type = serial\n");
		}
	}

	return c;
}

static void
w_send(struct T_client *con, unsigned char c)
{
	unsigned char buffer[BUFFER_MAX + 1];
	buffer[0] = c;
	buffer[1] = 0;
	int lg = strlen(buffer);

	if (con->type == 't') {
		int n = send(con->id, buffer, lg, 0); // Transmission
		if (n < 0) {
			perror("Error w_send :: con.type = tcp\n");
		}
	} else if (con->type == 's') {
		if (!Tx(con->id, &c, 0)) {
			printf("Error w_send :: con.type = serial\n");
		}
	}
}

static void
w_send_all(struct T_client *anchor, unsigned char c)
{
	struct T_client *current_node = anchor;

	while (current_node != NULL) {
		w_send(current_node, c);
		current_node = current_node->next;
	}
}

static struct T_client *
w_connect(unsigned char c)
{
	struct T_client *con = (struct T_client*) malloc(sizeof(struct T_client));

	if (c == 't') {
		printf("En attente de connection...\n");
		int server, client;
		server = setup_tcp_serveur(ADR_LISTEN, PORT_NET, MAX_BUFFER, TX_TIMEOUT, RX_TIMEOUT);
		if (server != -1) {
			client = accept_tcp_client(server);
			close(server);
			con->id = client;
			con->name = player_count++;
			printf("Connection établie\n");
		} else {
			printf("Error w_connect :: con.type = tcp\n");
		}
	} else if (c == 's') { // connection sur root
		char name[10];
		printf("Entrez le nom du port série\n> ");
		scanf("%s", name);
		int fd = ini(name);
		if (fd < 1) {
			printf("Error w_connect :: con.type = serial\n");
		} else {
			con->id = fd;
			con->name = player_count++;
			printf("Connection établie\n");
		}
	}

	return con;
}

static void
draw_interface(struct T_client *anchor)
{
	/* optionnel */
}

static void
init_client(struct T_client *anchor)
{
	struct T_client *current_node;

	current_node = anchor;

	char answer = 0;
	int done, isOk;

	done = 0;
	while (!done) {
		isOk = 0;
		while (!isOk) {
			printf("Choisir un type de connection[t/s]\n> ");
			answer = 0;
			scanf(" %c", &answer);
			if (answer == 't' || answer == 's') {
				current_node->next = w_connect(answer);
				isOk = 1;
			}
		}

		isOk = 0;
		while (!isOk) {
			printf("Ajouter un client?[o/n]\n> ");
			answer = 0;
			scanf(" %c", &answer);
			if (answer == 'o' || answer == 'n') {
				isOk = 1;
			}

			if (answer == 'n') {
				done = 1;
			}
		}

		current_node = current_node->next;
	}
}

static void
init_game(struct T_client *anchor)
{
	/*
	 * Pour chaque client, demander ou placer les cases
	 * TODO: A voir pour les bons codes
	 * 1. demander au client de donner ses cases et les recevoir
	 */

	struct T_client *current_node = anchor;
	int isOk, count;
	char received;

	while (current_node != NULL) {
		printf("Demande des position au joueur %d\n", current_node->name);

		/* 1 */
		w_send(current_node, 100); /* demande de connection */
		while (count <= BOAT_MAX) {
			received = w_receive(current_node);
			if (0 <= received && received <= 99){
				current_node->board[received] = 1;
				count++;
				w_send(current_node, 100);
			} else {
				w_send(current_node, 101);
			}
		}

		w_send(current_node, 100); // confirmation

		current_node = current_node->next;
	}
}

static void
end_game(struct T_client *anchor)
{
	/*
	 * TODO: voir pour utiliser les bons codes
	 */

	struct T_client *current_node = anchor;
	int isOk, count;
	char received;

	while (current_node != NULL) {
		printf("Demande des position au joueur %d\n", current_node->name);

		w_send(current_node, 105);

		count = BOAT_MAX;
		for (int i = 0; i < 100; i++) {
			if (current_node->board[i] == 1 && main_board[i] == 1) {
				count--;
			}
		}
		
		if (count == 0) {
			w_send(current_node, 101); /* lose */
		} else {
			w_send(current_node, 100); /* win */
		}

		current_node = current_node->next;
	}
}

static int
game_check(struct T_client *anchor)
{
	/*
	 * Si fini retourne 1
	 * sinon retourne 0
	 */
	
	struct T_client *current_node = anchor;
	int count;

	while (current_node != NULL) {
		count = BOAT_MAX;
		for (int i = 0; i < 100; i++) {
			if (current_node->board[i] == 1 && main_board[i] == 1) {
				count--;
			}
		}
		
	}

	return 0;
}

static void
game(struct T_client *anchor)
{
	/*
	 * Déroulement de la partie.
	 */

	struct T_client *current_node = anchor;
	int done = 0, isOk = 0;
	char answer = 0;

	while (!done) {
		w_send(current_node, 100);
		isOk = 0;
		while (!isOk) {
			answer = w_receive(current_node);
			if (0 <= answer && answer <= 99) {
				if (main_board[answer] == 0) {
					main_board[answer] = 1;
					isOk = 1;
					w_send(current_node, 100);
					w_send_all(anchor, answer);
				} else {
					w_send(current_node, 101);
				}
			} else {
				w_send(current_node, 101);
			}
		}

		if (game_check(anchor) == 1) {
			done = 1;
		}

		if (current_node->next == NULL) {
			current_node = anchor;
		} else {
			current_node = current_node->next;
		}
	}
}

int
main(int argc, char const *argv[])
{
	struct T_client *anchor = (struct T_client*) malloc(sizeof(struct T_client));
	init_client(anchor);
	init_game(anchor);
	game(anchor);
	end_game(anchor);

	return 0;
}
