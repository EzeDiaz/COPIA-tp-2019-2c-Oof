#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <semaphore.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "deserializar.h"
#include "globales.h"

void atender_cliente(int );
void realizar_request(void *, int);
void eliminar_semaforos();
void* recibir_buffer(int* , int );


int iniciar_servidor(){

	remove("log_Servidor.log");
	logger = log_create("log_Servidor.log","Servidor", 0, LOG_LEVEL_DEBUG);

	//leer_config_y_setear();

	suse_esta_atendiendo = true;

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family= AF_INET;
	direccionServidor.sin_addr.s_addr= inet_addr(IP); //INADDR_ANY;
	direccionServidor.sin_port=htons(PUERTO_ESCUCHA);

	//El PUERTO_ESCUCHA deberia ser int (?)

	int servidor = socket(AF_INET, SOCK_STREAM , 0);
	sem_wait(&mutex_log_servidor);
	log_info(logger, "Levantamos el servidor\n");
	sem_post(&mutex_log_servidor);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if( bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0){
		sem_wait(&mutex_log_servidor);
		log_info(logger, "Fallo el bind \n ");
		sem_post(&mutex_log_servidor);
		return 1;
	}

	int cliente;
	printf("Soy Suse, estoy escuchando... \n");

	sem_wait(&mutex_log_servidor);
	log_info(logger, "Servidor listo para recibir un cliente\n");
	sem_post(&mutex_log_servidor);


	while(suse_esta_atendiendo){

		listen(servidor, 100);
		pthread_t *hilo;
		struct sockaddr_in direccionCliente;
		unsigned tamanioDireccion= sizeof(struct sockaddr_in);
		cliente = accept(servidor, (void*) &direccionCliente, &tamanioDireccion);

		sem_wait(&mutex_log_servidor);
		log_info(logger, "Recibimos un cliente\n");
		sem_post(&mutex_log_servidor);
		pthread_create(&hilo, NULL,  (void*)atender_cliente, cliente);
		pthread_detach(hilo);
		//cliente = accept(servidor, (void*) &direccionCliente, &tamanioDireccion);

	}




	return 0;
	sem_wait(&mutex_log_servidor);
	log_info(logger, "Cerro el servidor\n");
	sem_post(&mutex_log_servidor);
	printf("Cerro el servidor \n");
	log_destroy(logger);


	eliminar_semaforos();
	return 0;

}


void atender_cliente(int cliente){

	void* buffer;
	int alocador;

	sem_wait(&mutex_log_servidor);
	log_info(logger, "Recibimos conexion \n");
	sem_post(&mutex_log_servidor);

	buffer=recibir_buffer(&alocador,cliente);

	while(0<alocador){
		realizar_request(buffer, cliente);
		free(buffer);
		buffer=recibir_buffer(&alocador,cliente);
	}

	sem_wait(&mutex_log_servidor);
	log_info(logger, "Se desconecto el cliente\n");
	sem_post(&mutex_log_servidor);

	close(cliente);
}


void* recibir_buffer(int* alocador, int cliente){

	void* buffer;
	int resultado = recv(cliente, alocador, 4, MSG_WAITALL);
	if(resultado!=0){
		buffer = malloc(*alocador);
		recv(cliente, buffer,*alocador, MSG_WAITALL);
		return buffer;
	}else{
		*alocador=0;
		return buffer;
	}
}

void realizar_request(void *buffer,int cliente){

	identificar_paquete_y_ejecutar_comando(cliente, buffer);


}

void eliminar_semaforos(){


}

