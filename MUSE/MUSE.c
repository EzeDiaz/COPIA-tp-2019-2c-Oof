//Bibliotecas propias
#include <libMUSE.h>
#include <serverMUSE.h>

//Commons
#include <commons/string.h>

//Standards
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//Para poder pedir el pid
#include <unistd.h>

//Para usar sockets
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

int main(){
	/*
	remove("log_MPServer.log");
	logger = log_create("log_MPServer.log","ServidorMP", 0, LOG_LEVEL_DEBUG);	//hacemo' el log
	char* linea= readline("Path del config a utilizar: ");
	 */

	//Todas estas cosas van de una o se dan luego del init?

	//Traer las cosas necesarias del config
	//Inicializar semaforos
	//Malloc gigante
	//Inicializar metricas

	iniciar_servidor();

	return 0;
}
