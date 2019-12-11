/*
 * Caballeros_de_SisOp_Afinador.c
 *
 *  Created on: 29 nov. 2019
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include <hilolay/hilolay.h>
#include <unistd.h>
#include "hilolay_alumnos.c"
#include <SUSE/SUSE-Cli.h>
#include <pthread.h>

#define CANT_NOTAS 420

struct hilolay_sem_t* solo_hiper_mega_piola;
struct hilolay_sem_t* afinado;
int fin = 0;

void *preparar_solo()
{
	int i;
	for(i = 0;i<20;i++){
		sleep(1);
		hilolay_yield();
	}

	for(i = 0; i < CANT_NOTAS; i++)
	{
		hilolay_wait(solo_hiper_mega_piola);
		hilolay_signal(afinado);
		hilolay_signal(solo_hiper_mega_piola);
	}
	printf("\nPude afinar %d veces en el tiempo que tuve\n", i);
	return 0;
}

void *tocar_solo(void* num)
{
	int cont = 0;

	for(int i = 0; i < CANT_NOTAS/4; i++)
	{
		hilolay_wait(afinado);
		hilolay_wait(solo_hiper_mega_piola);
		cont++;
		printf("%d: PARAPAPAM! Nota %d\n", num, cont);
		hilolay_signal(solo_hiper_mega_piola);
	}

	printf("\nPude tocar %d notas bien\n", cont);
	return 0;
}

int solea3();
int afina3();
int main(){


	pthread_t* afinador;
	pthread_t* soleador;

	pthread_create(&afinador,NULL,afina3,NULL);
	pthread_detach(afinador);

	pthread_create(&soleador,NULL,solea3,NULL);
	pthread_detach(soleador);



	while(1){}
}

int solea3(void){

	struct hilolay_t guitarrista[4];

		hilolay_init();

		solo_hiper_mega_piola = hilolay_sem_open("solo_hiper_mega_piola");
		afinado = hilolay_sem_open("afinado");

		hilolay_create(&guitarrista[0], NULL, &tocar_solo, (void*)0);
		hilolay_create(&guitarrista[1], NULL, &tocar_solo, (void*)1);
		hilolay_create(&guitarrista[2], NULL, &tocar_solo, (void*)2);
		hilolay_create(&guitarrista[3], NULL, &tocar_solo, (void*)3);

		hilolay_join(&guitarrista[0]);
		hilolay_join(&guitarrista[1]);
		hilolay_join(&guitarrista[2]);
		hilolay_join(&guitarrista[3]);

		hilolay_sem_close(solo_hiper_mega_piola);
		hilolay_sem_close(afinado);

		return hilolay_return(0);

}
int afina3(void)
{
	struct hilolay_t afinador;

	hilolay_init();

	solo_hiper_mega_piola = hilolay_sem_open("solo_hiper_mega_piola");
	afinado = hilolay_sem_open("afinado");

	hilolay_create(&afinador, NULL, &preparar_solo, NULL);

	hilolay_join(&afinador);

	hilolay_sem_close(solo_hiper_mega_piola);
	hilolay_sem_close(afinado);


return hilolay_return(0);
}


void hilolay_init(){

	_hilolay_init();
	init_internal(&hiloops);


}
