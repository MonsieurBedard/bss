/*
 * MIT License
 *
 * Copyright (c) 2020 Alexandre Bédard
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

#include <stdio.h>
#include <string.h>

#include "./sg_rs232.h"
#include "./sg_tcp.h"


#define CLIENTS_NUMBER 10
#define BUFFER_MAX 100
#define BOAT_MAX 3


typedef struct {
	int id;
	char type; /* s for serial of t for tcp */
	int board[100];
} T_connection;


static char w_receive(T_connection con);
static void w_send(T_connection con, unsigned char c);
static void w_connect();
static void print_help();


T_connection clients[CLIENTS_NUMBER];


static char
w_receive(T_connection con)
{
	unsigned char c;
	unsigned char buffer[BUFFER_MAX + 1];

	if (con.type == 't') {
		bzero(buffer, BUFFER_MAX);
		int lg = recv(con.id, buffer, BUFFER_MAX, 0); // reception
		if (lg < 0) {
			printf("Error w_receive :: con.type = tcp\n");
		} else {
			c = buffer[0];
		}
	} else if (con.type == 's') {
		if(!Rx(con.id, &c, 0)) {
			printf("Error w_receive :: con.type = serial\n");
		}
	}

	return c;
}

static void
w_send(T_connection con, unsigned char c)
{
	unsigned char buffer[BUFFER_MAX + 1];
	buffer[0] = c;
	buffer[1] = 0;
	int lg = strlen(buffer);

	if (con.type == 't') {
		int n = send(con.id, buffer, lg, 0); // Transmission
		if (n < 0) {
			perror("Error w_send :: con.type = tcp\n");
		}
	} else if (con.type == 's') {
		if (!Tx(con.id, &c, 0)) {
			printf("Error w_send :: con.type = serial\n");
		}
	}
}

static void
w_connect()
{
	/* TODO: */
}

static void
print_help() 
{
	printf("This is the help\n");
}

int
main(int argc, char const *argv[])
{
	printf("Hello there!\n");

	return 0;
}
