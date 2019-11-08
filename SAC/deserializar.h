/*
 * deserializar.h
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */

#ifndef DESERIALIZAR_H_
#define DESERIALIZAR_H_
#include <commons/log.h>

//TADS
typedef struct{
	char* path;
	mode_t mode;
}directorio_a_crear_t;

typedef struct{
	char* path;
	char* string_nombre_de_archivos;
}directorio_a_listar_t;

t_log* logger_de_deserializacion;



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



//Prototipos
int determinar_protocolo(void*);
void identificar_paquete_y_ejecutar_comando(int, void*);
void* listar_metadata_directorio_y_archivos(void*);
void* decifrar_archivo_a_crear(void*);
void* decifrar_archivo_a_escribir(void*);
void* decifrar_archivo_a_leer(void*);
void* decifrar_archivo_a_borrar(void*);
directorio_a_crear_t* decifrar_directorio_a_crear(void*);
directorio_a_listar_t* decifrar_directorio_a_listar(void*);
void* decifrar_directorio_a_borrar(void*);

#endif /* DESERIALIZAR_H_ */
