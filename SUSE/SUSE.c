/*
 * SUSE.c
 *
 *  Created on: 18 sep. 2019
 *      Author: utnso
 */
#include "SUSE.h"
#include "funciones_aux.h"
#include "metricas.h"
#include "colas.h"
#include "globales.h"
#include "funciones_aux.h"
#include <unistd.h>
#include <pthread.h>
#include <hilolay/hilolay.h>
#include <semaphore.h>
#include <stdint.h>
#include <sys/types.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
#include <readline/readline.h>

#define ATTR_C11_THREAD ((void*)(uintptr_t)-1)

int main(){

	suse_init();

	/*DESARROLLO*/
	iniciar_servidor();

	//terminate_SUSE();
	liberar_recursos();
	return 0;
}


void suse_init(){

	/*INICIALIZO HILOS*/

	hilo_t *hilo_new_to_ready;
	pthread_create(&hilo_new_to_ready,NULL,estadoNew,NULL);
	pthread_detach(hilo_new_to_ready);

	/*INICIALIZO VARIABLES GLOBALES*/


	/*INICIALIZO RECURSOS*/

	hilos_finalizados = list_create();

	/*INICIALIZO LOG*/

	incializar_logs_sistema();

	/*SETEAR CONFIG*/

	leer_config();

	/*INICIALIZO DICCIONARIOS*/

	diccionario_de_procesos = dictionary_create();
	diccionario_procesos_x_queues = dictionary_create();
	diccionario_de_procesos_x_semaforo = dictionary_create();
	diccionario_bloqueados_por_semafaro = dictionary_create();

	/*INICIALIZO COLAS*/

	cola_new = queue_create();
	cola_exit = queue_create();

	/*INICIALIZO SEMAFOROS*/

	/*Mutex*/
	sem_init(&semaforo_log_colas,0,1);
	sem_init(&semaforo_diccionario_de_procesos,0,1);
	sem_init(&semaforo_diccionario_procesos_x_queues,0,1);
	sem_init(&semaforo_diccionario_procesos_x_semaforo,0,1);
	sem_init(&semaforo_lista_procesos_finalizados,0,1);
	sem_init(&mutex_log_servidor,0,1);
	sem_init(&semaforo_estado_blocked,0,1);
	sem_init(&semaforo_diccionario_por_semaforo,0,1);

	/*Contador*/
	sem_init(&grado_de_multiprogramacion_contador,0,MAX_MULTIPROG);
	sem_init(&procesos_en_New,0,0);
	sem_init(&semaforo_lista_procesos_finalizados,0,0);

}

void leer_config(){

	char* nombre_del_config = readline("Ingresar ruta del config:");
	t_config* un_config = config_create(nombre_del_config);

	ALPHA_SJF = config_get_int_value(un_config,"ALPHA_SJF")/100;
	PUERTO_ESCUCHA = config_get_int_value(un_config,"LISTEN_PORT");
	IP = config_get_string_value(un_config, "IP");
	METRICS_TIMER = config_get_int_value(un_config,"METRICS_TIMER");
	MAX_MULTIPROG = config_get_int_value(un_config,"MAX_MULTIPROG");
	SEM_IDS = config_get_array_value(un_config,"SEM_IDS");
	SEM_INIT = config_get_array_value(un_config,"SEM_INIT");
	SEM_MAX = config_get_array_value(un_config,"SEM_MAX");

	free(nombre_del_config);
	config_destroy(un_config);
}
/*int puesta_en_marcha_del_hilo(hilolay_t* hilo, hilolay_attr_t* atributos_del_hilo, void*(*funcion_main)(void*),void*  argumento){

	const struct hilolay_attr_t* atributo_auxiliar = (struct hilolay_attr_t*) atributos_del_hilo;
	struct hilolay_attr_t atributos_default;
	bool esta_libre_cpu = false;
	bool c11 = (atributos_del_hilo == ATTR_C11_THREAD);
	if(atributo_auxiliar == NULL || c11){
		lll_lock
	}


}*/


void liberar_recursos(){

	/*Mutex*/
	sem_destroy(&semaforo_log_colas);
	sem_destroy(&semaforo_diccionario_de_procesos);
	sem_destroy(&semaforo_diccionario_procesos_x_queues);
	sem_destroy(&semaforo_diccionario_procesos_x_semaforo);
	sem_destroy(&semaforo_lista_procesos_finalizados);
	sem_destroy(&mutex_log_servidor);
	sem_destroy(&semaforo_estado_blocked);
	sem_destroy(&semaforo_diccionario_por_semaforo);

	/*Contador*/
	sem_destroy(&grado_de_multiprogramacion_contador);
	sem_destroy(&procesos_en_New);
	sem_destroy(&semaforo_lista_procesos_finalizados);

	/*DICCIONARIOS*/

	void destructor_hilos(hilo_t* un_hilo){

		free(un_hilo->metricas);
		free(un_hilo->hilo_informacion);
		free(un_hilo);
	}

	void destructor_de_procesos(char* PID, proceso_t* un_proceso){
		free(PID);
		sem_destroy(&un_proceso->procesos_en_Ready);

		list_destroy_and_destroy_elements(un_proceso->hilos_del_programa,destructor_hilos);
	}

	dictionary_destroy_and_destroy_elements(diccionario_de_procesos,destructor_de_procesos);


	void destructor_de_queues(char* PID, t_queue* vector_queue[]){
		free(PID);
		queue_destroy_and_destroy_elements(vector_queue[COLA_EXEC],destructor_hilos);
		queue_destroy_and_destroy_elements(vector_queue[COLA_READY],destructor_hilos);
	}

	dictionary_destroy_and_destroy_elements(diccionario_procesos_x_queues,destructor_de_queues);


	void destructor_de_semaforos(char* PID, char* nombre_de_semaforo){
		free(PID);
		free(nombre_de_semaforo);
	}

	dictionary_destroy_and_destroy_elements(diccionario_de_procesos_x_semaforo,destructor_de_semaforos);


	void destructor_de_lista_de_bloqueados(char* PID, t_list* lista_de_bloqueados){
		free(PID);
		list_destroy_and_destroy_elements(lista_de_bloqueados,destructor_hilos);
	}

	dictionary_destroy_and_destroy_elements(diccionario_bloqueados_por_semafaro,destructor_de_lista_de_bloqueados);

	/*COLAS*/

	queue_destroy_and_destroy_elements(cola_new,destructor_hilos);
	queue_destroy_and_destroy_elements(cola_exit,destructor_hilos);

	destruir_logs_sistema(); //TODO falta destruir un par mas de logs

}

