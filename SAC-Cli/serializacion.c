/*
 * serializacion.c
 *
 *  Created on: 30 oct. 2019
 *      Author: utnso
 */
#include "serializacion.h"
#include <errno.h>
#include <fuse.h>
#include "globales.h"
#include <commons/collections/list.h>
#include <commons/string.h>

int serializar_fs_rmdir(char* path){
	printf("llego un rmdir\n");
	void* paquete = serializar_paquete_para_eliminar_directorio(path);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	return 0;


}

int serializar_fs_opendir(const char *path, struct fuse_file_info *fi){

	if(!string_contains(path,"SAC")){

		return ENONET;


	}else{

		printf("llego un opendir\n");
		void* paquete = serializar_paquete_para_abrir_directorio(path);
		void* resultado = enviar_paquete(paquete);
		free(paquete);
		int retorno;
		memcpy(&retorno,resultado,sizeof(int));
		return retorno;
	}

}

int serializar_fs_create(const char *path, mode_t mode , struct fuse_file_info * fi){
	printf("llego un create\n");
	void* paquete = serializar_paquete_para_crear_archivo(path,mode);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;

}
int serializar_fs_mknod(char* name,mode_t mode,dev_t dev){
	printf("llego un mknod\n");
	void* paquete = serializar_paquete_fs_mknod(name,mode,dev);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;

}

