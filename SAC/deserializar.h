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

	int fd;
	void *buf;
	size_t count;

}archivo_descifrado_escritura;

typedef struct{
	char* path;
	char* string_nombre_de_archivos;
}directorio_a_listar_t;





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
	LISTAR_METADATA_DIRECTORIO_Y_ARCHIVOS=9
};


//Prototipos
int determinar_protocolo(void*);
void identificar_paquete_y_ejecutar_comando(int, void*);
void* listar_metadata_directorio_y_archivos(void*);
void* decifrar_archivo_a_crear(void*);
archivo_descifrado_escritura* decifrar_archivo_a_escribir(void*);
void* decifrar_archivo_a_leer(void*);
void* decifrar_archivo_a_borrar(void*);
creacion* decifrar_directorio_a_crear(void*);
directorio_a_listar_t* decifrar_directorio_a_listar(void*);
char* decifrar_directorio_a_borrar(void*);

#endif /* DESERIALIZAR_H_ */
