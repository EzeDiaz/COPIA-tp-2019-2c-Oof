/*
 * globales.h
 *
 *  Created on: 30 oct. 2019
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_
#include <commons/config.h>
t_config* config;
int socket_sac_server;


//tads
typedef struct{

	char*nombre_de_archivo;
	char estado;
	int puntero_padre;
	int tamanio_del_archivo;
	long fecha_de_creacion;
	long fecha_de_modificacion;
	//ptrGBloque punteros_indirectos_simples[1000];

}nodo_t;
#endif /* GLOBALES_H_ */