int serializar_fs_statfs(const char* path, struct statvfs* stats){
	printf("llego un statfs\n");
	void* paquete = serializar_paquete_fs_statfs(path, stats);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;

	//The 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
	int offset=0;
	for(int i=0;i<6;i++){
		memcpy(&stats->__f_spare[i],resultado+offset,sizeof(int));
		offset+=sizeof(int);
	}

	memcpy(&stats->__f_unused,resultado+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(&stats->f_bavail,resultado+offset,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(&stats->f_bfree,resultado+offset,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(&stats->f_blocks,resultado+offset,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(&stats->f_bsize,resultado+offset,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(&stats->f_ffree,resultado+offset,sizeof(__fsfilcnt64_t));
	offset+=sizeof(__fsfilcnt64_t);
	memcpy(&stats->f_files,resultado+offset,sizeof(__fsfilcnt64_t));
	offset+=sizeof(__fsfilcnt64_t);
	memcpy(&stats->f_frsize,resultado+offset,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(&stats->f_namemax,resultado+offset,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(&stats->__f_unused,resultado+offset,sizeof(int));
	offset+=sizeof(int);


	free(resultado);
	return 0;
}

int serializar_fs_truncate(const char *path, off_t offset, struct fuse_file_info *fi){

	printf("llego un truncate\n");
	void* paquete = serializar_paquete_fs_truncate(path,offset);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;

}


int serializar_fs_access(const char *path, int flags){

	printf("llego un accsess\n");
	void* paquete = serializar_paquete_fs_accses(path,flags);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;
}

int serializar_fs_rename(char* old_path,char* new_path,int flags){

	printf("llego un rename\n");
	void* paquete = serializar_paquete_fs_rename(old_path,new_path,flags);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;
}

int serializar_fs_getattr(const char *path, struct stat *unos_stats,struct fuse_file_info *fi){

	printf("llego un getattr\n");

	if(!string_contains(path,"SAC")){

		//stat(path,unos_stats);
		return 0;


	}else{


		void* paquete = serializar_paquete_para_obtener_atributos(path);
		void* resultado = enviar_paquete(paquete);
		free(paquete);

		int offset=0;
		memcpy(&unos_stats->st_atim.tv_nsec,resultado+offset,sizeof(unos_stats->st_atim));
		offset+=sizeof(unos_stats->st_atim);
		memcpy(&unos_stats->st_atim.tv_sec,resultado+offset,sizeof(unos_stats->st_atim));
		offset+=sizeof(unos_stats->st_atim);
		memcpy(&unos_stats->st_gid,resultado+offset,sizeof(unos_stats->st_gid));
		offset+=sizeof(unos_stats->st_gid);
		memcpy(&unos_stats->st_mode,resultado+offset,sizeof(unos_stats->st_mode));
		offset+=sizeof(unos_stats->st_mode);
		memcpy(&unos_stats->st_mtim.tv_nsec,resultado+offset,sizeof(unos_stats->st_mtim));
		offset+=sizeof(unos_stats->st_mtim);
		memcpy(&unos_stats->st_mtim.tv_sec,resultado+offset,sizeof(unos_stats->st_mtim));
		offset+=sizeof(unos_stats->st_mtim);
		memcpy(&unos_stats->st_nlink,resultado+offset,sizeof(unos_stats->st_nlink));
		offset+=sizeof(unos_stats->st_nlink);
		memcpy(&unos_stats->st_size,resultado+offset,sizeof(unos_stats->st_size));
		offset+=sizeof(unos_stats->st_size);
		memcpy(&unos_stats->st_uid,resultado+offset,sizeof(unos_stats->st_uid));

		return 0;

	}




}


int serializar_fs_readdir(const char *path, void *buffer, fuse_fill_dir_t cosa, off_t offset, struct fuse_file_info *fi , int flags){
	void* paquete = serializar_paquete_para_leer_directorio(path, buffer,offset);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));

	return retorno;
}

int serializar_fs_mkdir(const char *path, mode_t mode){
	void* paquete = serializar_paquete_para_crear_directorio(path,mode);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;

}


int serializar_fs_open(const char *pathname, struct fuse_file_info *fi){


	printf("llego un open \n");
	if(!string_contains(pathname,"SAC")){

		return 0;


	}else{

		void* paquete = serializar_paquete_para_abrir_archivo(pathname);
		void* resultado = enviar_paquete(paquete);
		free(paquete);
		int retorno;
		memcpy(&retorno,resultado,sizeof(int));
		free(resultado);
		if(retorno!=-1){
			return ENOSYS ;
		}else{return -1;}
	}

}
int serializar_fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	void* paquete = serializar_paquete_para_leer_archivo( path, buf, size, offset);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno=0;
	memcpy(buf,resultado,size);
	free(resultado);
	retorno=strlen(buf);
	return retorno;


}
int serializar_fs_write(const char * path1, const char *path2, size_t size, off_t offset, struct fuse_file_info *fi){
	// TODO esto quizas esta medio mal, el prototipo de FUSE me dice algo y la syscall otra cosa
	void* paquete = serializar_paquete_para_escribir_archivo( path1, path2,  size, offset);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;

}

void* serializar_paquete_para_crear_archivo(const char *path, mode_t mode){
	int peso=0;
	int peso_path=string_length(path)+1;
	int peso_mode= sizeof(mode);
	peso=peso_path+peso_mode + 2* sizeof(int);
	int offset=0;
	int codigo_de_operacion= CREAR_ARCHIVO;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,path,peso_path);
	offset+=peso_path;
	memcpy(paquete+offset,&mode,sizeof(mode_t));

	return paquete;
}


void* serializar_paquete_fs_statfs(const char *path, struct statvfs* stats){

	int peso=0;
	int peso_path=string_length(path)+1;
	int peso_stats= sizeof(struct statvfs);
	peso=peso_path+peso_stats+ 2* sizeof(int);
	int offset=0;
	int codigo_de_operacion= STATFS;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,path,peso_path);
	offset+=peso_path;
	memcpy(paquete+offset,&stats->__f_unused,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&stats->f_bavail,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(paquete+offset,&stats->f_bfree,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(paquete+offset,&stats->f_blocks,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(paquete+offset,&stats->f_bsize,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(paquete+offset,&stats->f_ffree,sizeof(__fsfilcnt64_t));
	offset+=sizeof(__fsfilcnt64_t);
	memcpy(paquete+offset,&stats->f_files,sizeof(__fsfilcnt64_t));
	offset+=sizeof(__fsfilcnt64_t);
	memcpy(paquete+offset,&stats->f_frsize,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(paquete+offset,&stats->f_namemax,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(paquete+offset,&stats->__f_unused,sizeof(int));
	offset+=sizeof(int);


	return paquete;





}

void* serializar_paquete_fs_mknod(const char* name,mode_t mode,dev_t dev){
	int peso=0;
	int peso_path=string_length(name)+1;
	int peso_mode= sizeof(mode);
	int peso_dev=sizeof(dev);
	peso=peso_path+peso_mode+peso_dev + 2* sizeof(int);
	int offset=0;
	int codigo_de_operacion= MKNOD;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,name,peso_path);
	offset+=peso_path;
	memcpy(paquete+offset,&mode,sizeof(mode_t));
	offset+=sizeof(mode_t);
	memcpy(paquete+offset,&dev,sizeof(dev_t));


	return paquete;

}

void* serializar_paquete_fs_truncate(const char* path,off_t offset_dato){


	int peso_path=strlen(path)+1;
	void* paquete=malloc(sizeof(int)*3+peso_path);
	int offset=0;
	int codigo_de_operacion=TRUNCATE;

	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&offset_dato,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,path,peso_path);



	return paquete;


}

void*serializar_paquete_fs_accses(const char* path,int flags){
	int peso_path=strlen(path)+1;
	void* paquete=malloc(sizeof(int)*3+peso_path);
	int offset=0;
	int codigo_de_operacion=ACCESS;

	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&flags,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,path,peso_path);



	return paquete;


}
void* serializar_paquete_fs_rename(const char *old_path, const char *new_path, unsigned int flags){

	int peso_old_path=strlen(old_path);
	int peso_new_path=strlen(new_path);
	void* paquete=malloc(sizeof(int)*3+peso_old_path+peso_new_path);
	int offset=0;
	int codigo_de_operacion=RENAME;

	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&flags,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_old_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,old_path,peso_old_path);
	offset+=peso_old_path;
	memcpy(paquete+offset,&peso_new_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,new_path,peso_new_path);
	offset+=peso_new_path;



	return paquete;

}


void* serializar_paquete_para_eliminar_directorio(char* path){

	int peso_path = string_length(path)+1;
	int peso=peso_path+ 2* sizeof(int);
	int desplazamiento=0;
	int codigo_de_operacion = ELIMINAR_DIRECTORIO;
	void*paquete = malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&codigo_de_operacion,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&peso_path,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,path,peso_path);

	return paquete;



}
void* serializar_paquete_para_leer_directorio(const char *path, void *buffer,off_t offset){

	char* nombres_de_archivos=leer_nombres_de_archivos_y_directorios(buffer);
	int peso = 0;
	int peso_path = string_length(path)+1;
	int peso_buffer = string_length(nombres_de_archivos)+1;
	peso=peso_path+peso_buffer + 3* sizeof(int);
	int desplazamiento=0;
	int codigo_de_operacion = LISTAR_DIRECTORIO_Y_ARCHIVOS;
	void*paquete = malloc(peso+sizeof(int));


	memcpy(paquete,&peso,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&codigo_de_operacion,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&offset,sizeof(off_t));
	desplazamiento+=sizeof(off_t);
	memcpy(paquete+desplazamiento,&peso_path,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,path,peso_path);
	desplazamiento+=peso_path;
	memcpy(paquete+desplazamiento,&peso_buffer,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,nombres_de_archivos,peso_path);

	return paquete;
}

void* serializar_paquete_para_crear_directorio(const char* path,mode_t mode){

	int peso=0;
	int peso_path=string_length(path)+1;
	int peso_mode= sizeof(mode);
	peso=peso_path+peso_mode + 2* sizeof(int);
	int offset=0;
	int codigo_de_operacion= CREAR_DIRECTORIO;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,path,peso_path);
	offset+=peso_path;
	memcpy(paquete+offset,&mode,sizeof(mode_t));

	return paquete;
}


void* serializar_paquete_para_leer_archivo(const char *path, char *buf, size_t size, off_t desplazamiento){
	int peso=0;
	int peso_path= strlen(path)+1;
	int peso_buf=strlen(buf)+1;
	peso=peso_path+peso_buf + 3* sizeof(int)+sizeof(size_t)+sizeof(off_t);
	int offset=0;
	int codigo_de_operacion= LEER_ARCHIVO;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete+offset,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);

	memcpy(paquete+offset,&size,sizeof(size_t));
	offset+=sizeof(size_t);

	memcpy(paquete+offset,&desplazamiento,sizeof(off_t));
	offset+=sizeof(off_t);

	memcpy(paquete+offset,&peso_path,sizeof(int));
	offset+=sizeof(int);

	memcpy(paquete+offset,path,peso_path);
	offset+=peso_path;

	memcpy(paquete+offset,&peso_buf,sizeof(int));
	offset+=sizeof(int);

	memcpy(paquete+offset,buf,peso_buf);
	offset+=peso_buf;

	return paquete;

}
void* serializar_paquete_para_escribir_archivo(const char * path1, const char *path2, size_t size, off_t offset){

	int peso=0;
	int peso_path1=strlen(path1)+1;//hay que checkear esto TODO
	int peso_path2= strlen(path2)+1;
	int peso_offset= sizeof(off_t);
	int peso_size = sizeof(size_t);
	peso=peso_path1+peso_path2 +peso_offset+peso_size+ sizeof(int);
	int desplazamiento=0;
	int codigo_de_operacion= ESCRIBIR_ARCHIVO;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&peso_path1,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,path1,sizeof(int));
	desplazamiento+=peso_path1;
	memcpy(paquete+desplazamiento,&peso_path2,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,path2,sizeof(int));
	desplazamiento+=peso_path2;
	memcpy(paquete+desplazamiento,&size,sizeof(size_t));
	desplazamiento+=sizeof(size_t);
	memcpy(paquete+desplazamiento,&offset,sizeof(off_t));
	return paquete;





}
void* serializar_paquete_para_abrir_archivo(const char *pathname){
	int peso=0;
	int peso_path=string_length(pathname)+1;
	peso=+peso_path + 2* sizeof(int);
	int offset=0;
	int codigo_de_operacion= ABRIR_ARCHIVO;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,pathname,peso_path);
	offset+=peso_path;
	return paquete;






}

