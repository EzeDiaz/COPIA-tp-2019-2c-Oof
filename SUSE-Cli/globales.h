/*
 * globales.h
 *
 *  Created on: 2 nov. 2019
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_

int socket_suse_server;

enum {
	HILOLAY_INIT = 0,
	SUSE_CREATE = 1,
	SUSE_SCHEDULER_NEXT = 2,
	SUSE_WAIT = 3,
	SUSE_SIGNAL = 4,
	SUSE_JOIN = 5,
	SUSE_CLOSE = 6,
};

#endif /* GLOBALES_H_ */
