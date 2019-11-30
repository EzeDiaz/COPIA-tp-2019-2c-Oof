/*
 * Caballeros_de_SisOp_Afinador.c
 *
 *  Created on: 29 nov. 2019
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include <hilolay/hilolay.h>
#include "hilolay_alumnos.c"

#define CANT_NOTAS 420

struct hilolay_sem_t* solo_hiper_mega_piola;
struct hilolay_sem_t* afinado;
int fin = 0;

void *preparar_solo()
{
	int i;
	for(i = 0;i<10000000;i++);

	for(i = 0; i < CANT_NOTAS; i++)
	{
		hilolay_wait(solo_hiper_mega_piola);
		hilolay_signal(afinado);
		hilolay_signal(solo_hiper_mega_piola);
	}
	printf("\nPude afinar %d veces en el tiempo que tuve\n", i);
	return 0;
}

int main(int argc, char *argv[])
{
	struct hilolay_t afinador;

	hilolay_init();
	init_internal(&hiloops);


	solo_hiper_mega_piola = hilolay_sem_open("solo_hiper_mega_piola");
	afinado = hilolay_sem_open("afinado");

	hilolay_create(&afinador, NULL, &preparar_solo, NULL);

	//hilolay_join(&afinador);
	//hilolay_wait(afinado);
	//hilolay_signal(afinado);

	//TODO no tenemos hilos en ready y rompe
	hilolay_sem_close(solo_hiper_mega_piola);
	hilolay_sem_close(afinado);


return hilolay_return(0);
}
