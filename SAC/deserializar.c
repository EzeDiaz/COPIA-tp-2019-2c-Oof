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
#include <sys/statvfs.h>

int determinar_protocolo(void* buffer){
	int codigo_de_operacion;
	int offset = 0;
	memcpy(&(codigo_de_operacion), (buffer+offset), sizeof(int));


	log_info(logger_de_deserializacion, "Obtuvimos el codigo de operacion \n");
	return codigo_de_operacion;
}

void identificar_paquete_y_ejecutar_comando(int cliente_socket, void* buffer){

	sem_wait(&mutex_log_servidor);
	log_info(logger_de_deserializacion, "Estamos por deserializar el codigo de operacion\n");
	sem_post(&mutex_log_servidor);
	int codigo_de_operacion=determinar_protocolo(buffer);
	void* resultado;
	void* paquete_decifrado;
	creacion* creacion;
	int flag;

	switch(codigo_de_operacion){

	case CREAR_ARCHIVO:
		log_info(logger_de_deserializacion, "Es el codigo de 'creacion de archivos', comenzando la deserializacion de parametros\n");
		creacion=decifrar_creacion(buffer);
		flag=crear_archivo(creacion->path,creacion->mode);
		resultado= serializar_flag(flag);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case ESCRIBIR_ARCHIVO:
		log_info(logger_de_deserializacion, "Es el codigo de 'escribir de archivos', comenzando la deserializacion de parametros\n");
		archivo_descifrado_escritura*escritura_params=decifrar_archivo_a_escribir(buffer);
		flag=escribir_archivo(escritura_params->string1,escritura_params->string2,escritura_params->size,escritura_params->offset);
		resultado= serializar_flag(flag);
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
		char* path_a_borrar=decifrar_archivo_a_borrar(buffer);
		flag=serializar(borrar_archivo(path_a_borrar));
		resultado= serializar_flag(flag);
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
		flag=eliminar_directorio(path);
		resultado= serializar_flag(flag);

		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case LISTAR_METADATA_DIRECTORIO_Y_ARCHIVOS:
		log_info(logger_de_deserializacion, "Es el codigo de 'listado de metadata', comenzando la deserializacion de parametros\n");
		paquete_decifrado=listar_metadata_directorio_y_archivos(buffer);
		flag=listar_metadata(paquete_decifrado);
		resultado= serializar_flag(flag);
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
		flag=realizar_mknod(parametros->name,parametros->mode,parametros->dev);
		resultado= serializar_flag(flag);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case RENAME:
		log_info(logger_de_deserializacion, "Es el codigo de 'renombrar', comenzando la deserializacion de parametros\n");
		rename_params*params=decifrar_rename(buffer);
		resultado=obtener_atributos(params->old_path,params->new_path,params->flag);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case ABRIR_ARCHIVO:
		log_info(logger_de_deserializacion, "Es el codigo de 'abrir archivos', comenzando la deserializacion de parametros\n");
		open_file_params*parametros_archivo=decifrar_openfile(buffer);
		flag=abrir_archivo(parametros_archivo->name,parametros_archivo->mode,parametros_archivo->flags);
		resultado= serializar_flag(flag);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;
	case ABRIR_DIRECTORIO:
		log_info(logger_de_deserializacion, "Es el codigo de 'abrir directorio', comenzando la deserializacion de parametros\n");
		char* path_directorio=decifrar_opendir(buffer);
		int flag_directorio=abrir_directorio(path_directorio);
		resultado= serializar_flag(flag_directorio);
		serializar_y_enviar_resultado(resultado,cliente_socket);

		break;

	case ACCESS:
		log_info(logger_de_deserializacion, "Es el codigo de 'acceder a', comenzando la deserializacion de parametros\n");
		access_params* access=decifrar_access(buffer);
		resultado=acceder(access->path,access->flags);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case TRUNCATE:

		log_info(logger_de_deserializacion, "Es el codigo de 'truncate', comenzando la deserializacion de parametros\n");
		truncate_params* truncate=decifrar_truncate(buffer);
		flag=truncar(truncate->path,truncate->offset);
		resultado= serializar_flag(flag);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case STATFS:

		log_info(logger_de_deserializacion, "Es el codigo de 'statfs', comenzando la deserializacion de parametros\n");
		statfs_params* statfs = decifrar_statfs(buffer);
		resultado=estadisticas_fs(statfs->path,statfs->stats);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	default:
		;
		char*mensaje=string_new();
		string_append(&mensaje,"Codigo Invalido");
		send(cliente_socket, mensaje, 16, 0);
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


open_file_params*decifrar_openfile(void* buffer){

	open_file_params*retorno= (open_file_params*)malloc(sizeof(open_file_params));
	int offset=0;
	int peso_path;
	int peso_mode;
	memcpy(&retorno->flags,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(&peso_mode,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(&retorno->mode,buffer+offset,peso_mode);
	offset+=peso_mode;
	memcpy(&peso_path,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	retorno->name=(char*)malloc(peso_path);
	memcpy(retorno->name,buffer+offset,peso_path);

	return retorno;
}

truncate_params* decifrar_truncate(void* buffer){

	truncate_params* params= (truncate_params*)malloc(sizeof(truncate_params));
	int offset=0;

	int peso_path=0;

	memcpy(&params->offset,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(&peso_path,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	params->path=malloc(peso_path);
	memcpy(params->path,buffer+offset,peso_path);

	return params;


}

statfs_params* decifrar_statfs(void* buffer){

	statfs_params* params= (statfs_params*)malloc(sizeof(statfs_params));
	params->stats=(struct statvfs*)malloc(sizeof(struct statvfs));
	int offset=sizeof(int);
	int peso_path=0;

	memcpy(&params->stats->__f_unused,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(&params->stats->f_bavail,buffer+offset,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(&params->stats->f_bfree,buffer+offset,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(&params->stats->f_blocks,buffer+offset,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(&params->stats->f_bsize,buffer+offset,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(&params->stats->f_ffree,buffer+offset,sizeof(__fsfilcnt64_t));
	offset+=sizeof(__fsfilcnt64_t);
	memcpy(&params->stats->f_files,buffer+offset,sizeof(__fsfilcnt64_t));
	offset+=sizeof(__fsfilcnt64_t);
	memcpy(&params->stats->f_frsize,buffer+offset,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(&params->stats->f_namemax,buffer+offset,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(&params->stats->__f_unused,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(&peso_path,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	params->path=malloc(peso_path);
	memcpy(params->path,buffer+offset,peso_path);

	return params;
}


access_params* decifrar_access(void* buffer){

	access_params* params= (access_params*)malloc(sizeof(access_params));
	int offset=0;

	int peso_path=0;

	memcpy(&params->flags,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(&peso_path,buffer+offset,sizeof(int));
	offset+=sizeof(int);
	params->path=malloc(peso_path);
	memcpy(params->path,buffer+offset,peso_path);

	return params;


}


char* decifrar_opendir(void*buffer){
	char* path;
	int longitud;

	memcpy(&longitud, buffer,sizeof(int));
	path=malloc(longitud);
	memcpy(path,buffer+sizeof(int),longitud);

	return path;
}

archivo_descifrado_escritura* decifrar_archivo_a_escribir(void*paquete){

	archivo_descifrado_escritura* archivo= malloc(sizeof(archivo_descifrado_escritura));
	int desplazamiento=0;
	int peso_string;

	desplazamiento+=sizeof(int);

	memcpy(paquete+desplazamiento,&peso_string,sizeof(int));
	desplazamiento+=sizeof(int);
	archivo->string1=malloc(peso_string);
	memcpy(paquete+desplazamiento,archivo->string1,sizeof(int));
	desplazamiento+=peso_string;
	memcpy(paquete+desplazamiento,&peso_string,sizeof(int));
	desplazamiento+=sizeof(int);
	archivo->string2=malloc(peso_string);
	memcpy(paquete+desplazamiento,archivo->string2,sizeof(int));
	desplazamiento+=peso_string;
	memcpy(paquete+desplazamiento,&archivo->size,sizeof(size_t));
	desplazamiento+=sizeof(size_t);
	memcpy(paquete+desplazamiento,&archivo->offset,sizeof(off_t));

	return archivo;

}
params_lectura* decifrar_archivo_a_leer(void*buffer){

	int offset=sizeof(int);

	params_lectura* params=(params_lectura*)malloc(sizeof(params_lectura));
	int peso_path=0;
	int peso_buf=0;

	memcpy(&params->size,buffer+offset,sizeof(size_t));
	offset+=sizeof(size_t);

	memcpy(&params->desplazamiento,buffer+offset,sizeof(off_t));
	offset+=sizeof(off_t);

	memcpy(&peso_path,buffer+offset,sizeof(int));
	offset+=sizeof(int);

	params->path=malloc(peso_path);
	memcpy(params->path,buffer+offset,peso_path);
	offset+=peso_path;

	memcpy(&peso_buf,buffer+offset,sizeof(int));
	offset+=sizeof(int);

	params->path=malloc(peso_buf);
	memcpy(params->buf,buffer+offset,peso_buf);
	offset+=peso_buf;

	return params;


}
char* decifrar_archivo_a_borrar(void*buffer){

	char* path;
	int peso;
	memcpy(&peso,buffer,sizeof(int));
	path=malloc(peso);
	memcpy(path,buffer+sizeof(int),peso);

	return path;//creo que el borrar directorio tambien borra Archivos TODO
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
	directorio_a_listar_t* retorno=(directorio_a_listar_t* )malloc(sizeof(directorio_a_listar_t));
	int offset=0;
	int peso;
	memcpy(&peso, buffer+offset,sizeof(int));
	offset+=sizeof(int);
	retorno->path=(char*) malloc(peso);
	memcpy(retorno->path, buffer+offset,peso);
	offset+=peso;
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
	memcpy(&peso_total,resultado,sizeof(int));
	send(cliente_socket, resultado, peso_total+sizeof(int), 0);


}

void* serializar(int valor){
	int peso=sizeof(int);
	void*cosa=malloc(sizeof(int)*2);
	memcpy(cosa,&peso,sizeof(int));
	memcpy(cosa+sizeof(int),&valor,sizeof(int));
	return cosa;
}


