/*
 * globales.h
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_

t_log* log_servidor;
int PUERTO_ESCUCHA;
char* IP;

//Semaforos
sem_t* mutex_log_servidor;

#endif /* GLOBALES_H_ */