void terminate_SUSE(){


	/*LIBERO RECURSOS GLOBALES*/
	destruir_logs_sistema();


	//destruir_semaforos();

}

void aceptar_proceso(int PID){

	sem_t semaforo_exec;
	sem_init(&semaforo_exec,0,1);
	agregar_al_diccionario(PID,&semaforo_exec);
	sem_post(&procesos_en_New);
}

void agregar_al_diccionario(int PID, sem_t* semaforo_exec){
	sem_wait(&semaforo_diccionario_procesos_x_semaforo);
	dictionary_put(diccionario_de_procesos_x_semaforo,string_itoa(PID),semaforo_exec);
	sem_post(&semaforo_diccionario_procesos_x_semaforo);
}


hilo_t* suse_schedule_next(int PID){

	t_queue* cola_Ready = obtener_cola_ready_de(PID);

	void sjf(hilo_t* un_hilo){
		un_hilo->prioridad = calcular_sjf(un_hilo);
	}

	list_iterate(cola_Ready->elements,sjf);

	bool elemento_mas_grande(hilo_t* hilo_mas_prioridad,hilo_t*hilo_menor_prioridad){
		return hilo_mas_prioridad->prioridad>hilo_menor_prioridad->PID;
	}
	list_sort(cola_Ready->elements,elemento_mas_grande);

	return list_remove(cola_Ready->elements,0);


}




void* suse_join(int TID_que_quiero_ejecutar){

	hilo_t* hilo_a_ejecutar=buscar_hilo_por_TID(TID_que_quiero_ejecutar);
	int PID=hilo_a_ejecutar->PID;
	t_queue* cola_exec= obtener_cola_exec_de(PID);
	hilo_t* hilo_a_bloquear=list_get(cola_exec->elements,0);
	char* clave=string_new();
	string_append(&clave,string_itoa(PID));
	string_append(&clave,string_itoa(TID_que_quiero_ejecutar));
	//OPTIMIZABLE, OJO CON LOS LEAKS DE LOS STRINGS Todo

	bloquear_hilo(clave,hilo_a_bloquear);
	queue_push(cola_exec,hilo_a_ejecutar);

	free(clave);

	return 1;
}


bool suse_wait(char* semaforo, int PID){

	return wait(semaforo,PID);
}


void* suse_signal(char* semaforo, int PID){
	//Genero una operacion signal sobre el semaforo dado
	signal(semaforo,PID);
	//no sabemos que retorna, revisar issue #22
	return NULL;
}

void* suse_close(int TID){
	// Con el TID que me pasan yo tengo que identificar al hilo en cuestion
	// para poder mandarlo a EXIT
	hilo_t* un_hilo;
	un_hilo=buscar_hilo_por_TID(TID); //Esto esta matado
	exit_thread(un_hilo);
	return NULL;
}



int _hilolay_init(int PID){
			proceso_t* un_proceso;
			un_proceso =malloc(sizeof(proceso_t));
			un_proceso->hilos_del_programa=list_create();

			dictionary_put(diccionario_de_procesos, PID ,un_proceso);
			t_queue* vector_queues[2];
			vector_queues[COLA_READY]=queue_create();
			vector_queues[COLA_EXEC]=queue_create();
			dictionary_put(diccionario_procesos_x_queues,PID, vector_queues);

			hilo_t* un_hilo;
			pthread_create(un_hilo, NULL, estadoReady(PID), NULL);
			pthread_detach(un_hilo);

			return PID;

}

int suse_create(int tid, int socket){

	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo->PID = socket;
	hilo->hilo_informacion=malloc(sizeof(hilolay_t));
	hilo->hilo_informacion->tid = tid;
	hilo->estado_del_hilo = NEW;
	hilo->prioridad = 0;
	hilo->metricas = malloc(sizeof(metricas_t));
	proceso_t* un_proceso;

	if(dictionary_has_key(diccionario_de_procesos, socket)){

	un_proceso = dictionary_get(diccionario_de_procesos, socket);


	}else{

		return -1;

	}


	list_add(un_proceso->hilos_del_programa,hilo);

	encolar_en_new(hilo);

	return tid;
}

proceso_t* obtener_proceso(int socket){

	if(dictionary_has_key(diccionario_de_procesos,string_itoa(socket))){
		return dictionary_get(diccionario_de_procesos,string_itoa(socket));
	}

	proceso_t* un_proceso = inicializar_proceso(socket);
	dictionary_put(diccionario_de_procesos,string_itoa(socket),un_proceso);
	return un_proceso;
}

proceso_t* inicializar_proceso(int socket){

	proceso_t* un_proceso = malloc(sizeof(proceso_t));
	un_proceso->hilos_del_programa = list_create();
	return un_proceso;
}
