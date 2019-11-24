/*
 * funciones_aux.c
 *
 *  Created on: 20 nov. 2019
 *      Author: utnso
 */

//FUNCIONES AUXILIARES

#include "funciones_aux.h"

void* serializar_flag(int flag_resultado){
	void* paquete=malloc(sizeof(int));
	memcpy(paquete,&flag_resultado,sizeof(int));
	return paquete;

}

void* encontrar_posicion_en_disco(int numero_de_bloque){

	return (primer_bloque+ numero_de_bloque* BLOCK_SIZE);

}

void agregar_datos_de_escritura_a_tabla_de_nodo(uint32_t direccion,t_list* bloque_a_escribir){

	int i=0;


	while(i<1024){
	if(tabla_de_nodos[i]->blk_indirect==direccion){

		void agregar_datos(int bloque){
		agregar_bloque_dato_a_bloque_puntero(tabla_de_nodos[i]->blk_indirect,bloque);
		}
		list_iterate(bloque_a_escribir,agregar_datos);
	}
	i++;
	}
}

void agregar_bloque_dato_a_bloque_puntero(ptrGBloque vector_de_direcciones[] ,int bloque){


	for(int i=0;i<1000;i++){
		if(!esta_full_el_bloque_puntero(vector_de_direcciones[i])){
			void* direccion_escritura=obtener_direccion_de_escritura(vector_de_direcciones[i]);
			void* direccion_bloque=encontrar_posicion_en_disco(bloque);
			agregar_direccion_del_bloque_de_dato(direccion_escritura,direccion_bloque);
		}
	}
}

void agregar_direccion_del_bloque_de_dato(void* direccion_escritura,void *direccion_bloque){

	memcpy(direccion_escritura,direccion_bloque,sizeof(ptrGBloque));
}

bool esta_full_el_bloque_puntero(ptrGBloque una_direccion){
	int i=0;
	void*direccion_dentro;
	while(i<1024 && (int)direccion_dentro!=0){
	memcpy(direccion_dentro,(void*)una_direccion+i*sizeof(una_direccion),sizeof(una_direccion));
	}
	if(direccion_dentro==0){

		return false;
	}
	return true;
}

void* obtener_direccion_de_escritura(ptrGBloque direcciones){
	void* direccion;
	void* punto_de_lectura= direcciones;
	int offset=0;
	while(((int)direccion)!=0){
	memcpy(direccion,punto_de_lectura+offset,sizeof(direcciones));
	offset+=sizeof(direcciones);
	}
	return (direccion-sizeof(direcciones));
}

bool verificar_path_este_permitido(char*path){

	//puede mejorar TODO
	return string_contains(path, PUNTO_DE_MONTAJE);
}


int buscar_bloque_libre(int tipo_de_busqueda){

	int nro_de_nodo=-1;
	int cantidad_de_bloques;
	int i=0;
	if(tipo_de_busqueda==BUSQUEDA_ARCHIVO){
		cantidad_de_bloques=bitarray_get_max_bit(bitarray);
		i=cantidad_de_bloques_reservados;
	}else{
		cantidad_de_bloques=cantidad_de_bloques_reservados;
		i=1+(tamanio_disco/BLOCK_SIZE/8)/BLOCK_SIZE;

	}
	while ((i<cantidad_de_bloques && nro_de_nodo<0)){
		if(! bitarray_test_bit(bitarray,i))
			nro_de_nodo=i;

		i++;


	}

	return nro_de_nodo;

}

GFile* buscar_nodo_libre(){

	for(int i=0;i<1024;i++){
		if(tabla_de_nodos[i]==NULL){

			GFile* nodo_libre;
			nodo_libre=(GFile*)malloc(sizeof(GFile));
			tabla_de_nodos[i]=nodo_libre;
			return nodo_libre;
		}
		if(tabla_de_nodos[i]->state==BORRADO){
			return tabla_de_nodos[i];
		}


	}
	return NULL;
}

void crear_vector_de_punteros(ptrGBloque array_de_bloques[], int n){

	for(int i=0;i<n;i++){
		array_de_bloques[i]=0;
	}
}



ptrGBloque* obtener_puntero_padre(char* path){


	char** vector= string_split(path, "/");

	int i =0;
	while(vector[i]!=NULL){
		i++;
	}

	char* nombre_nodo_padre=vector[i-2];

	GFile* nodo_padre =encontrar_en_tabla_de_nodos(nombre_nodo_padre);
	return nodo_padre->blk_indirect;


}


void obtener_nombre_de_archivo(char fname[],char* path){

	char** vector= string_split(path, "/");
	int i =0;
	while(vector[i]!=NULL){
		i++;
	}
	for(int j=0;j<71;j++){
		fname[j]=vector[i][j];
	}


}


