/*
 * deserializar.c
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */
#include "deserializar.h"

int determinar_protocolo(void* buffer){
	int codigo_de_operacion;
	int offset = 0;
	memcpy(&(codigo_de_operacion), (buffer+offset), sizeof(int));
	log_info(logger_de_deserializacion, "Nos llego un codigo invalido\n");
	return codigo_de_operacion;
}

void identificar_paquete_y_ejecutar_comando(int cliente_socket, void* buffer, int codigo_de_operacion){
	log_info(logger_de_deserializacion, "Estamos por deserializar el codigo %d\n",codigo_de_operacion);
	switch(codigo_de_operacion){
	default:
		send(cliente_socket, "Codigo Invalido", 16, 0);
		log_info(logger_de_deserializacion, "Nos llego un codigo invalido\n");
	}
}
