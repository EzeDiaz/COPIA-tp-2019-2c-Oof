/*
 * SAC_server.c
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <semaphore.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "globales.h"

void atender_cliente(int);


int main(){

	start_up();
	struct sockaddr_in direccion_servidor;
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = inet_addr(IP);
	direccion_servidor.sin_port = htons(PUERTO_ESCUCHA);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);
	sem_wait(&mutex_log_servidor);
	log_info(log_servidor, "Levantamos el servidor\n");
	sem_post(&mutex_log_servidor);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(servidor, (void*) &direccion_servidor, sizeof(direccion_servidor))!=0){
		sem_wait(&mutex_log_servidor);
		log_info(log_servidor, "Fallo el bind \n");
		sem_post(&mutex_log_servidor);
	}

	int cliente;
	printf("Listos para escuchar\n");

	sem_wait(&mutex_log_servidor);
	log_info(log_servidor,"Servidor listo para recibir un cliente\n");
	sem_post(&mutex_log_servidor);

	while(1){
		listen(servidor, 100);
		pthread_t* hilo;
		struct sockaddr_in direccion_cliente;
		unsigned tamanio_direccion = sizeof(struct sockaddr_in);
		cliente = accept(servidor, (void*) &direccion_cliente, &tamanio_direccion);

		sem_wait(&mutex_log_servidor);
		log_info(log_servidor, "Recibimos un cliente\n");
		sem_post(&mutex_log_servidor);
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

void start_up(){ //TODO
	NULL;
}

void eliminar_semaforos(){ //TODO
	NULL;
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

void realizar_request(){
	NULL;
}

void recibir_buffer(){
	NULL;
}

