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


//enums
enum codigo_de_operacion{

	CREAR_ARCHIVO=0,
	ESCRIBIR_ARCHIVO=1,
	ABRIR_ARCHIVO=2,
	LEER_ARCHIVO=3,
	BORRAR_ARCHIVO=4,
	CREAR_DIRECTORIO=5,
	LISTAR_DIRECTORIO_Y_ARCHIVOS=6,
	ELIMINAR_DIRECTORIO=7,
	ABRIR_DIRECTORIO=8,
	LISTAR_METADATA_DIRECTORIO_Y_ARCHIVOS=9,
	GET_ATTRIBUTES=10,
	MKNOD=11,
	RENAME=12

};


//PROTOTIPOS

//serializacion
int serializar_fs_mkdir(const char*, mode_t );
int serializar_fs_readdir(const char*, void*, fuse_fill_dir_t, off_t, struct fuse_file_info*);
int serializar_fs_rmdir(char* );
int serializar_fs_read(int , void *, size_t );
int serializar_fs_open(const char *,int, mode_t );
int serializar_fs_write(int , const void *, size_t );
int serializar_fs_opendir(const char* path);
int serializar_fs_create(const char *, mode_t  , struct fuse_file_info * );
int serializar_fs_getattr(const char *, struct stat *);
int serializar_fs_mknod( char* ,mode_t,dev_t );
int serializar_fs_rename( char *, char *,  int);


//envios a SAC_SERVER
void* serializar_paquete_para_leer_directorio(const char*, void*);
void* serializar_paquete_para_crear_directorio(const char* ,mode_t );
void* serializar_paquete_para_eliminar_directorio(char* );
void* serializar_paquete_para_obtener_atributos(const char*);
void* serializar_paquete_para_leer_archivo(int , void *, size_t );
void* serializar_paquete_para_escribir_archivo(int , const void *, size_t );
void* serializar_paquete_para_abrir_archivo(const char *,int, mode_t );
void* enviar_paquete(void*);
void* recibir_resultado(int* );
void* serializar_paquete_para_abrir_directorio(const char*);
void* serializar_paquete_para_crear_archivo(const char *,mode_t);
void* serializar_paquete_fs_rename(const char *, const char *, unsigned int);
void* serializar_paquete_fs_mknod(const char* ,mode_t,dev_t );



//Funciones Auxiliares
char* leer_nombres_de_archivos_y_directorios(void*);

#endif /* SERIALIZACION_H_ */
