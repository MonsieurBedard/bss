/******************************************************************

	Module d'utilisation du port RS232 sur Unix / Linux
	
	Stevens Gagnon
	Departement Informatique
	College Shawinigan
	
*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

#define OFF 0
#define ON 1

#define ctrl_handshake OFF

/*-------------------------------------------------------------*/

int ini(char *nom)
{
	int fd;
	char Dev_rs232[100];
	struct termios rs232;

	/*-------------------------------------------------------------*/

	sprintf(Dev_rs232, "/dev/%s", nom);
	printf("\nDevice rs232: %s\n", Dev_rs232);

	fd = open(Dev_rs232, O_RDWR | O_NOCTTY);

	if (fd < 1)
	{
		return -1;
	}
	else
	{
		tcflush(fd, TCIFLUSH);

		rs232.c_cflag = 0;
		rs232.c_oflag = 0;
		rs232.c_iflag = 0;
		rs232.c_lflag = 0;

		rs232.c_cflag = B38400 | CS8 | CREAD | CLOCAL | IGNBRK;
		rs232.c_iflag = IGNBRK | IGNPAR;

		//------------------------------------

		//		rs232.c_cc[VMIN] = 1; // attend 10x10 msec max OU  1 caractere
		//		rs232.c_cc[VTIME] = 10;

		rs232.c_cc[VMIN] = 1; // Attend 1 car indefiniment
		rs232.c_cc[VTIME] = 0;

		//------------------------------------

		cfsetospeed(&rs232, B38400);
		tcsetattr(fd, TCSANOW, &rs232);

		return fd;
	}
}

int Tx(int fd, unsigned char *car, unsigned char handshake_on)
{
	int nb, dump;
	char handshake;

	nb = write(fd, car, (size_t)1);
	if (handshake_on)
		do
		{
			dump = read(fd, &handshake, (size_t)1); // mecanisme de HANDSHAKE
		} while (!dump);

	return nb;
}

int Rx(int fd, unsigned char *car, unsigned char handshake_on)
{
	int nb, dump;
	char handshake = '@';

	nb = read(fd, car, (size_t)1);

	if (nb && handshake_on)
		dump = write(fd, &handshake, (size_t)1); // mecanisme de HANDSHAKE

	return nb;
}
