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

#include <hilolay/alumnos.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define ATTR_C11_THREAD ((void*)(uintptr_t)-1)



int main(){

	/*SETEAR CONFIG -192.168.0.17*/
	leer_config();

	suse_init();

	/*DESARROLLO*/
	iniciar_servidor();

	//terminate_SUSE();
	liberar_recursos();
	return 0;
}


void suse_init(){

	/*INICIALIZO RECURSOS*/

	hilos_finalizados = list_create();

	/*INICIALIZO LOG*/

	incializar_logs_sistema();


	/*INICIALIZO COLAS*/

	cola_new = queue_create();
	cola_exit = queue_create();

	/*INICIALIZO SEMAFOROS*/

	/*Mutex*/
	sem_init(&semaforo_log_colas,0,1);
	sem_init(&semaforo_diccionario_de_procesos,0,1);
	sem_init(&semaforo_diccionario_procesos_x_queues,0,1);
	sem_init(&semaforo_diccionario_procesos_x_semaforo,0,1);
	//sem_init(&semaforo_lista_procesos_finalizados,0,1); ESTOY PROBANDO ALGO
	sem_init(&mutex_log_servidor,0,1);
	sem_init(&semaforo_estado_blocked,0,1);
	sem_init(&semaforo_diccionario_por_semaforo,0,1);

	/*Contador*/
	sem_init(&procesos_en_new,0,MAX_MULTIPROG);
	sem_init(&grado_de_multiprogramacion_contador,0,MAX_MULTIPROG);



	//sem_init(&semaforo_lista_procesos_finalizados,0,0);

	/*INICIALIZO DICCIONARIOS*/

	diccionario_de_procesos = dictionary_create();
	diccionario_procesos_x_queues = dictionary_create();
	diccionario_de_procesos_x_semaforo = dictionary_create();
	diccionario_bloqueados_por_semafaro = dictionary_create();
	diccionario_de_valor_por_semaforo = dictionary_create();


	/*Semaforos de SUSE*/

	int i=0;
	while(SEM_IDS[i]!=NULL){

	valores_semaforo_t* un_valor= (valores_semaforo_t*)malloc(sizeof(valores_semaforo_t));
	un_valor->valor_inicial=atoi(SEM_INIT[i]);
	un_valor->valor_max=atoi(SEM_MAX[i]);
	un_valor->valor_actual=un_valor->valor_inicial;
	dictionary_put(diccionario_de_valor_por_semaforo,SEM_IDS[i],un_valor);
	i++;
	}


	/*INICIALIZO HILOS*/

	hilo_t *hilo_new_to_ready;
	pthread_create(&hilo_new_to_ready,NULL,estadoNew,NULL);
	pthread_detach(hilo_new_to_ready);


}

void leer_config(){

	char* nombre_del_config = readline("Ingresar ruta del config:");
	t_config* un_config = config_create(nombre_del_config);

	//IP=(char*)malloc(50);
	//strncpy(IP ,config_get_string_value(un_config, "IP"),strlen(config_get_string_value(un_config, "IP")));
	IP = malloc(strlen(config_get_string_value(un_config,"IP")));
	strcpy(IP ,config_get_string_value(un_config, "IP"));
	PUERTO_ESCUCHA = config_get_int_value(un_config,"LISTEN_PORT");
	METRICS_TIMER = config_get_int_value(un_config,"METRICS_TIMER");
	MAX_MULTIPROG = config_get_int_value(un_config,"MAX_MULTIPROG");
	SEM_IDS = config_get_array_value(un_config,"SEM_IDS");
	SEM_INIT = config_get_array_value(un_config,"SEM_INIT");
	SEM_MAX = config_get_array_value(un_config,"SEM_MAX");
	ALPHA_SJF = config_get_double_value(un_config,"ALPHA_SJF");

	free(nombre_del_config);
	config_destroy(un_config);
}

void leer_config2(){

	structConfig * config;
	//char * montaje = strdup("");
	char* nombre_del_config = readline("Ingresar ruta del config:");
	t_config * configuracion = config_create(nombre_del_config);

	//config->IP = strdup(config_get_string_value(configuracion, "IP"));
	//string_append(&montaje, config_get_string_value(configuracion, "PUNTO_MONTAJE")); //IP
	string_append(&config->IP, config_get_string_value(configuracion, "IP"));
	string_append(&config->LISTEN_PORT, config_get_string_value(configuracion, "LISTEN_PORT"));
	config->METRICS_TIMER = config_get_int_value(configuracion, "METRICS_TIMER");
	config->MAX_MULTIPROG = config_get_int_value(configuracion, "MAX_MULTIPROG");
	config->SEM_IDS = config_get_array_value(configuracion, "SEM_IDS");
	config->SEM_INIT = config_get_array_value(configuracion, "SEM_INIT");
	config->SEM_MAX = config_get_array_value(configuracion, "SEM_MAX");
	config->ALPHA_SJF =  config_get_int_value(configuracion, "ALPHA_SJF");

	//log_info(alog, "Lee el archivo de configuracion");

	//config->montaje = obtenerMontaje(montaje);

	//free(montaje);
	config_destroy(configuracion);
}


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
	sem_destroy(&procesos_en_new);

	/*DICCIONARIOS*/

	void destructor_hilos(hilo_t* un_hilo){

		free(un_hilo->metricas);
		free(un_hilo->hilo_informacion);
		free(un_hilo);
	}

	void destructor_de_procesos(char* PID, proceso_t* un_proceso){
		free(PID);
		sem_destroy(&un_proceso->procesos_en_ready);

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

	destruir_logs_sistema();
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
	sem_post(&procesos_en_new);
}

