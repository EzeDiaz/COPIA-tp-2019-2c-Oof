#ifndef FUNCIONES_AUX_H_
#define FUNCIONES_AUX_H_

#include "TADs.h"
#include "metricas.h"
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>

//Prototipos
//para sacar colas
t_queue* obtener_cola_ready_de(char* );
t_queue* obtener_cola_exec_de(char*);
t_queue* obtener_cola_de(char*, int);

//para poner colas
void poner_cola_ready_de(char* ,t_queue*) ;
void poner_cola_exec_de(char*,t_queue* );
void poner_cola_de(char*,int,t_queue*);

//cosas variadas
long calcular_sjf(hilo_t*);
void destruir_logs_sistema();
void incializar_logs_sistema();
void leer_config_y_setear();

hilo_t* buscar_hilo_por_TID(int,int);




#endif /* FUNCIONES_AUX_H_ */
