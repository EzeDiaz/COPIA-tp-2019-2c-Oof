#include "SUSE.h"

#define ATTR_C11_THREAD ((void*)(uintptr_t)-1)

int main(){

	/*SETEAR CONFIG*/
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

	/*INICIALIZO LOG*/

	incializar_logs_sistema();


	/*INICIALIZO COLAS*/

	cola_new = queue_create();
	cola_exit = queue_create();
	bloqueados=list_create();

	/*INICIALIZO SEMAFOROS*/

	/*Mutex*/
	sem_init(&semaforo_log_colas,0,1);
	sem_init(&semaforo_diccionario_de_procesos,0,1);
	sem_init(&semaforo_diccionario_procesos_x_queues,0,1);
	sem_init(&semaforo_diccionario_procesos_x_semaforo,0,1);
	sem_init(&semaforo_diccionario_por_semaforo,0,1);

	sem_init(&semaforo_lista_procesos_finalizados,0,1);
	sem_init(&mutex_log_servidor,0,1);
	sem_init(&mutex_lista_block,0,1);
	sem_init(&mutex_diccionario,0,1);
	sem_init(&mutex_lista,0,1);
	sem_init(&mutex_cola_new,0,1);
	sem_init(&mutex_calcular_sjf,0,1);
	sem_init(&mutex_elementos_en_new,0,1);
	sem_init(&mutex_log_metricas,0,1);
	sem_init(&mutex_new_to_ready,0,1);
	sem_init(&mutex_exec,0,1);
	sem_init(&mutex_encolar_new,0,1);
	sem_init(&mutex_suse_schedule_next,0,1);
	sem_init(&mutex_listas_metricas,0,1);
	sem_init(&mutex_obtener_tiempo_total,0,1);
	sem_init(&mutex_lista_hilos_programa,0,1);
	sem_init(&mutex_wait,0,1);
	sem_init(&mutex_signal,0,1);
	sem_init(&mutex_cola_exit,0,1);
	sem_init(&mutex_cronometro,0,1);
	sem_init(&mutex_lista_bloqueados,0,1);
	sem_init(&mutex_bloquear_hilo,0,1);
	sem_init(&mutex_diccionario_sincro_wait_signal,0,1);
	sem_init(&mutex_desbloquear_hilo,0,1);
	/*Contador*/
	sem_init(&procesos_en_new,0,MAX_MULTIPROG);
	sem_init(&grado_de_multiprogramacion_contador,0,MAX_MULTIPROG);
	//sem_init(&semaforo_lista_procesos_finalizados,0,0);

	/*Binario*/
	sem_init(&sem_encolar_en_new,0,0);

	/*INICIALIZO DICCIONARIOS*/

	diccionario_de_procesos = dictionary_create();
	diccionario_procesos_x_queues = dictionary_create();
	diccionario_de_procesos_x_semaforo = dictionary_create();
	diccionario_bloqueados_por_semafaro = dictionary_create();
	diccionario_de_valor_por_semaforo = dictionary_create();
	diccionario_sincro_wait_signal=dictionary_create();


	/*Semaforos de SUSE*/

	int i=0;
	while(SEM_IDS[i]!=NULL){


		valores_semaforo_t* un_valor= (valores_semaforo_t*)malloc(sizeof(valores_semaforo_t));
		un_valor->valor_inicial=atoi(SEM_INIT[i]);
		un_valor->valor_max=atoi(SEM_MAX[i]);
		un_valor->valor_actual=atoi(SEM_INIT[i]);
		pthread_mutex_t* mutex=(pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
		sem_init(mutex,0,1);
		dictionary_put(diccionario_de_valor_por_semaforo,SEM_IDS[i],un_valor);
		dictionary_put(diccionario_bloqueados_por_semafaro,SEM_IDS[i],list_create());
		dictionary_put(diccionario_sincro_wait_signal,SEM_IDS[i],mutex);


		i++;
	}

	/*INICIALIZO HILOS*/


	pthread_t* hilo_metricas;
	pthread_create(&hilo_metricas,NULL,mostrar_metricas_cada_delta_t,NULL);
	pthread_detach(hilo_metricas);

	suse_esta_atendiendo=true;

	pthread_t *hilo_new_to_ready;
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

void liberar_recursos(){

	/*Mutex*/
	sem_destroy(&semaforo_log_colas);
	sem_destroy(&semaforo_diccionario_de_procesos);
	sem_destroy(&semaforo_diccionario_procesos_x_queues);
	sem_destroy(&semaforo_diccionario_procesos_x_semaforo);
	sem_destroy(&semaforo_lista_procesos_finalizados);
	sem_destroy(&mutex_log_servidor);
	sem_destroy(&semaforo_diccionario_por_semaforo);
	sem_destroy(&mutex_lista_block);
	sem_destroy(&mutex_diccionario);
	sem_destroy(&mutex_lista);
	sem_destroy(&mutex_cola_new);
	sem_destroy(&mutex_calcular_sjf);
	sem_destroy(&mutex_elementos_en_new);
	sem_destroy(&mutex_log_metricas);
	sem_destroy(&mutex_new_to_ready);
	sem_destroy(&mutex_exec);
	sem_destroy(&mutex_encolar_new);
	sem_destroy(&mutex_suse_schedule_next);
	sem_destroy(&mutex_listas_metricas);
	sem_destroy(&mutex_obtener_tiempo_total);
	sem_destroy(&mutex_lista_hilos_programa);
	sem_destroy(&mutex_wait);
	sem_destroy(&mutex_signal);
	sem_destroy(&mutex_cola_exit);
	sem_destroy(&mutex_cronometro);
	sem_destroy(&mutex_lista_bloqueados);
	sem_destroy(&mutex_bloquear_hilo);
	sem_destroy(&mutex_desbloquear_hilo);

	/*Contador*/
	sem_destroy(&grado_de_multiprogramacion_contador);
	sem_destroy(&procesos_en_new);

	/*Binario*/
	sem_destroy(&sem_encolar_en_new);

	/*DICCIONARIOS*/

	void destructor_hilos(hilo_t* un_hilo){

		sem_destroy(&un_hilo->mutex_estado_del_hilo);
		sem_destroy(&un_hilo->mutex_tiempo_exec);
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

proceso_t* obtener_proceso(int socket){

	char* pid= string_itoa(socket);
	sem_wait(&semaforo_diccionario_de_procesos);
	proceso_t* un_proceso = dictionary_get(diccionario_de_procesos,pid);
	sem_post(&semaforo_diccionario_de_procesos);
	free(pid);
	return un_proceso;


	/*
	proceso_t* un_proceso = inicializar_proceso(socket);
	dictionary_put(diccionario_de_procesos,pid,un_proceso);
	return un_proceso;
	 */
}

proceso_t* inicializar_proceso(int socket){

	proceso_t* un_proceso = malloc(sizeof(proceso_t));
	un_proceso->hilos_del_programa = list_create();
	return un_proceso;
}

int _hilolay_init(int PID){

	char* pid = string_itoa(PID);
	proceso_t* un_proceso;
	t_list* vector_queues=list_create();
	t_queue* cola_ready=queue_create();
	t_queue* cola_exec=queue_create();

	un_proceso =(proceso_t*)malloc(sizeof(proceso_t));

	un_proceso->hilos_del_programa=list_create();

	sem_wait(&mutex_diccionario);
	un_proceso->diccionario_joineados_por_tid=dictionary_create();
	sem_post(&mutex_diccionario);

	sem_wait(&semaforo_diccionario_de_procesos);
	dictionary_put(diccionario_de_procesos, pid ,un_proceso);
	sem_post(&semaforo_diccionario_de_procesos);

	sem_wait(&mutex_lista);
	list_add_in_index(vector_queues,COLA_READY,cola_ready);
	sem_post(&mutex_lista);

	sem_wait(&mutex_lista);
	list_add_in_index(vector_queues,COLA_EXEC,cola_exec);
	sem_post(&mutex_lista);

	sem_wait(&mutex_diccionario);
	dictionary_put(diccionario_procesos_x_queues,pid, vector_queues);
	sem_post(&mutex_diccionario);

	sem_init(&un_proceso->semaforo_scheduler_next,0,1);
	sem_init(&un_proceso->semaforo_close,0,1);
	sem_init(&un_proceso->semaforo_join,0,1);
	sem_init(&un_proceso->mutex_cola_exec,0,1);
	sem_init(&un_proceso->mutex_cola_ready,0,1);
	sem_init(&un_proceso->proceso_en_exec,0,1);
	sem_init(&un_proceso->mutex_lista_hilos,0,1);
	sem_init(&un_proceso->mutex_operaciones,0,1);
	sem_init(&un_proceso->procesos_en_ready,0,0);
	sem_init(&un_proceso->consumir_hilo_exec,0,0);
	pthread_t* un_hilo;
	pthread_t* otro_hilo;

	free(pid);
	return PID;

}

int suse_create(int tid, int socket){
	proceso_t* un_proceso;

	char* pid= string_itoa(socket);

	sem_wait(&semaforo_diccionario_de_procesos);
	bool flag = dictionary_has_key(diccionario_de_procesos,pid);
	sem_post(&semaforo_diccionario_de_procesos);

	if(flag){
		sem_wait(&semaforo_diccionario_de_procesos);
		un_proceso = dictionary_get(diccionario_de_procesos, pid);
		sem_post(&semaforo_diccionario_de_procesos);
	}else{
		return -1;
	}

	sem_wait(&un_proceso->mutex_operaciones);


	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo->PID = socket;
	hilo->hilo_informacion=malloc(sizeof(hilolay_t));
	hilo->hilo_informacion->tid = tid;
	hilo->estado_del_hilo = NEW;
	hilo->prioridad = 0;
	hilo->tiempos=malloc(sizeof(tiempos_t));
	hilo->tiempos->tiempo_en_ejecucion_real = 0;
	hilo->tiempos->tiempo_llegada_a_EXEC = 0;
	hilo->tiempos->tiempo_llegada_a_NEW = 0;
	hilo->tiempos->tiempo_llegada_a_READY = 0;
	hilo->tiempos->sumatoria_tiempos_en_EXEC = 0;
	hilo->tiempos->sumatoria_tiempos_en_READY = 0;
	hilo->metricas = malloc(sizeof(metricas_t));
	hilo->metricas->porcentaje_total_tiempo_de_ejecucion_de_hilos=0;
	hilo->metricas->tiempo_de_ejecucion=0;
	hilo->metricas->tiempo_de_espera=0;
	hilo->metricas->tiempo_de_uso_del_cpu=0;
	sem_init(&hilo->mutex_estado_del_hilo,0,1);
	sem_init(&hilo->mutex_tiempo_exec,0,1);
	sem_init(&hilo->mutex_sumatoria_exec,0,1);
	sem_init(&hilo->mutex_sumatoria_ready,0,1);
	hilo->es_conductor=list_is_empty(un_proceso->hilos_del_programa);



	sem_wait(&un_proceso->mutex_lista_hilos);
	list_add(un_proceso->hilos_del_programa,hilo);
	sem_post(&un_proceso->mutex_lista_hilos);

	sem_wait(&un_proceso->mutex_cola_exec);
	char*pid_exec=string_itoa(socket);
	t_queue*cola_exec= obtener_cola_exec_de(pid_exec);
	bool esta_vacia_la_cola_exec= cola_exec->elements->elements_count==0;
	free(pid_exec);
	sem_post(&un_proceso->mutex_cola_exec);

	if(esta_vacia_la_cola_exec && hilo->es_conductor){

		sem_wait(&mutex_encolar_new);
		encolar_en_new(hilo);
		sem_post(&mutex_encolar_new);
		sem_wait(&un_proceso->procesos_en_ready);

		t_queue* cola_ready = obtener_cola_ready_de(pid);

		bool es_conductor(hilo_t* conductor){
			return conductor->es_conductor;
		}
		sem_wait(&un_proceso->mutex_cola_ready);
		hilo_t*hilo_conductor=list_remove_by_condition(cola_ready->elements,es_conductor);
		sem_post(&un_proceso->mutex_cola_ready);

		sem_wait(&un_proceso->mutex_cola_exec);
		queue_push(cola_exec,hilo);
		sem_post(&un_proceso->mutex_cola_exec);
		sem_post(&un_proceso->consumir_hilo_exec);
		sem_wait(&hilo->mutex_estado_del_hilo);
		hilo->estado_del_hilo = EXECUTE;
		sem_post(&hilo->mutex_estado_del_hilo);

		sem_wait(&mutex_exec);
		exec(hilo);
		sem_post(&mutex_exec);
		poner_cola_ready_de(pid,cola_ready);

	}else{
		sem_wait(&mutex_encolar_new);
		encolar_en_new(hilo);
		sem_post(&mutex_encolar_new);
	}

	char* TID=string_itoa(tid);
	dictionary_put(un_proceso->diccionario_joineados_por_tid,TID,list_create());
	free(TID);
	sem_post(&un_proceso->mutex_operaciones);
	poner_cola_exec_de(pid,cola_exec);
	free(pid);
	return tid;
}

int suse_schedule_next(int PID){

	char* pid=string_itoa(PID);
	sem_wait(&semaforo_diccionario_de_procesos);
	proceso_t* un_proceso= dictionary_get(diccionario_de_procesos,pid);
	sem_post(&semaforo_diccionario_de_procesos);
	sem_wait(&un_proceso->mutex_operaciones);
	sem_wait(&un_proceso->semaforo_scheduler_next);
	t_queue* cola_exec= obtener_cola_exec_de(pid);
	hilo_t* hilo_de_exec;

	//**********ESTADO DE LAS COLAS***************//
	t_queue* cola_exec2= obtener_cola_exec_de(pid);
	t_queue* cola_ready2= obtener_cola_ready_de(pid);
	bloqueados;
	//*******************************************//

	if(cola_exec->elements->elements_count>0){

		sem_wait(&un_proceso->mutex_cola_exec);
		hilo_de_exec=queue_peek(cola_exec);
		sem_post(&un_proceso->mutex_cola_exec);
		sem_post(&un_proceso->proceso_en_exec);

		sem_wait(&hilo_de_exec->mutex_estado_del_hilo);
		hilo_de_exec->estado_del_hilo=EXECUTE;
		sem_post(&hilo_de_exec->mutex_estado_del_hilo);


		sem_post(&un_proceso->mutex_operaciones);
		sem_post(&un_proceso->semaforo_scheduler_next);
		poner_cola_exec_de(pid,cola_exec);


		//**********ESTADO DE LAS COLAS***************//
		t_queue* cola_exec2= obtener_cola_exec_de(pid);
		t_queue* cola_ready2= obtener_cola_ready_de(pid);
		bloqueados;
		//*******************************************//
		free(pid);
		return hilo_de_exec->hilo_informacion->tid;
	}

	sem_wait(&un_proceso->procesos_en_ready);
	t_queue* cola_ready = obtener_cola_ready_de(pid);




	bool es_conductor(hilo_t* conductor){
		return conductor->es_conductor;
	}
	sem_wait(&un_proceso->mutex_cola_ready);
	hilo_t*hilo_conductor=list_remove_by_condition(cola_ready->elements,es_conductor);
	sem_post(&un_proceso->mutex_cola_ready);

	if(hilo_conductor!=NULL){

		sem_wait(&un_proceso->proceso_en_exec);
		sem_wait(&un_proceso->mutex_cola_exec);
		queue_push(cola_exec,hilo_conductor);
		sem_post(&un_proceso->mutex_cola_exec);
		sem_post(&un_proceso->consumir_hilo_exec);

		sem_wait(&hilo_conductor->mutex_estado_del_hilo);
		hilo_conductor->estado_del_hilo = EXECUTE;
		sem_post(&hilo_conductor->mutex_estado_del_hilo);

		sem_wait(&mutex_exec);
		exec(hilo_conductor);
		sem_post(&mutex_exec);




		sem_wait(&semaforo_log_colas);
		log_info(log_colas,"Se paso el proceso a Exec \n");
		sem_post(&semaforo_log_colas);


		sem_post(&un_proceso->semaforo_scheduler_next);
		sem_post(&un_proceso->mutex_operaciones);
		poner_cola_exec_de(pid,cola_exec);
		poner_cola_ready_de(pid,cola_ready);


		//**********ESTADO DE LAS COLAS***************//
		t_queue* cola_exec2= obtener_cola_exec_de(pid);
		t_queue* cola_ready2= obtener_cola_ready_de(pid);
		bloqueados;
		//*******************************************//
		free(pid);
		return hilo_conductor->hilo_informacion->tid;
	}

	void sjf(hilo_t* un_hilo){
		un_hilo->prioridad = calcular_sjf(un_hilo);
	}
	bool prioridad_cero(hilo_t* hilo_de_ready){
		return hilo_de_ready->prioridad==0;
	}

	sem_wait(&un_proceso->mutex_cola_ready);
	list_iterate(cola_ready->elements,sjf);
	sem_post(&un_proceso->mutex_cola_ready);


	bool elemento_mas_grande(hilo_t* hilo_mas_prioridad,hilo_t*hilo_menor_prioridad){
		return hilo_mas_prioridad->prioridad <= hilo_menor_prioridad->prioridad;
	}
	sem_wait(&un_proceso->mutex_cola_ready);
	list_sort(cola_ready->elements,elemento_mas_grande);
	sem_post(&un_proceso->mutex_cola_ready);

	sem_wait(&un_proceso->mutex_cola_ready);
	hilo_t* un_hilo= queue_pop(cola_ready);
	sem_post(&un_proceso->mutex_cola_ready);

	sem_wait(&un_proceso->proceso_en_exec);
	sem_wait(&un_proceso->mutex_cola_exec);
	queue_push(cola_exec,un_hilo);
	sem_post(&un_proceso->mutex_cola_exec);
	sem_post(&un_proceso->consumir_hilo_exec);

	sem_wait(&un_hilo->mutex_estado_del_hilo);
	un_hilo->estado_del_hilo = EXECUTE;
	sem_post(&un_hilo->mutex_estado_del_hilo);

	sem_wait(&mutex_exec);
	exec(un_hilo);
	sem_post(&mutex_exec);




	sem_wait(&semaforo_log_colas);
	log_info(log_colas,"Se paso el proceso a Exec \n");
	sem_post(&semaforo_log_colas);

	poner_cola_exec_de(pid,cola_exec);
	poner_cola_ready_de(pid,cola_ready);


	sem_post(&un_proceso->semaforo_scheduler_next);
	sem_post(&un_proceso->mutex_operaciones);

	free(pid);
	return un_hilo->hilo_informacion->tid;

}

int suse_wait(char* semaforo, int TID,int PID){

	proceso_t* un_proceso=obtener_proceso(PID);

	sem_wait(&un_proceso->mutex_operaciones);
	sem_wait(&mutex_diccionario_sincro_wait_signal);
	pthread_mutex_t* mutex= dictionary_get(diccionario_sincro_wait_signal,semaforo);
	sem_post(&mutex_diccionario_sincro_wait_signal);

	sem_wait(mutex);

	sem_wait(&mutex_diccionario_sincro_wait_signal);
	dictionary_put(diccionario_sincro_wait_signal,semaforo,mutex);
	sem_post(&mutex_diccionario_sincro_wait_signal);

	int retorno=wait(semaforo,TID,PID);

	sem_wait(&mutex_diccionario_sincro_wait_signal);
	mutex= dictionary_get(diccionario_sincro_wait_signal,semaforo);
	sem_post(&mutex_diccionario_sincro_wait_signal);


	sem_post(mutex);

	sem_wait(&mutex_diccionario_sincro_wait_signal);
	dictionary_put(diccionario_sincro_wait_signal,semaforo,mutex);
	sem_post(&mutex_diccionario_sincro_wait_signal);


	sem_post(&un_proceso->mutex_operaciones);

	return retorno;
}

int suse_signal(char* semaforo,int PID){
	proceso_t* un_proceso=obtener_proceso(PID);
	sem_wait(&un_proceso->mutex_operaciones);

	sem_wait(&mutex_diccionario_sincro_wait_signal);
	pthread_mutex_t* mutex= dictionary_get(diccionario_sincro_wait_signal,semaforo);
	sem_post(&mutex_diccionario_sincro_wait_signal);

	sem_wait(mutex);
	int retorno=signal(semaforo,PID);
	sem_post(mutex);
	sem_post(&un_proceso->mutex_operaciones);

	return retorno;
}

int suse_join(int TID_que_quiero_ejecutar,int PID){


	//**********ESTADO DE LAS COLAS***************//
	char* pid=string_itoa(PID);
	t_queue* cola_exec2= obtener_cola_exec_de(pid);
	t_queue* cola_ready2= obtener_cola_ready_de(pid);
	bloqueados;
	//*******************************************//

	printf("\n EMPIEZO UN SUSE_JOIN \n");
	proceso_t* un_proceso= obtener_proceso(PID);
	sem_wait(&un_proceso->semaforo_join);
	sem_wait(&un_proceso->mutex_operaciones);
	hilo_t* hilo_a_ejecutar=buscar_hilo_por_TID(TID_que_quiero_ejecutar,PID);
	hilo_t* hilo_a_bloquear;
	sem_wait(&hilo_a_ejecutar->mutex_estado_del_hilo);
	bool flag_estado_ready=hilo_a_ejecutar->estado_del_hilo==READY;
	bool flag_estado_blocked=hilo_a_ejecutar->estado_del_hilo==BLOCKED;
	sem_post(&hilo_a_ejecutar->mutex_estado_del_hilo);

	char* tid=string_itoa(TID_que_quiero_ejecutar);
	//char* pid=string_itoa(PID);


	t_queue* cola_exec= obtener_cola_exec_de(pid);
	sem_wait(&un_proceso->consumir_hilo_exec);
	sem_wait(&un_proceso->mutex_cola_exec);
	hilo_a_bloquear=queue_pop(cola_exec);
	sem_post(&un_proceso->mutex_cola_exec);

	sem_post(&un_proceso->proceso_en_exec);


	if(hilo_a_bloquear==NULL){

		poner_cola_exec_de(pid,cola_exec);



		sem_post(&un_proceso->mutex_operaciones);
		sem_post(&un_proceso->semaforo_join);
		//TODO loguear el error; no hay hilos a los cuales joinear

		//**********ESTADO DE LAS COLAS***************//
		t_queue* cola_exec2= obtener_cola_exec_de(pid);
		t_queue* cola_ready2= obtener_cola_ready_de(pid);
		bloqueados;
		//*******************************************//
		free(tid);
		free(pid);
		return  -1;

	}


	if(flag_estado_ready){
		char* pid_para_pop= string_itoa(PID);
		t_queue* cola_ready= obtener_cola_ready_de(pid_para_pop);
		bool sacar_hilo_para_popear(hilo_t* hilo_a_popear){

			return hilo_a_popear->hilo_informacion->tid==TID_que_quiero_ejecutar;
		}

		sem_wait(&un_proceso->procesos_en_ready);
		sem_wait(&un_proceso->mutex_cola_exec);
		list_remove_by_condition(cola_ready->elements,sacar_hilo_para_popear);
		sem_post(&un_proceso->mutex_cola_exec);


		sem_wait(&un_proceso->proceso_en_exec);
		sem_wait(&un_proceso->mutex_cola_exec);
		queue_push(cola_exec,hilo_a_ejecutar);
		sem_post(&un_proceso->mutex_cola_exec);
		sem_post(&un_proceso->consumir_hilo_exec);

		sem_wait(&hilo_a_ejecutar->mutex_estado_del_hilo);
		hilo_a_ejecutar->estado_del_hilo = EXECUTE;
		sem_post(&hilo_a_ejecutar->mutex_estado_del_hilo);

		sem_wait(&mutex_exec);
		exec(hilo_a_ejecutar);
		sem_post(&mutex_exec);



		sem_wait(&semaforo_log_colas);
		log_info(log_colas,"Se paso el proceso a Exec \n");
		sem_post(&semaforo_log_colas);
		poner_cola_ready_de(pid_para_pop,cola_ready);

		free(pid_para_pop);

	}



	t_list* lista_de_joineados;



	lista_de_joineados=dictionary_get(un_proceso->diccionario_joineados_por_tid,tid);

	list_add(lista_de_joineados,hilo_a_bloquear->hilo_informacion->tid);
	sem_wait(&mutex_bloquear_hilo);
	meter_en_block(hilo_a_bloquear);
	sem_post(&mutex_bloquear_hilo);
	dictionary_put(un_proceso->diccionario_joineados_por_tid,tid,lista_de_joineados);

	poner_cola_exec_de(pid,cola_exec);






	sem_post(&un_proceso->mutex_operaciones);
	sem_post(&un_proceso->semaforo_join);
	printf("\n TERMINO UN SUSE_JOIN \n");

	//**********ESTADO DE LAS COLAS***************//
	t_queue* cola_exec3= obtener_cola_exec_de(pid);
	t_queue* cola_ready3= obtener_cola_ready_de(pid);
	bloqueados;
	//*******************************************//
	free(pid);
	free(tid);

	return 1;
}

void* suse_close(int TID,int PID){
	// Con el TID que me pasan yo tengo que identificar al hilo en cuestion
	// para poder mandarlo a EXIT

	//**********ESTADO DE LAS COLAS***************//
	char* pid=string_itoa(PID);
	t_queue* cola_exec2= obtener_cola_exec_de(pid);
	t_queue* cola_ready2= obtener_cola_ready_de(pid);
	bloqueados;
	hilo_t* hilo_new= queue_peek(cola_new);
	hilo_t* hilo_exit=queue_peek(cola_exit);
	hilo_t*hilo_ready=queue_peek(cola_ready2);
	hilo_t*hilo_exec=queue_peek(cola_exec2);
	hilo_t*hilo_block=list_get(bloqueados,0);
	//*******************************************//


	proceso_t* un_proceso= obtener_proceso(PID);
	sem_wait(&un_proceso->mutex_operaciones);
	t_queue* cola_exec= obtener_cola_exec_de(pid);
	hilo_t* hilo_de_exec;
	sem_wait(&un_proceso->consumir_hilo_exec);
	sem_wait(&un_proceso->mutex_cola_exec);
	hilo_de_exec=queue_pop(cola_exec);
	sem_post(&un_proceso->mutex_cola_exec);
	sem_post(&un_proceso->proceso_en_exec);

	if(hilo_de_exec->hilo_informacion->tid==TID){
		exit_thread(hilo_de_exec);
	}
	poner_cola_exec_de(pid,cola_exec);

	sem_post(&un_proceso->mutex_operaciones);

	//**********ESTADO DE LAS COLAS***************//
	t_queue* cola_exec3= obtener_cola_exec_de(pid);
	t_queue* cola_ready3= obtener_cola_ready_de(pid);
	bloqueados;
	//*******************************************//
	free(pid);
	return NULL;
}
