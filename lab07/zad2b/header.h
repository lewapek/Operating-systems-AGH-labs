#ifndef _HEADER_H_
#define _HEADER_H_

#define RAND_STR_LENGTH 100
#define MAX_CLIENTS_NAME_LENGTH 15
#define ERROR_HANDLER(s) { perror(s); exit(EXIT_FAILURE); }

struct msgbuf
{
	long mtype;
	char mtext[RAND_STR_LENGTH + MAX_CLIENTS_NAME_LENGTH + 8];
};

#endif /*_HEADER_H_*/