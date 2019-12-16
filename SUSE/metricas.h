#ifndef METRICAS_H_
#define METRICAS_H_

#include "TADs.h"
#include "semaforos_hilolay.h"
#include <commons/string.h>
#include <commons/log.h>
#include <string.h>

/*PROTOTIPOS*/
void mostrar_metricas_cada_delta_t();
void mostrar_metricas();
void metricas_por_sistema();
void metricas_por_programa();
void metricas_por_hilo();
int cantidad_hilos(proceso_t*);
void metricas_de_hilos_por_estado(char*,proceso_t*);
int cantidad_hilos_en_el_estado(proceso_t* , int);
int obtener_tiempo_total_de(proceso_t*);

#endif /* METRICAS_H_ */