void* serializar_paquete_para_obtener_atributos(const char*path){

	int peso_path = string_length(path)+1;
	int peso=peso_path+ 2* sizeof(int);
	int desplazamiento=0;
	int codigo_de_operacion = GET_ATTRIBUTES;
	void*paquete = malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&codigo_de_operacion,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&peso_path,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,path,peso_path);

	return paquete;



}

void* serializar_paquete_para_abrir_directorio(const char* path){
	int peso_path = string_length(path)+1;
	int peso=peso_path+ 2* sizeof(int);
	int desplazamiento=0;
	int codigo_de_operacion = ABRIR_DIRECTORIO;
	void*paquete = malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&codigo_de_operacion,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&peso_path,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,path,peso_path);

	return paquete;



}


//RECIBIR Y ENVIAR A SAC_SERVER
void* enviar_paquete(void*paquete){

	int peso;
	memcpy(&peso,paquete,sizeof(int));
	send(socket_sac_server,paquete,peso, 0);
	int alocador;
	return recibir_resultado(&alocador);

}


void* recibir_resultado(int* alocador){

	void* buffer;

	if(recv(socket_sac_server, alocador, 4, MSG_WAITALL)!=0){
		buffer = malloc(*alocador);
		recv(socket_sac_server, buffer, *alocador, MSG_WAITALL);
		return buffer;
	}else{
		*alocador = 0;
		return buffer;
	}
}

//Funciones Auxiliares

char* leer_nombres_de_archivos_y_directorios(void* buffer){

	return (char*) buffer;

}
