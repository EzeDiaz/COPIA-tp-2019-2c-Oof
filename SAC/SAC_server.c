/*
 * SAC_server.c
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */

#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <semaphore.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <commons/config.h>
#include "globales.h"
#include "deserializar.h"
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <commons/bitarray.h>
#include <sys/mman.h>

void atender_cliente(int);
void* recibir_buffer(int* , int);
t_log* log_servidor;

int main(){

	remove("log_servidor.log");
	log_servidor = log_create("log_servidor.log","SAC_Servidor",0,LOG_LEVEL_DEBUG);
	obtener_datos_del_config();
	start_up();
	inicializar_globales();
	struct sockaddr_in direccion_servidor;
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = inet_addr(IP);
	direccion_servidor.sin_port = htons(PUERTO_ESCUCHA);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);
	log_info(log_servidor,"Levantamos el servidor\n");


	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(servidor, (void*) &direccion_servidor, sizeof(direccion_servidor))!=0){
		sem_wait(&mutex_log_servidor);
		log_info(log_servidor, "Fallo el bind \n");
		sem_post(&mutex_log_servidor);
	}

	int cliente;
	printf("Listos para escuchar\n");

	log_info(log_servidor,"Servidor listo para recibir un cliente\n");

	while(1){
		listen(servidor, 100);
		pthread_t* hilo;
		struct sockaddr_in direccion_cliente;
		unsigned tamanio_direccion = sizeof(struct sockaddr_in);
		cliente = accept(servidor, (void*) &direccion_cliente, &tamanio_direccion);

		log_info(log_servidor, "Recibimos un cliente\n");
		pthread_create(&hilo, NULL, (void*) atender_cliente, cliente);
		pthread_detach(hilo);
	}

	sem_wait(&mutex_log_servidor);
	log_info(log_servidor, "Cerro el servidor\n");
	sem_post(&mutex_log_servidor);
	printf("Cerro el servidor");
	log_destroy(log_servidor);

	eliminar_semaforos();
	return 0;


}
void inicializar_semaforos(){

	sem_init(&mutex_log_servidor, 0, 1);
	sem_init(&mutex_tabla_de_nodos,0,1);
	sem_init(&mutex_log_servidor,0,1);

}


void inicializar_globales(){

	//LOGGERS
	logger_de_deserializacion=log_create("log de deserializacion","deserializacion",0,LOG_LEVEL_INFO);

	//SEMAFOROS
	inicializar_semaforos();
}


void start_up(){
	char* comando = string_new();
	string_append(&comando,"dd if=/dev/zero iflag=fullblock of=");
	string_append(&comando,PUNTO_DE_MONTAJE);
	string_append(&comando,NOMBRE_DEL_DISCO);
	string_append(&comando," bs=");
	string_append(&comando,string_itoa(BLOCK_SIZE));
	string_append(&comando," count=");
	string_append(&comando,string_itoa(CANT_MAX_BLOQUES));
	int retorno = system(comando);
	free(comando);
	if(retorno<0){
		printf("No podemos levantar el FileSystem\n");
		exit(1);
	}
	comando=string_new();
	string_append(&comando,"./sac-format ");
	string_append(&comando,NOMBRE_DEL_DISCO);
	system(comando);
	free(comando);

	comando=string_new();
	string_append(&comando,"./sac-dump ");
	string_append(&comando,NOMBRE_DEL_DISCO);
	system(comando);

	free(comando);
	char* ruta = string_new();
	string_append(&ruta,PUNTO_DE_MONTAJE);
	string_append(&ruta,NOMBRE_DEL_DISCO);
	int file_descriptor_disco = open(ruta, O_RDWR, S_IRUSR | S_IWUSR);
	char* error=strerror(errno);
	printf(error);
	int cantidad_bloques= CANT_MAX_BLOQUES;
	primer_bloque = mmap(NULL, cantidad_bloques, PROT_READ | PROT_WRITE, MAP_SHARED,file_descriptor_disco, 0);

	crear_bitmap(config);
	setear_fs();



}