void leer_cada_archivo_y_borrar(char* path){
	char fname[71];
	obtener_nombre_de_archivo(fname, path);
	GFile* nodo_dir=encontrar_en_tabla_de_nodos(fname);
	for(int i=0;i<1024;i++){
		tabla_de_nodos[i];
		if(tabla_de_nodos[i]->parent_dir_block==nodo_dir->blk_indirect){
			borrar_del_bitmap(tabla_de_nodos[i]->blk_indirect);
			tabla_de_nodos[i]->state=BORRADO;
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

	void paquetizar(GFile* un_nodo){

		memcpy(retorno+offset ,&un_nodo->state,1);//TODO esto puede romper
		offset+=1;
		memcpy(retorno+offset,&un_nodo->c_date,sizeof(long int));
		offset+=sizeof(long int);
		memcpy(retorno+offset,&un_nodo->m_date,sizeof(long int));
		offset+=sizeof(long int);
		memcpy(retorno+offset,&un_nodo->parent_dir_block,sizeof(int));
		offset+=sizeof(int);
		memcpy(retorno+offset,&un_nodo->file_size,sizeof(int));
		offset+=sizeof(int);
		memcpy(retorno+offset,un_nodo->fname,72);
		offset+=72;

	}
	list_iterate(lista, paquetizar);

	return retorno;
}


void borrar_del_bitmap(uint32_t blk_indirect[]){

	for(int i=0;i<1000;i++){
		void* bloque_punt_indirecto=leer_en_disco((void*)blk_indirect[i],1024*sizeof(uint32_t));
		int offset=0;
		for(int j=0;j<1024;j++){
			uint32_t direccion_a_borrar;
			memcpy(&direccion_a_borrar,bloque_punt_indirecto+offset,sizeof(uint32_t));
			offset+=sizeof(uint32_t);
			if(direccion_a_borrar){
				int numero_de_bloque=((int)primer_bloque - direccion_a_borrar)/BLOCK_SIZE;
				bitarray_clean_bit(bitarray,numero_de_bloque);
				//TODO ESTO PUEDE MALIR SAL
			}
		}
		if(blk_indirect[i])
			bitarray_clean_bit(bitarray,primer_bloque-blk_indirect[i]/BLOCK_SIZE);
	}
}


GFile* encontrar_en_tabla_de_nodos(char* nombre_de_nodo){

	GFile* buscar_nodo_por_nombre(char* nombre){

		for(int i=0;i<1024;i++){

			if(!strcmp(tabla_de_nodos[i]->fname,nombre))
				return tabla_de_nodos[i];

		}

		return NULL;
	}
	sem_wait(mutex_tabla_de_nodos);
	GFile* el_nodo_a_retornar = buscar_nodo_por_nombre(nombre_de_nodo);
	sem_post(mutex_tabla_de_nodos);
	return el_nodo_a_retornar;
}

void* preparar_nodo_para_grabar(GFile* nodo_libre){

	void* paquete= malloc(sizeof(GFile));
	int offset=0;
	memcpy(paquete+offset,&nodo_libre->state,sizeof(nodo_libre->state));
	offset+=sizeof(nodo_libre->state);

	memcpy(paquete+offset,&nodo_libre->fname,strlen(nodo_libre->fname));
	offset+=strlen(nodo_libre->fname);

	memcpy(paquete+offset,&nodo_libre->c_date,sizeof(nodo_libre->c_date));
	offset+=sizeof(nodo_libre->c_date);

	memcpy(paquete+offset,&nodo_libre->m_date,sizeof(nodo_libre->m_date));
	offset+=sizeof(nodo_libre->m_date);

	memcpy(paquete+offset,&nodo_libre->file_size,sizeof(nodo_libre->file_size));
	offset+=sizeof(nodo_libre->file_size);

	memcpy(paquete+offset,&nodo_libre->parent_dir_block,sizeof(nodo_libre->parent_dir_block));
	offset+=sizeof(nodo_libre->parent_dir_block);

	memcpy(paquete+offset,&nodo_libre->blk_indirect,sizeof(nodo_libre->blk_indirect));
	offset+=sizeof(nodo_libre->blk_indirect);//TODO REVISAR SI FUNCA SINO HAY QUE HACER UN FOR

	return paquete;
}

void escribir_en_disco(void* lo_que_quiero_escribir,int un_bloque){


	void* posicion = encontrar_posicion_en_disco(un_bloque);
	memcpy(posicion,lo_que_quiero_escribir,sizeof(*lo_que_quiero_escribir));
	free(lo_que_quiero_escribir);

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
