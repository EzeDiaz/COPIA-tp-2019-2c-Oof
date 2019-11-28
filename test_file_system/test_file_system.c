/*
 ============================================================================
 Name        : test_file_system.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <SAC/SAC-Cli.h>

int main(int argc,char**argv) {

	run_sac(argc,argv);
	creat("/pepito.bin",S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	mkdir("/memingo",S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	FILE* sasa=fopen("/pepito.bin","w");
}
