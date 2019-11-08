/*
 * serializacion.h
 *
 *  Created on: 30 oct. 2019
 *      Author: utnso
 */

#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_
#define _FILE_OFFSET_BITS 64
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
#include <fuse.h>

int serializar_fs_mkdir(const char*, mode_t );
int serializar_fs_readdir(const char*, void*, fuse_fill_dir_t, off_t, struct fuse_file_info*);
void* serializar_paquete_para_leer_directorio(const char*, void*, fuse_fill_dir_t, off_t, struct fuse_file_info*);
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


//Funciones Auxiliares
char* leer_nombres_de_archivos_y_directorios(void*);
#endif /* SERIALIZACION_H_ */
