/*
 * deserializar.h
 *
 *  Created on: 2 nov. 2019
 *      Author: utnso
 */

#ifndef DESERIALIZAR_H_
#define DESERIALIZAR_H_
#include <commons/log.h>
t_log* logger_de_deserializacion;

enum {
	HILOLAY_INIT = 0,
	SUSE_CREATE = 1,
	SUSE_SCHEDULER_NEXT = 2,
	SUSE_WAIT = 3,
	SUSE_SIGNAL = 4,
	SUSE_JOIN = 5,
	SUSE_CLOSE = 6,
};

#endif /* DESERIALIZAR_H_ */
