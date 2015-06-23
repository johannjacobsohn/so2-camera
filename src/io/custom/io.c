/*

 * This implements the file output system, responsible for writing
 * the gathered data to disk and to process the data into reasonable
 * file formats. This could also do i.e. packaging files into a .tar
 * file.
 *
 * Currently, there are two write modes:
 * - dumb mode, the camera data and all relevent headers are dumped to
 *   files
 * - png mode, the camera data is converted to pngs, and relevant
 *   headers are written to ancillary text chunks
 *
 * PNG mode is a lot slower, but produces smaller files which can be
 * easily viewed and used for further processing.
 * Dumb mode requires less processing and does not alter the original
 * data, but requires additional work for viewing and evalution.
 *
 */
#include<stdio.h>
#include<time.h>
#include<string.h>
#include<opencv/cv.h>
#include<opencv/highgui.h>
#include "common.h"
#include "configurations.h"
#include "../io.h"
#include "make_png_header.c"

/* local prototypes */
int createFilename(sParameterStruct *sSO2Parameters, char *filename, char *filetype);
IplImage *bufferToImage(short *buffer);
int dateStructToISO8601(timeStruct * time, char * iso_date);
int insertValue(char * png, char * name, float value, int png_length);
int insertHeader(char * png, char * name, char * content, int png_length);
int insertHeaders(char * png, sParameterStruct * sSO2Parameters, int png_length);
int io_writeImage(sParameterStruct * sSO2Parameters);
int io_writeDump(sParameterStruct * sSO2Parameters);

/* local variables */
static int processing = 0;

/*
 * `io_init`
 * Initialize the IO functionality. Currently, this does nothing, but this could be the place to
 * set up folders and start tar files.
 */
int io_init(sConfigStruct * config)
{
	if(config->processing){
		processing = config->processing;
	}
	log_message("io_init");
	return 0;
}

/*
 * `io_write`
 * Writes a single images stream to disk. Delegates the real work to
 * `io_writeImage` or `io_writeDump`, depending on the value of processing
 * which was set in `io_init`.
 */
int io_write(sParameterStruct * sSO2Parameters)
{
	/*
	 * processing:
	 *   1 = dumb
	 *   2 = png
	 *   3 = both
	 * else = both
	 *
	 * e.g.
	 *  !1 &&  2 = 2
	 *  !2 &&  1 = 1
	 *  !1 %% !2 = 3
	 */
	if(processing != 1){
		io_writeImage(sSO2Parameters);
	}
	if(processing != 2){
		io_writeDump(sSO2Parameters);
	}
	// FIXME: return correct status
	return 0;
}

/*
 * io_uninit
 * uninitialize/stops the IO functionality. Currently, this does nothing,
 * but could be the place to finish tar archives, calculate checksums,
 * return file handlers and do general clean up.
 */
