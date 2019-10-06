/*
 * SUSE.c
 *
 *  Created on: 18 sep. 2019
 *      Author: utnso
 */

#include "metricas.h"
#include "colas.h"
#include "globales.h"
#include <semaphore.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/string.h>

int main(){

	start_up();

	/*DESARROLLO*/

	terminate_SUSE();
	return 0;
}

void start_up(){

	/*INICIALIZO RECURSOS*/

	hilos_finalizados = list_create();

	/*INICIALIZO LOG*/

	incializar_log_sistema();

	/*INICIALIZO DICCIONARIOS*/

	diccionario_de_procesos = dictionary_create();
	diccionario_procesos_x_queues = dictionary_create();
	diccionario_de_procesos_x_semaforo = dictionary_create();

	/*INICIALIZO COLAS*/

	cola_new = queue_create();
	cola_exit = queue_create();
	cola_blocked = queue_create();

	/*INICIALIZO SEMAFOROS*/

	/*Mutex*/
	sem_init(&semaforo_log_colas,0,1);
	sem_init(&semaforo_diccionario_de_procesos,0,1);
	sem_init(&semaforo_diccionario_procesos_x_queues,0,1);
	sem_init(&semaforo_diccionario_procesos_x_semaforo,0,1);
	sem_init(&semaforo_lista_procesos_finalizados,0,1);
	sem_init(&mutex_log_servidor,0,1);

	/*Contador*/

	sem_init(&procesos_x_grado_de_multiprogramacion,0,0/*MULTIPROGRAMACION*/);
	sem_init(&procesos_en_Ready,0,0);
	sem_init(&procesos_en_New,0,0);
	sem_init(&semaforo_lista_procesos_finalizados,0,0);

}

void terminate_SUSE(){


	/*LIBERO RECURSOS GLOBALES*/
	destruir_log_sistema();


}

void aceptar_proceso(int PID){

	sem_t* semaforo_exec;
	sem_init(semaforo_exec,0,1);
	sem_wait(semaforo_diccionario_procesos_x_semaforo);
	dictionary_put(diccionario_de_procesos_x_semaforo,string_itoa(PID),semaforo_exec);
	sem_post(semaforo_diccionario_procesos_x_semaforo);
	sem_post(procesos_en_New);
}
