void setear_fs(){

	tamanio_disco=4294967296;
	int n_bloques=(tamanio_disco/BLOCK_SIZE/8)/BLOCK_SIZE;
	cantidad_de_bloques_reservados= 1025 +n_bloques;


	GFile* primer_nodo=buscar_nodo_libre();
	primer_nodo->parent_dir_block=NULL;
	primer_nodo->state=DIRECTORIO;
	primer_nodo->c_date=(long)time (NULL);
	primer_nodo->m_date=primer_nodo->c_date;
	primer_nodo->file_size=0;
	crear_vector_de_punteros(primer_nodo->blk_indirect,1000);

	for(int i=0; i<71;i++){
	primer_nodo->fname[i]=PUNTO_DE_MONTAJE[i];
	}
	escribir_en_disco(preparar_nodo_para_grabar(primer_nodo),buscar_bloque_libre(BUSQUEDA_NODO));
}

void crear_bitmap(t_config* config){



	//Tengo que traer a memoria el bitarray que ya hay en el FS
	char* ruta= string_new();
	string_append(&ruta,PUNTO_DE_MONTAJE);
	int cantidad_bloques=CANT_MAX_BLOQUES;
	if((cantidad_bloques % 8) == 0) {
		cantidad_bloques = cantidad_bloques / 8;
	} else {
		cantidad_bloques = (cantidad_bloques / 8) + 1;
	}

	int file_Desc_Bitarray = open(ruta, O_RDWR, S_IRUSR | S_IWUSR);

	//char* array_de_bits = mmap(NULL, cantidad_bloques, PROT_READ | PROT_WRITE, MAP_SHARED, file_Desc_Bitarray, 0);
	char* direccion_bitmap=(char*)(primer_bloque+BLOCK_SIZE);
	bitarray = bitarray_create_with_mode(direccion_bitmap,cantidad_bloques,LSB_FIRST);

	free(ruta);


}
void eliminar_semaforos(){ //TODO
	sem_destroy(&mutex_log_servidor);
	sem_destroy(&mutex_tabla_de_nodos);
	sem_destroy(&mutex_log_servidor);
}


void atender_cliente(int cliente_socket){

	void* buffer;
	int alocador;

	sem_wait(&mutex_log_servidor);
	log_info(log_servidor, "Recibimos una conexion\n");
	sem_post(&mutex_log_servidor);

	buffer = recibir_buffer(&alocador, cliente_socket);

	while(alocador>0){
		realizar_request(buffer, cliente_socket);
		free(buffer);
		buffer = recibir_buffer(&alocador, cliente_socket);

	}

	sem_wait(&mutex_log_servidor);
	log_info(log_servidor, "Se desconecto el cliente\n");
	sem_post(&mutex_log_servidor);

	close(cliente_socket);
}

void realizar_request(void *buffer, int cliente_socket){


	identificar_paquete_y_ejecutar_comando(cliente_socket, buffer);
	/*Deserializa y hace la opercion correspondiente*/
	log_info(log_servidor,"Terminamos el request\n");

}

void* recibir_buffer(int* alocador, int cliente_socket){

	void* buffer;

	if(recv(cliente_socket, alocador, 4, MSG_WAITALL)!=0){
		buffer = malloc(*alocador);
		recv(cliente_socket, buffer, *alocador, MSG_WAITALL);
		return buffer;
	}else{
		*alocador = 0;
		return buffer;
	}
}

void obtener_datos_del_config(){
	char* nombre_config;


	//IP = string_new();

	nombre_config = readline("Ingresar nombre del config a utilizar: ");

	config = config_create(nombre_config);

	char* aux = string_new();

	string_append(&aux, "Leimos el config: ");
	string_append(&aux, nombre_config);
	string_append(&aux, "\n");

	log_info(log_servidor, aux);
	free(aux);
	free(nombre_config);
	IP = malloc(10);
	IP = config_get_string_value(config, "IP");
	PUERTO_ESCUCHA = config_get_int_value(config, "PUERTO_ESCUCHA");
	PUNTO_DE_MONTAJE = malloc(50);
	PUNTO_DE_MONTAJE = config_get_string_value(config, "PUNTO_DE_MONTAJE");
	BLOCK_SIZE = config_get_int_value(config, "BLOCK_SIZE");
	CANT_MAX_BLOQUES = config_get_int_value(config, "CANT_MAX_BLOQUES");
	NOMBRE_DEL_DISCO= malloc(10);
	NOMBRE_DEL_DISCO= config_get_string_value(config, "NOMBRE_DEL_DISCO");
	mkdir(PUNTO_DE_MONTAJE,S_IRWXU);


}

