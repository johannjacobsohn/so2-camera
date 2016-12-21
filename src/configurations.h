#ifndef _CONFIGURATIONS_
#define _CONFIGURATIONS_

/**********************************************************
 *
 * Header-File with miscellaneous configuration functions
 * for Program, Camera and Framegrabber
 *
 **********************************************************/

/******************************
 *   HEADER INCLUDES
 ******************************/

#include<stdio.h>
#include<math.h>
#include "timehelpers.h"

/******************************
 *   MACROS
 ******************************/

#define MAX_STRING_LENGTH 256

/******************************
 *   Structures
 ******************************/

/* config structure */
typedef struct {
	/* flag. If trueish, processing is done */
	int processing;

	/* flag. If falseish, debugging output is suppressed */
	int debug;

	/* Number of images that are to be taken. Useful for testing */
	int noofimages;

	/* Number of pixels in 1 Image */
	int dBufferlength;

	/* exposure time in [us] */
	double dExposureTime_a;
	double dExposureTime_b;

	/* number of images taken */
	int dImageCounter;

	/* delay between two frames in [ms] */
	int dInterFrameDelay;

	/* contains the config filename */
	char * cConfigFileName;

	/* contains a prefix for all images */
	char * cFileNamePrefix;

	/* path to image directory */
	char * cImagePath;

	/*
	 * A switch to set the exposuretime fix to the value given in
	 * the config file
	 */
	int dFixTime;

	/*
	 * serial device used to talk to the filterwheel firmware
	 * eg. /dev/ttyUSB0
	 */
	char * filterwheel_device;

	/* number of images between */
	int darkframeintervall;

	/* serial device to talk to for the spectrometer shutter */
	const char * spectrometer_shutter_device;

	/*
	 * serial device channel on which the servo is connected (only used
	 * by the pololu maestro servo controller)
	 */
	unsigned char spectrometer_shutter_channel;

	/* Resolution in x direction */
	int webcam_xRes;

	/* Resolution in y direction */
	int webcam_yRes;

	/* port on which a tcp connection can be made */
	int comm_port;

} sConfigStruct;


/*  */
typedef struct {
	double * wavelengths;

	double * lastSpectrum;

	double * electronic_offset;

	double * dark_current;

	double max;

	int integration_time_micros;

	int spectrum_length;

	timeStruct *timestampBefore;

	timeStruct *timestampAfter;
} sSpectrometerStruct;

/* Camera parameters */
typedef struct {
	/* not used right now. should contain a value for image noise */
	unsigned long dDarkCurrent;

	/* A handle to identify the camera */
	unsigned long hCamera;

	/* Timestamp taken *before* the image has been triggered */
	timeStruct *timestampBefore;

	/* Timestamp taken after the image buffer has been returned */
	timeStruct *timestampAfter;

	/* length of the triggerpulse in [ms] */
	unsigned long dTriggerPulseWidth;

	/* contains the Exposuretime in [ms] */
	double dExposureTime;

	/* Pointer to image buffer */
	short * stBuffer;

	/* ~Callback stuff~ */

	/* Event Flags */
	volatile unsigned long fBufferReady;

	/* Control Flags */
	volatile unsigned long fFifoOverFlow;

	/* Camera identifier */
	char identifier;

	/* flag to indicate that the current image is a dark image */
	int dark;

} sParameterStruct;


/* Webcam parameters */
typedef struct {
	/* Pointer to image buffer x*y*24 RAW */
	char *buffer;

	/* Size of Buffer in bytes */
	int bufferSize;

	/* Timestamp taken *before* the image has been triggered */
	timeStruct *timestampBefore;

	/* Timestamp taken *after* the image has been received */
	timeStruct *timestampAfter;
} sWebCamStruct;


 /******************************
 *   FUNCTIONS
 ******************************/
int config_process_cli_arguments(int argc, char *argv[], sConfigStruct *config);
int config_load_configfile(sConfigStruct *config);
void config_load_default(sConfigStruct *config);
void config_init_sParameterStruct(sParameterStruct *sSO2Parameters, sConfigStruct *config, char identifier);
void config_init_sConfigStruct(sConfigStruct *config);
#endif
