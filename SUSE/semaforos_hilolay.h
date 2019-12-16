#ifndef SEMAFOROS_HILOLAY_H_
#define SEMAFOROS_HILOLAY_H_

#include "globales.h"
#include "TADs.h"
#include "funciones_aux.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

//DICCIONARIOS
t_dictionary* diccionario_de_valor_por_semaforo;

//PROTOTIPOS

/*int wait(char*,int,int);
int signal(char*,int);
void bloquear_hilo( hilo_t*);
int desbloquear_hilo(int,int);*/


//PROTOTIPOS 2


//cosas de wait
void bloquear_hilo_por_semaforo(hilo_t* ,char* );
int wait(char*,int ,int );
void meter_en_block( hilo_t* );
void decrementar_semaforo(valores_semaforo_t*,char* );
void bloquear(int ,int ,char* );
hilo_t* obtener_hilo_a_bloquear(proceso_t*,int,char*);


//cosas de signal

int signal(char*,int);
int desbloquear_hilo(int,int);

#endif /* SEMAFOROS_HILOLAY_H_ */
