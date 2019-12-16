/*
 * deserializar.h
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */

#ifndef DESERIALIZAR_H_
#define DESERIALIZAR_H_
#include <commons/log.h>
#include "globales.h"

//TADS
typedef struct{
	char* path;
	mode_t mode;
}creacion;

typedef struct{

	const char * string1;
	const char *string2;
	size_t size;
	off_t offset;
}archivo_descifrado_escritura;

typedef struct{

	const char *path;
	char *buf;
	size_t size;
	off_t desplazamiento;

}params_lectura;

typedef struct{
	char* path;
	char* string_nombre_de_archivos;
}directorio_a_listar_t;

typedef struct{
	char* old_path;
	char* new_path;
	int flag;
}rename_params;

typedef struct{

	char* name;
	mode_t mode;
	dev_t dev;

}mknod_params;


typedef struct{

	char*name;
	mode_t mode;
	int flags;

}open_file_params;

typedef struct{
	int flags;
	char* path;

}access_params;

typedef struct{

	off_t offset;
	char* path;

}truncate_params;

typedef struct{

	char* path;
	struct statvfs* stats;

}statfs_params;

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



//Prototipos
int determinar_protocolo(void*);
void identificar_paquete_y_ejecutar_comando(int, void*);
void* listar_metadata_directorio_y_archivos(void*);
void* decifrar_archivo_a_crear(void*);
archivo_descifrado_escritura* decifrar_archivo_a_escribir(void*);
params_lectura* decifrar_archivo_a_leer(void*);
char* decifrar_archivo_a_borrar(void*);
creacion* decifrar_creacion(void*);
directorio_a_listar_t* decifrar_directorio_a_listar(void*);
char* decifrar_directorio_a_borrar(void*);
mknod_params* decifrar_mknod(void*);
rename_params*decifrar_rename(void*);
char* decifrar_get_atributes(void*);
open_file_params*decifrar_openfile(void*);
char* decifrar_opendir(void*);
access_params* decifrar_access(void* );
void* serializar(int);
truncate_params* decifrar_truncate(void* );
statfs_params* decifrar_statfs(void*);


#endif /* DESERIALIZAR_H_ */
