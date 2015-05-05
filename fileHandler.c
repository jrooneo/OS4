#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileHandler.h"

FILE *fileHandlerRead(char *filename){
	FILE *fp;
	fp = fopen(filename, "r");
	if(fp == NULL){
		perror("Could not open file");
	}
	return fp;
}

FILE *fileHandlerWrite(char *filename){
	FILE *fp;
	fp = fopen(filename, "a");
	if(fp == NULL){
		perror("Could not open file");
	}
	return fp;
}