#pragma GCC diagnostic ignored "-Wunused-parameter"
int io_uninit(sConfigStruct * config)
{
	log_message("io_uninit");
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"


/*--------------------------------------------------------------------
 * Additional, private functions that are used within this file
 * (compilation unit) but are not available elsewhere.
 */


int io_writeDump(sParameterStruct * sSO2Parameters)
{
	FILE * imageFile;
	FILE * fp;
	char headerfile[100];
	char rawfile[100];
	int fwriteReturn;
	int status = 0;
	char iso_date[25];

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
		dateStructToISO8601(sSO2Parameters->timestampBefore, iso_date);
		fprintf(fp, "timestampBefore %s\n", iso_date);

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
int io_writeImage(sParameterStruct * sSO2Parameters){
	FILE * fp;
	short * stBuffer;
	IplImage *img;
	CvMat *png;
	int l_pad;
	int l;
	int ii;
	char filename[100];
	int status;
	char * buffer;

	stBuffer = sSO2Parameters->stBuffer;

	/* generate filenames */
	status = createFilename(sSO2Parameters, filename, "png");
	if(status){
		log_error("could not create txt filename");
	}

	/* convert the image buffer to an openCV image */
	// TODO: check if this has already been done
	img = bufferToImage(stBuffer);

	/*
	 * encode image as png to buffer
	 * playing with the compression is a huge waste of time with no benefit
	 */
	png = cvEncodeImage(".png", img, 0);
	l = png->rows * png->cols;
	l_pad = l;
	cvReleaseImage(&img);

	// pry the actual buffer pointer from png
	buffer = (char *)malloc(l);
	memcpy(buffer, png->data.s, l);
	cvReleaseMat(&png);

	/* add headers */
	l_pad = insertHeaders(buffer, sSO2Parameters, l);

	/* save image to disk*/
	fp = fopen(filename, "wb");
	if (fp) {
		ii = fwrite(buffer, 1, l_pad, fp);
		log_debug("write image l_pad %i, return %i", l_pad, ii);
		// FIXME: check return value
	} else {
		log_error("Something wrong writing to File.");
	}

	// cleanup
	free(buffer);

	log_message("png image written");

	return 0;
}


int insertHeaders(char * png, sParameterStruct * sSO2Parameters, int png_length){
	char iso_date[25];
	dateStructToISO8601(sSO2Parameters->timestampBefore, iso_date);
	png_length = insertHeader(png, "Creation Time ",    iso_date, png_length);
	png_length = insertValue(png, "dBufferlength",      (float)sSO2Parameters->dBufferlength,      png_length);
	png_length = insertValue(png, "dHistMinInterval",   (float)sSO2Parameters->dHistMinInterval,   png_length);
	png_length = insertValue(png, "dHistPercentage",    (float)sSO2Parameters->dHistPercentage,    png_length);
	png_length = insertValue(png, "dDarkCurrent",       (float)sSO2Parameters->dDarkCurrent,       png_length);
	png_length = insertValue(png, "dImageCounter",      (float)sSO2Parameters->dImageCounter,      png_length);
	png_length = insertValue(png, "dInterFrameDelay",   (float)sSO2Parameters->dInterFrameDelay,   png_length);
	png_length = insertValue(png, "dTriggerPulseWidth", (float)sSO2Parameters->dTriggerPulseWidth, png_length);
	png_length = insertValue(png, "dExposureTime",      (float)sSO2Parameters->dExposureTime,      png_length);
	png_length = insertValue(png, "dFixTime",           (float)sSO2Parameters->dFixTime,           png_length);
	png_length = insertValue(png, "dfilesize",          (float)sSO2Parameters->dfilesize,          png_length);
	png_length = insertValue(png, "dImagesFile",        (float)sSO2Parameters->dImagesFile,        png_length);

	return png_length;
}

int insertValue(char * png, char * name, float value, int png_length){
	char text[200];
	sprintf(text, "%s: %f", name, value);
	return insertHeader(png, "Comment ", text , png_length);
}

int insertHeader(char * png, char * name, char * content, int png_length){
	int head[200];
	char text[180]; // can be of arbitrary length, but must be shorter than HEADERLENGTH
	int png_length_padded;
	int name_length = strlen(name);
	int l, i;
	int header_length;
	char * padded_png;

	strcpy(text, name);
	strcat(text, content);

	l = strlen(text);
	header_length = l + 12;
	text[name_length-1] = 0; // FIXME: explain

	make_png_header(text, l, head, header_length);

	png_length_padded = png_length + header_length;

	padded_png = (char *)realloc(png, png_length_padded);
	if(padded_png == NULL){
		log_error("could not realloc!");
		free(padded_png);
	} else {
		png = padded_png;
	}

	// copy end of png %% FIXME: Explain better what is being done here
	for (i = 12; i > 0; i--) { // 4 bytes content length (00 00 00 00), 4 bytes type (IEND), 4 bytes crc (ae 42 60 82)
		png[png_length_padded - i] = png[png_length - i];
	}

	// fill in
	for (i = 0; i < l + 12; i++) {
		png[png_length - 12 + i] = head[i];
	}

	return png_length_padded;
}



/*
 * convert timeStruct to ISO 8601 as in http://www.w3.org/TR/NOTE-datetime
 * which is the standard time format for PNG Creation Time text chunks
 *
 * Time is allways in UTC.
 *
 * This should conform to /\d{4}-[01]\d-[0-3]\dT[0-2]\d:[0-5]\d:[0-5]\d\.\d+([+-][0-2]\d:[0-5]\d|Z)/
 */
int dateStructToISO8601(timeStruct * time, char iso_date[25])
{
	int strl;
	// date is of the form YYYY-MM-DDThh:mm:ss.sssZ (ISO 8601)
	strl = sprintf(iso_date, "%04i-%02i-%02iT%02i:%02i:%02i.%03iZ",
		time->year,
		time->mon,
		time->day,
		time->hour,
		time->min,
		time->sec,
		time->milli
	);

	log_debug("iso_date %s", iso_date);

	return strl > 0 ? 1 : 0;
}


int createFilename(sParameterStruct *sSO2Parameters, char *filename, char *filetype)
{
	int status;
	char id = sSO2Parameters->identifier;
	timeStruct *time = sSO2Parameters->timestampBefore;	// Datum und Uhrzeit

	/* identify Camera for filename Prefix */
	char * camname = id == 'a' ? "top" : "bot";

	/* write header string with information from system time for camera B. */
	status = sprintf(filename,
		"%s%s_%04d_%02d_%02d-%02d_%02d_%02d_%03d_cam_%s.%s",
		sSO2Parameters->cImagePath, sSO2Parameters->cFileNamePrefix,
		time->year, time->mon, time->day, time->hour, time->min,
		time->sec, time->milli, camname, filetype);

	return status > 0 ? 0 : 1;
}
