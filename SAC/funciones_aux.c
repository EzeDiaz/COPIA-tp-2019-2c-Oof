/*
 * funciones_aux.c
 *
 *  Created on: 20 nov. 2019
 *      Author: utnso
 */

//FUNCIONES AUXILIARES

#include "funciones_aux.h"

void* encontrar_posicion_en_disco(int numero_de_bloque){

	return (primer_bloque+ numero_de_bloque* BLOCK_SIZE);

}

bool verificar_path_este_permitido(char*path){

	//puede mejorar TODO
	return string_contains(path, PUNTO_DE_MONTAJE);
}


int buscar_nodo_libre(){

	int nro_de_nodo=-1;
	int cantidad_de_bloques=bitarray_get_max_bit(bitarray);
	int i=0;
	while (i<cantidad_de_bloques || nro_de_nodo<0){
		if(! bitarray_test_bit(bitarray,i))
		nro_de_nodo=i;

		i++;


	}
	return nro_de_nodo;

}

void crear_vector_de_punteros(ptrGBloque array_de_bloques[], int n){

	for(int i=0;i<n;i++){
		for(int j=0; j<1024;j++){

			array_de_bloques[i].bloques[j].bloque=NULL;
		}
	}
}


ptrGBloque* obtener_puntero_padre(char* path){


	char** vector= string_split(path, "/");

	int i =0;
	while(vector[i]!=NULL){
		i++;
	}

	char* nombre_nodo_padre=vector[i-2];

	nodo_t* nodo_padre =encontrar_en_tabla_de_nodos(nombre_nodo_padre);
	return nodo_padre->punteros_indirectos_simples;


}


char* obtener_nombre_de_archivo(char* path){

	char** vector= string_split(path, "/");

	int i =0;
	while(vector[i]!=NULL){
		i++;
	}

	return vector[i-2];


}


void leer_cada_archivo_y_borrar(char* path){


	char* directorio = string_new();
	struct dirent *dir;
	DIR* directorio_tabla= opendir(directorio);
	if(directorio_tabla != NULL){

		while ((dir = readdir(directorio_tabla)) != NULL){
			if(!strcmp(dir->d_name,"..") || !strcmp(dir->d_name, ".")){

			}else{
				borrar_archivo(strcat(path,dir->d_name));
			}
		}
	}
}

void* paquetizar_metadata_de_directorio(t_list*lista){

	int peso=103;
	int peso_total=((peso)* lista->elements_count)+sizeof(int)+sizeof(int);
	void* retorno=malloc(peso_total);
	int offset=0;
	memcpy(retorno+offset,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(retorno+offset,&lista->elements_count,sizeof(int));
	offset+=sizeof(int);

	void paquetizar(nodo_t* un_nodo){

		memcpy(retorno+offset ,&un_nodo->estado,1);//TODO esto puede romper
		offset+=1;
		memcpy(retorno+offset,&un_nodo->fecha_de_creacion,sizeof(long int));
		offset+=sizeof(long int);
		memcpy(retorno+offset,&un_nodo->fecha_de_modificacion,sizeof(long int));
		offset+=sizeof(long int);
		memcpy(retorno+offset,&un_nodo->puntero_padre,sizeof(int));
		offset+=sizeof(int);
		memcpy(retorno+offset,&un_nodo->tamanio_del_archivo,sizeof(int));
		offset+=sizeof(int);
		memcpy(retorno+offset,un_nodo->nombre_de_archivo,72);
		offset+=72;

	}
	list_iterate(lista, paquetizar);

	return retorno;
}

nodo_t* encontrar_en_tabla_de_nodos(char* nombre_de_nodo){

	nodo_t* buscar_nodo_por_nombre(char* nombre){

		for(int i=0;i<1024;i++){

			if(!strcmp(tabla_de_nodos[i]->nombre_de_archivo,nombre))
				return tabla_de_nodos[i];

		}

		return NULL;
	}
	sem_wait(mutex_tabla_de_nodos);
	nodo_t* el_nodo_a_retornar = buscar_nodo_por_nombre(nombre_de_nodo);
	sem_post(mutex_tabla_de_nodos);
	return el_nodo_a_retornar;
}

void escribir_en_disco(void* lo_que_quiero_escribir){

	int un_nodo = buscar_nodo_libre();
	void* posicion = encontrar_posicion_en_disco(un_nodo);

	memcpy(posicion,lo_que_quiero_escribir,sizeof(*lo_que_quiero_escribir));

}

void* leer_en_disco(void* desde_donde_leo, int cant_de_bytes_a_leer){

	void* retorno = malloc(cant_de_bytes_a_leer);

	memcpy(retorno,desde_donde_leo,cant_de_bytes_a_leer);

	return retorno;
}

void* abrir_en_disco(int numero_de_bloque){
	return encontrar_posicion_en_disco(numero_de_bloque);
}

void liberar_bloque(char* path){

//TODO


}
