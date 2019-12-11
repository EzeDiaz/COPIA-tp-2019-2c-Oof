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

	CREAR_ARCHIVO=100,
	ESCRIBIR_ARCHIVO=101,
	ABRIR_ARCHIVO=102,
	LEER_ARCHIVO=103,
	BORRAR_ARCHIVO=104,
	CREAR_DIRECTORIO=105,
	LISTAR_DIRECTORIO_Y_ARCHIVOS=106,
	ELIMINAR_DIRECTORIO=107,
	ABRIR_DIRECTORIO=108,
	LISTAR_METADATA_DIRECTORIO_Y_ARCHIVOS=109,
	GET_ATTRIBUTES=110,
	MKNOD=111,
	RENAME=112,
	ACCESS=113,
	TRUNCATE=114,
	STATFS = 115

};


//PROTOTIPOS

//serializacion
int serializar_fs_mkdir(const char*, mode_t );
int serializar_fs_readdir(const char*, void*, fuse_fill_dir_t, off_t, struct fuse_file_info*,int);
int serializar_fs_rmdir(char* );
int serializar_fs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info*);
int serializar_fs_open(const char *pathname, struct fuse_file_info *fi);
int serializar_fs_write(const char * path1, const char *path2, size_t size, off_t offset,struct fuse_file_info* );
int serializar_fs_opendir(const char *, struct fuse_file_info *);
int serializar_fs_create(const char *, mode_t  , struct fuse_file_info * );
int serializar_fs_getattr(const char *, struct stat *,struct fuse_file_info *);
int serializar_fs_mknod( char* ,mode_t,dev_t );
int serializar_fs_rename( char *, char *,  int);
int serializar_fs_access(const char*,int);
int serializar_fs_truncate(const char *, off_t , struct fuse_file_info *);
int serializar_fs_statfs(const char*, struct statvfs*);

//envios a SAC_SERVER
void* serializar_paquete_para_leer_directorio(const char*, void*,off_t);
void* serializar_paquete_para_crear_directorio(const char* ,mode_t );
void* serializar_paquete_para_eliminar_directorio(char* );
void* serializar_paquete_para_obtener_atributos(const char*);
void* serializar_paquete_para_leer_archivo(const char *path, char *buf, size_t size, off_t offset);
void* serializar_paquete_para_escribir_archivo(const char * , const char *, size_t , off_t );
void* serializar_paquete_para_abrir_archivo(const char *);
void* enviar_paquete(void*);
void* recibir_resultado(int* );
void* serializar_paquete_para_abrir_directorio(const char*);
void* serializar_paquete_para_crear_archivo(const char *,mode_t);
void* serializar_paquete_fs_rename(const char *, const char *, unsigned int);
void* serializar_paquete_fs_mknod(const char* ,mode_t,dev_t );
void*serializar_paquete_fs_accses(const char* ,int );
void* serializar_paquete_fs_truncate(const char* ,off_t);
void* serializar_paquete_fs_statfs(const char*, struct statvfs*);


//Funciones Auxiliares
char* leer_nombres_de_archivos_y_directorios(void*);

#endif /* SERIALIZACION_H_ */
