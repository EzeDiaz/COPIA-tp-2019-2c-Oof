/*
 * deserializar.c
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */
#include "deserializar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int determinar_protocolo(void* buffer){
	int codigo_de_operacion;
	int offset = 0;
	memcpy(&(codigo_de_operacion), (buffer+offset), sizeof(int));
	log_info(logger_de_deserializacion, "Obtuvimos el codigo de operacion \n");
	return codigo_de_operacion;
}

void identificar_paquete_y_ejecutar_comando(int cliente_socket, void* buffer){
	log_info(logger_de_deserializacion, "Estamos por deserializar el codigo de operacion\n");
	int codigo_de_operacion=determinar_protocolo(buffer);
	void* resultado;
	void* paquete_decifrado;

	switch(codigo_de_operacion){

	case CREAR_ARCHIVO:
		log_info(logger_de_deserializacion, "Es el codigo de 'creacion de archivos', comenzando la deserializacion de parametros\n");
		paquete_decifrado=decifrar_archivo_a_crear(buffer);
		resultado=crear_archivo(paquete_decifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
	break;

	case ESCRIBIR_ARCHIVO:
		log_info(logger_de_deserializacion, "Es el codigo de 'escribir de archivos', comenzando la deserializacion de parametros\n");
		paquete_decifrado=decifrar_archivo_a_escribir(buffer);
		resultado=escribir_archivo(paquete_decifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
	break;

	case LEER_ARCHIVO:
		log_info(logger_de_deserializacion, "Es el codigo de 'lectura de archivos', comenzando la deserializacion de parametros\n");
		paquete_decifrado=decifrar_archivo_a_leer(buffer);
		resultado=leer_archivo(paquete_decifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
	break;

	case BORRAR_ARCHIVO:
		log_info(logger_de_deserializacion, "Es el codigo de 'eliminacion de archivos', comenzando la deserializacion de parametros\n");
		paquete_decifrado=decifrar_archivo_a_borrar(buffer);
		resultado=borrar_archivo(paquete_decifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
	break;

	case CREAR_DIRECTORIO:
		log_info(logger_de_deserializacion, "Es el codigo de 'creacion de direcorio', comenzando la deserializacion de parametros\n");
		directorio_a_crear_t* directorio=decifrar_directorio_a_crear(buffer);
		resultado=crear_directorio(directorio->path,directorio->mode);
		serializar_y_enviar_resultado(resultado,cliente_socket);
	break;

	case LISTAR_DIRECTORIO_Y_ARCHIVOS:
		log_info(logger_de_deserializacion, "Es el codigo de 'listado de directorio', comenzando la deserializacion de parametros\n");
		directorio_a_listar_t* directorio_a_listar=decifrar_directorio_a_listar(buffer);
		resultado=listar_directorio_y_archivos(directorio_a_listar);
		serializar_y_enviar_resultado(resultado,cliente_socket);
	break;

	case ELIMINAR_DIRECTORIO:
		log_info(logger_de_deserializacion, "Es el codigo de 'eliminacion de directorios', comenzando la deserializacion de parametros\n");
		paquete_decifrado=decifrar_directorio_a_borrar(buffer);
		resultado=eliminar_directorio(paquete_decifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
	break;

	case LISTAR_METADATA_DIRECTORIO_Y_ARCHIVOS:
		log_info(logger_de_deserializacion, "Es el codigo de 'listado de metadata', comenzando la deserializacion de parametros\n");
		paquete_decifrado=listar_metadata_directorio_y_archivos(buffer);
		resultado=listar_metadata(paquete_decifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	default:
		send(cliente_socket, "Codigo Invalido", 16, 0);
		log_info(logger_de_deserializacion, "Nos llego un codigo invalido\n");
	}
	free(resultado);
	free(paquete_decifrado);
}


//TODO hay que ver que entiende FUSE para realizar cada accion. Podemos crear un enum y castearlo arriba
void* decifrar_archivo_a_crear(void*buffer){

	return NULL;
}
void* decifrar_archivo_a_escribir(void*buffer){

	return NULL;
}
void* decifrar_archivo_a_leer(void*buffer){

	return NULL;
}
void* decifrar_archivo_a_borrar(void*buffer){

	return NULL;
}
directorio_a_crear_t* decifrar_directorio_a_crear(void*buffer){
	directorio_a_crear_t* directorio = malloc(sizeof(directorio_a_crear_t));
	int offset=0;
	int peso;
	memcpy(&peso, buffer+offset,sizeof(int));
	offset+=sizeof(int);
	directorio->path=(char*) malloc(peso);
	memcpy(directorio->path, buffer+offset,peso);
	offset+=peso;
	memcpy(&directorio->mode, buffer+offset,sizeof(mode_t));

	return directorio;
}
directorio_a_listar_t* decifrar_directorio_a_listar(void*buffer){
	directorio_a_listar_t* retorno;
	int offset=0;
	int peso;
	memcpy(&peso, buffer+offset,sizeof(int));
	offset+=sizeof(int);
	retorno->path=(char*) malloc(peso);
	memcpy(retorno->path, buffer+offset,peso);
	offset+=peso;
	memcpy(&peso, buffer+offset,sizeof(int));
	offset+=sizeof(int);
	retorno->string_nombre_de_archivos=(char*) malloc(peso);
	memcpy(retorno->string_nombre_de_archivos, buffer+offset,peso);

	return retorno;
}
void* decifrar_directorio_a_borrar(void*buffer){

	return NULL;
}

void* listar_metadata_directorio_y_archivos(void*buffer){


	return NULL;

}

void serializar_y_enviar_resultado(void* resultado,int cliente_socket){
/* ESTO QUEDO MUY BONITO, HAY QUE TESTEARLO. DESCONFIO DEMASIADO TODO
 *
 *
 * */

	/*DECLARO VARIABLES LOCALES*/
	int peso_total= sizeof(*resultado);
	int peso_paquete=peso_total+sizeof(int);

	/*ALOCO PAQUETE*/
	void* paquete=malloc(peso_paquete);

	/*ESCRIBO EN EL PAQUETE A ENVIAR, EL RESULTADO Y CUANTO PESA
	 * ASI PUEDEN ALOCARLO DESDE EL OTRO LADO*/
	memcpy(paquete,&peso_total,sizeof(int));
	memcpy(paquete+sizeof(int),resultado,peso_total);

	/*ENVIO EL PAQUETE*/
	send(cliente_socket, paquete, peso_paquete, 0);

	/*LIBERO RECURSOS*/
	free(paquete);

}