void agregar_al_diccionario(int PID, sem_t* semaforo_exec){
	sem_wait(&semaforo_diccionario_procesos_x_semaforo);
	dictionary_put(diccionario_de_procesos_x_semaforo,string_itoa(PID),semaforo_exec);
	sem_post(&semaforo_diccionario_procesos_x_semaforo);
}


hilo_t* suse_schedule_next(int PID){

	char* pid=string_itoa(PID);

	t_queue* cola_Ready = obtener_cola_ready_de(pid);

	if(cola_Ready->elements!=0){
	// TODO REVISAR ESTE SJF
	void sjf(hilo_t* un_hilo){
		un_hilo->prioridad = calcular_sjf(un_hilo);
	}

	list_iterate(cola_Ready->elements,sjf);

	bool elemento_mas_grande(hilo_t* hilo_mas_prioridad,hilo_t*hilo_menor_prioridad){
		return hilo_mas_prioridad->prioridad > hilo_menor_prioridad->prioridad;
	}
	list_sort(cola_Ready->elements,elemento_mas_grande);

	free(pid);
	return list_get(cola_Ready->elements,0);
	}else{
		return NULL;
	}


}




int suse_join(int TID_que_quiero_ejecutar){
	hilo_t* hilo_a_ejecutar=buscar_hilo_por_TID(TID_que_quiero_ejecutar);
	int PID=hilo_a_ejecutar->PID;


	char* tid=string_itoa(TID_que_quiero_ejecutar);
	char* pid=string_itoa(PID);

	proceso_t* un_proceso= obtener_proceso(PID);
	t_queue* cola_exec= obtener_cola_exec_de(pid);
	hilo_t* hilo_a_bloquear=queue_pop(cola_exec);
	if(hilo_a_bloquear==NULL){
		free(tid);
		free(pid);


		//TODO loguear el error; no hay hilos a los cuales joinear
		return  -1;

	}

	bloquear_hilo(un_proceso->lista_de_bloqueados,hilo_a_bloquear);
	queue_push(cola_exec,hilo_a_ejecutar);

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
	//char* pid=(char*)malloc(6);
	//pid =string_itoa(PID);
	char* pid = string_itoa(PID);
	proceso_t* un_proceso;
	un_proceso =malloc(sizeof(proceso_t));
	un_proceso->hilos_del_programa=list_create();
	un_proceso->lista_de_bloqueados=list_create();

	dictionary_put(diccionario_de_procesos, pid ,un_proceso);
	t_list* vector_queues=list_create();
	t_queue* cola_ready=queue_create();
	list_add_in_index(vector_queues,COLA_READY,cola_ready);
	t_queue* cola_exec=queue_create();
	list_add_in_index(vector_queues,COLA_EXEC,cola_exec);


	dictionary_put(diccionario_procesos_x_queues,pid, vector_queues);
	pthread_mutex_t semaforo_exec_x_proceso;
	sem_t* semaforo_procesos_en_ready=(sem_t*)malloc(sizeof(sem_t));

	sem_init(semaforo_procesos_en_ready,0,0);
	un_proceso->procesos_en_ready=semaforo_procesos_en_ready;
	sem_init(&semaforo_exec_x_proceso,0,1);
	dictionary_put(diccionario_de_procesos_x_semaforo,pid,&semaforo_exec_x_proceso);
	pthread_t* un_hilo;
	pthread_t* otro_hilo;
	pthread_create(&un_hilo, NULL, estadoReady, PID);
	pthread_detach(un_hilo);
	pthread_create(&otro_hilo,NULL,readyToExec,PID);
	pthread_detach(otro_hilo);

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

	char* pid= string_itoa(socket);
	if(dictionary_has_key(diccionario_de_procesos,pid)){

		un_proceso = dictionary_get(diccionario_de_procesos, pid);


	}else{

		return -1;

	}


	encolar_en_new(hilo);

	sem_post(&un_proceso->procesos_en_ready);
	list_add(un_proceso->hilos_del_programa,hilo);


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
