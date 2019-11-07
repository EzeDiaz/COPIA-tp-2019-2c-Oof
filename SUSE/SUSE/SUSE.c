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
#include <commons/string.h>

#define ATTR_C11_THREAD ((void*)(uintptr_t)-1)


void funcion_test(char*);
int main(){

	//_suse_init();

	hilolay_t* hiloprueba;

	while(1){
		printf("hacemos 1 hilo\n");
		//suse_create(hiloprueba,NULL,funcion_test,NULL);
		printf("%d \n",hiloprueba->tid);
	}


	/*DESARROLLO*/

	//terminate_SUSE();
	return 0;
}

void funcion_test(char* cosa){
	printf("saraza");
}

void _suse_init(){

	/*INICIALIZO RECURSOS*/

	hilos_finalizados = list_create();

	/*INICIALIZO LOG*/

	incializar_logs_sistema();

	/*SETEAR CONFIG*/
	/*INICIALIZO DICCIONARIOS*/

	diccionario_de_procesos = dictionary_create();
	diccionario_procesos_x_queues = dictionary_create();
	diccionario_de_procesos_x_semaforo = dictionary_create();

	/*INICIALIZO COLAS*/

	cola_new = queue_create();
	cola_exit = queue_create();

	/*INICIALIZO SEMAFOROS*/

	/*Mutex*/
	sem_init(semaforo_log_colas,0,1);
	sem_init(semaforo_diccionario_de_procesos,0,1);
	sem_init(semaforo_diccionario_procesos_x_queues,0,1);
	sem_init(semaforo_diccionario_procesos_x_semaforo,0,1);
	sem_init(semaforo_lista_procesos_finalizados,0,1);
	sem_init(mutex_log_servidor,0,1);

	/*Contador*/

	sem_init(procesos_x_grado_de_multiprogramacion,0,0/*MULTIPROGRAMACION*/);
	sem_init(procesos_en_Ready,0,0);
	sem_init(procesos_en_New,0,0);
	sem_init(semaforo_lista_procesos_finalizados,0,0);

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




void terminate_SUSE(){


	/*LIBERO RECURSOS GLOBALES*/
	destruir_logs_sistema();


	//destruir_semaforos();

}

void aceptar_proceso(int PID){

	sem_t semaforo_exec;
	sem_init(&semaforo_exec,0,1);
	agregar_al_diccionario(PID,&semaforo_exec);
	sem_post(procesos_en_New);
}

void agregar_al_diccionario(int PID, sem_t* semaforo_exec){
	sem_wait(semaforo_diccionario_procesos_x_semaforo);
	dictionary_put(diccionario_de_procesos_x_semaforo,string_itoa(PID),semaforo_exec);
	sem_post(semaforo_diccionario_procesos_x_semaforo);
}


hilo_t* suse_scheduler_next(int PID){

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
	//OPTIMIZABLE, OJO CON LOS LEAKS DE LOS STRINGS

	bloquear_hilo(clave,hilo_a_bloquear);
	queue_push(cola_exec,hilo_a_ejecutar);

	free(clave);

	return NULL; //TODO
}


bool suse_wait(char* semaforo, int PID){

	return wait(semaforo,PID);
}


void* suse_signal(char* semaforo, int PID){
	//TODO
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



void hilolay_init(){
	_suse_init();
}

int suse_create(hilolay_t *thread, const hilolay_attr_t *attr, void *(*start_routine)(void *), void *arg, int socket){
	//TODO

	thread=(hilolay_t*)malloc(sizeof(hilolay_t*));
	pthread_t* hilo2;
	pthread_create(&hilo2, NULL, (void*)funcion_test, NULL);
	//memcpy(&(thread->tid),&hilo2,sizeof(pthread_t*));

	hilo_t* nuevo_hilo= (hilo_t*)malloc(sizeof(hilo_t*)+sizeof(hilolay_t*));


	proceso_t* proceso_correspondiente = obtener_proceso(socket);

	if(proceso_correspondiente->hilos_del_programa->elements_count<proceso_correspondiente->grado_de_multiprogramacion)
	{
		metricas_t* metricas=(metricas_t*)malloc(sizeof(metricas_t));
		metricas->tiempo_de_ejecucion=0;
		metricas->tiempo_de_espera=0;
		metricas->tiempo_de_uso_del_cpu=0;
		metricas->porcentaje_total_tiempo_de_ejecucion_de_hilos=0;
		nuevo_hilo->metricas=metricas;

		nuevo_hilo->PID=getpid();
		nuevo_hilo->estado_del_hilo=NEW;
		nuevo_hilo->hilo_informacion=thread;
		nuevo_hilo->prioridad;
		list_add(proceso_correspondiente->hilos_del_programa,nuevo_hilo);
		thread->tid=rand()%proceso_correspondiente->grado_de_multiprogramacion;
		pthread_detach(hilo2);
		return thread;
	}else{

		log_info(logger,"El proceso esta al mango \n");
	}


	return 0;


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


int hilolay_yield(void){
	return 0;

}

int hilolay_join(hilolay_t *thread){
	return 0;

}

int hilolay_get_tid(void){
	return 0;
}






