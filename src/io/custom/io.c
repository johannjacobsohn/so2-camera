#include<stdio.h>
#include<time.h>
#include<string.h>
#include "common.h"
#include "configurations.h"
#include "../io.h"

/*prototypes*/
static int createFilename(sParameterStruct *sSO2Parameters, char *filename, char *filetype);

/* io_init
 *
 */
#pragma GCC diagnostic ignored "-Wunused-parameter"
int io_init(sParameterStruct * sSO2Parameters){
	log_message("io_init");
	return 0;
}
#pragma GCC diagnostic ignored "-Wunused-parameter"

/*
 *
 */
#pragma GCC diagnostic ignored "-Wunused-parameter"
int io_writeImage(sParameterStruct * sSO2Parameters){
	log_message("io_writeImage");
	return 0;
}
#pragma GCC diagnostic ignored "-Wunused-parameter"

//~
//~ sSO2Parameters->cFileNamePrefix in config
//~ sSO2Parameters->cImagePath in config
int createFilename(sParameterStruct *sSO2Parameters, char *filename, char *filetype)
{
	int status;
	char id = sSO2Parameters->identifier;
	timeStruct *time = sSO2Parameters->timestampBefore;	// Datum und Uhrzeit

	/* identify Camera for filename Prefix */
	char * camname = id == 'A' ? "top" : "bot";

	/* write header string with information from system time for camera B. */
	status =
	    sprintf(filename,
		    "%s%s_%04d_%02d_%02d-%02d_%02d_%02d_%03d_cam_%s.%s",
		    sSO2Parameters->cImagePath, sSO2Parameters->cFileNamePrefix,
		    time->year, time->mon, time->day, time->hour, time->min,
		    time->sec, time->milli, camname, filetype);

	return status > 0 ? 0 : 1;
}

int io_writeDump(sParameterStruct * sSO2Parameters)
{
	FILE * imageFile;
	FILE * fp;
	char headerfile[100];
	char rawfile[100];
	int fwriteReturn;
	int status = 0;

	/* generate filenames */
	status = createFilename(sSO2Parameters, headerfile, "txt");
	if(status){
		log_error("could not create txt filename");
	}
	status = createFilename(sSO2Parameters, rawfile, "raw");
	if(status){
		log_error("could not create txt filename");
	}

	/* Open a new file for the image (writeable, binary) */
	imageFile = fopen(rawfile, "wb");
	if(imageFile != NULL){
		fwriteReturn = fwrite(sSO2Parameters->stBuffer, 1, sSO2Parameters->dBufferlength * 2, imageFile);
		if(fwriteReturn != sSO2Parameters->dBufferlength * 2){
			log_error("could not write raw file");
		}
		fclose(imageFile);
	} else {
		log_error("not opened raw file");
	}
	/* write a text file containing header information */
	fp = fopen(headerfile, "ab");
	if (fp != NULL)
	{
		fprintf(fp, "dBufferlength %i\n", sSO2Parameters->dBufferlength);
		fprintf(fp, "dHistMinInterval %i\n", sSO2Parameters->dHistMinInterval);
		fprintf(fp, "dHistPercentage %i\n", sSO2Parameters->dHistPercentage);
		fprintf(fp, "dDarkCurrent %i\n", (int)sSO2Parameters->dDarkCurrent);
		fprintf(fp, "dImageCounter %i\n", (int)sSO2Parameters->dImageCounter);
		fprintf(fp, "dInterFrameDelay %i\n", (int)sSO2Parameters->dInterFrameDelay);
		fprintf(fp, "dTriggerPulseWidth %i\n", (int)sSO2Parameters->dTriggerPulseWidth);
		fprintf(fp, "dExposureTime %f\n", sSO2Parameters->dExposureTime);
		fprintf(fp, "cConfigFileName %s\n", sSO2Parameters->cConfigFileName);
		fprintf(fp, "cFileNamePrefix %s\n", sSO2Parameters->cFileNamePrefix);
		fprintf(fp, "cImagePath %s\n", sSO2Parameters->cImagePath);
		fprintf(fp, "dFixTime %i\n", sSO2Parameters->dFixTime);
		fprintf(fp, "dfilesize %i\n", sSO2Parameters->dfilesize);
		fprintf(fp, "dImagesFile %i\n", sSO2Parameters->dImagesFile);
		// @TODO: add time

		fclose(fp);
	} else {
		log_error("not opened text file");
	}



	log_message("dumb image written");


	return 0;
}

/*
 *
 */
int io_uninit(sParameterStruct * sSO2Parameters){
	log_message("io_uninit");
	return 0;
}
