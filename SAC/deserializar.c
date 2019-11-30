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
	creacion* creacion;

	switch(codigo_de_operacion){

	case CREAR_ARCHIVO:
		log_info(logger_de_deserializacion, "Es el codigo de 'creacion de archivos', comenzando la deserializacion de parametros\n");
		creacion=decifrar_creacion(buffer);
		int flag=crear_archivo(creacion->path,creacion->mode);
		resultado= serializar_flag(flag);
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
		creacion=decifrar_creacion(buffer);
		int flag_resultado=crear_directorio(creacion->path,creacion->mode);
		resultado=serializar_flag(flag_resultado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case LISTAR_DIRECTORIO_Y_ARCHIVOS:
		log_info(logger_de_deserializacion, "Es el codigo de 'listado de directorio', comenzando la deserializacion de parametros\n");
		directorio_a_listar_t* directorio_a_listar=decifrar_directorio_a_listar(buffer);
		resultado=listar_directorio_y_archivos(directorio_a_listar->path,directorio_a_listar->string_nombre_de_archivos);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case ELIMINAR_DIRECTORIO:
		log_info(logger_de_deserializacion, "Es el codigo de 'eliminacion de directorios', comenzando la deserializacion de parametros\n");
		char* path=decifrar_directorio_a_borrar(buffer);
		resultado=eliminar_directorio(path);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case LISTAR_METADATA_DIRECTORIO_Y_ARCHIVOS:
		log_info(logger_de_deserializacion, "Es el codigo de 'listado de metadata', comenzando la deserializacion de parametros\n");
		paquete_decifrado=listar_metadata_directorio_y_archivos(buffer);
		resultado=listar_metadata(paquete_decifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case GET_ATTRIBUTES:

		log_info(logger_de_deserializacion, "Es el codigo de 'listado de metadata', comenzando la deserializacion de parametros\n");
		char*ruta=decifrar_get_atributes(buffer);
		resultado=obtener_atributos(ruta);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;
	case MKNOD:
		log_info(logger_de_deserializacion, "Es el codigo de 'listado de metadata', comenzando la deserializacion de parametros\n");
		mknod_params* parametros=decifrar_mknod(buffer);
		resultado=realizar_mknod(parametros->name,parametros->mode,parametros->dev);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case RENAME:
		log_info(logger_de_deserializacion, "Es el codigo de 'listado de metadata', comenzando la deserializacion de parametros\n");
		rename_params*params=decifrar_rename(buffer);
		resultado=obtener_atributos(params->old_path,params->new_path,params->flag);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;



	default:
		send(cliente_socket, "Codigo Invalido", 16, 0);
		log_info(logger_de_deserializacion, "Nos llego un codigo invalido\n");
		resultado=malloc(1);
	}
	free(resultado);
	//free(paquete_decifrado);
}


//TODO hay que ver que entiende FUSE para realizar cada accion. Podemos crear un enum y castearlo arriba
void* decifrar_archivo_a_crear(void*buffer){

	return NULL;
}
archivo_descifrado_escritura* decifrar_archivo_a_escribir(void*paquete){

	archivo_descifrado_escritura* archivo= malloc(sizeof(archivo_descifrado_escritura));
	int offset=0;
	int peso_buffer;
	void* buf;
	int fd;
	int peso_count;
	size_t count;
	offset+=sizeof(int);
	memcpy(&fd,paquete+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(&peso_buffer,paquete+offset,sizeof(int));
	offset+=sizeof(int);
	buf= malloc(peso_buffer);
	memcpy(buf,paquete+offset,peso_buffer);
	offset+=peso_buffer;
	memcpy(&peso_count,paquete+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(&count,paquete+offset,peso_count);//si rompe aca quizas hay que alocar el count, aunque creo que no
	offset+=peso_count;

	archivo->fd=fd;
	archivo->buf=buf;
	archivo->count=count;
	return archivo;

}
void* decifrar_archivo_a_leer(void*buffer){

	return NULL;
}
void* decifrar_archivo_a_borrar(void*buffer){

	return NULL;//creo que el borrar directorio tambien borra Archivos TODO
}
creacion* decifrar_creacion(void*buffer){
	creacion* directorio = malloc(sizeof(creacion));
	int offset=sizeof(int);
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

char* decifrar_directorio_a_borrar(void*buffer){
	int desplazamiento=0;
	int longitud_a_copiar=0;
	char* nombre_del_path;

	desplazamiento+=sizeof(int);
	memcpy(&longitud_a_copiar,buffer+desplazamiento,sizeof(int));
	desplazamiento+=sizeof(int);
	nombre_del_path= malloc(longitud_a_copiar);
	memcpy(nombre_del_path,buffer+desplazamiento,longitud_a_copiar);

	return nombre_del_path;
}

void* listar_metadata_directorio_y_archivos(void*buffer){


	return NULL;

}

mknod_params* decifrar_mknod(void*buffer){

	mknod_params* parametros=(mknod_params*)malloc(sizeof(mknod_params));
	int peso=0;
	int peso_path;

	int offset=0;

	memcpy(&peso_path,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(parametros->name,buffer+offset,peso_path);
	offset+=peso_path;
	memcpy(parametros->mode,buffer+offset,sizeof(mode_t));
	offset+=sizeof(mode_t);
	memcpy(parametros->dev,buffer+offset,sizeof(dev_t));

	return parametros;


}
rename_params*decifrar_rename(void*buffer){

	rename_params* parametros= (rename_params*)malloc(sizeof(rename_params));

	int peso_old_path;
	int peso_new_path;
	int offset=0;

	memcpy(parametros->flag,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(&peso_old_path,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(parametros->old_path,buffer+offset,peso_old_path);
	offset+=peso_old_path;
	memcpy(&peso_new_path,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(parametros->new_path,buffer+offset,peso_new_path);
	offset+=peso_new_path;


	return parametros;
}


char* decifrar_get_atributes(void* buffer){

	int desplazamiento=0;
	int longitud_a_copiar=0;
	char* nombre_del_path;

	desplazamiento+=sizeof(int);
	memcpy(&longitud_a_copiar,buffer+desplazamiento,sizeof(int));
	desplazamiento+=sizeof(int);
	nombre_del_path= malloc(longitud_a_copiar);
	memcpy(nombre_del_path,buffer+desplazamiento,longitud_a_copiar);

	return nombre_del_path;




}

void serializar_y_enviar_resultado(void* resultado,int cliente_socket){
	int peso_total;
	int peso_paquete=peso_total+sizeof(int);
	memcpy(&peso_total,resultado,sizeof(int));


	send(cliente_socket, resultado, peso_paquete+sizeof(int), 0);


}
