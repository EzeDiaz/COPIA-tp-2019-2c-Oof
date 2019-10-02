/*
 * TADs.h
 *
 *  Created on: 18 sep. 2019
 *      Author: utnso
 */

#ifndef TADS_H_
#define TADS_H_

#include <commons/collections/list.h>
#include <stdlib.h>

typedef struct{

	char* id;
	int valor_actual;//este tipo de dato esta rancio TODO

}semaforo_t;


enum estado{
	NEW = 0,
	READY = 1,
	EXECUTE=2,
	BLOCKED = 3,
	EXIT=4

};

typedef struct{

	int estado_del_proceso;
	int cantidad_de_hilos;
	t_list* hilos_del_programa;
	int grado_de_multiprogramacion;
	int grado_de_multiprogramacion_actual;

}programa_t;

typedef struct{
	char* funcion; //Esto tiene olor a metaprogramacion donde no deberia(? ??
	int PID; // Del programa al que pertenece
	int TID; // Unico por thread
	int estado_del_hilo;
}hilo_t;

#endif /* TADS_H_ */
