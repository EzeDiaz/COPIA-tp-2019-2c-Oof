/*
 * funciones_aux.c
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */

#include "funciones_aux.h"
#include <commons/collections/dictionary.h>

float calcular_sjf(hilo_t*un_hilo){

return ( (1-alpha)* un_hilo->prioridad + alpha * un_hilo->comandos->elements_count);


}

t_queue* obtener_cola_ready_de(int PID){
	return obtener_cola_de(PID,COLA_READY);
}

t_queue* obtener_cola_exec_de(int PID){
	return obtener_cola_de(PID,COLA_EXEC);
}

t_queue* obtener_cola_de(int PID, int cola){
	sem_wait(semaforo_diccionario_procesos_x_queues);
	t_queue** vector_cola = dictionary_get(diccionario_procesos_x_queues,PID);

	sem_post(&semaforo_diccionario_procesos_x_queues);
	return vector_cola[cola];
}
