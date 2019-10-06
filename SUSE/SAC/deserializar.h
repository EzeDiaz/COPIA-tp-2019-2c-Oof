/*
 * deserializar.h
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */

#ifndef DESERIALIZAR_H_
#define DESERIALIZAR_H_
#include <commons/log.h>

t_log* logger_de_deserializacion;

//Prototipos
int determinar_protocolo(void*);
void identificar_paquete_y_ejecutar_comando(int, void*, int);

#endif /* DESERIALIZAR_H_ */
