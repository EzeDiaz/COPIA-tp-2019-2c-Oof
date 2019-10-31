/*
 * serializacion.h
 *
 *  Created on: 30 oct. 2019
 *      Author: utnso
 */

#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <string.h>
#include <arpa/inet.h>
#include "globales.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <commons/config.h>



static int serializar_fs_mkdir(const char*, mode_t );
void* serializar_paquete_para_crear_directorio(char* ,mode_t );
void* enviar_paquete(void*);
void* recibir_resultado(int* );

//enums
enum codigo_de_operacion{

	CREAR_ARCHIVO=0,
	ESCRIBIR_ARCHIVO=1,
	LEER_ARCHIVO=2,
	BORRAR_ARCHIVO=3,
	CREAR_DIRECTORIO=4,
	LISTAR_DIRECTORIO_Y_ARCHIVOS=5,
	ELIMINAR_DIRECTORIO=6,
	LISTAR_METADATA_DIRECTORIO_Y_ARCHIVOS=7
};

#endif /* SERIALIZACION_H_ */
