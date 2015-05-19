/*
 * mock for camera
 *
 */
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include "configurations.h"
int camera_init(sParameterStruct *sSO2Parameters){
	return 0;
}

int camera_abort(sParameterStruct *sSO2Parameters){
	return 0;
}

int camera_stop(sParameterStruct *sSO2Parameters){
	return 0;
}

int camera_trigger( sParameterStruct *sSO2Parameters, void (*callbackFunction)(void *sSO2Parameters) ){
	sleepMs(100);
	callbackFunction(sSO2Parameters);

	return 0;
}


/* HEADER is 64 bytes */
/* image size is  1344 * 1024 * 16/8 */
void *getBufferFromFile(char *filename){
	void *buffer;
	int readCount;
	size_t length;
	FILE *f = fopen (filename, "rb");
	if (f)
	{
		(void)fseek (f, 0, SEEK_END);
		length = (size_t)ftell (f);
		(void)fseek (f, 0, SEEK_SET);
		buffer = malloc (length);
		if (buffer) {
			readCount = fread (buffer, length, length, f);
		} else {
			printf("failed to read into buffer\n");
		}
		(void)fclose (f);
	} else {
		printf("failed to read file\n");
	}

	return buffer;
}

int camera_get( sParameterStruct *sSO2Parameters, short **stBuffer )
{
	char *filename;
	if(sSO2Parameters->identifier == 'a')
		filename = "src/camera/mock/fixtures/top.raw";
	else
		filename = "src/camera/mock/fixtures/bot.raw";

	*stBuffer = getBufferFromFile(filename);
	return 0;
}

int camera_setExposure(sParameterStruct *sSO2Parameters, stImageBuff stBuffer ){
	return 0;
}

int camera_setExposureSwitch(sParameterStruct *sSO2Parameters, int timeSwitch){
	return 0;
}

int camera_config(sParameterStruct *sSO2Parameters)
{
	return 0;
}


