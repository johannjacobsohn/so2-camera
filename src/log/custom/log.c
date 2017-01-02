#include<stdio.h>
#include<stdarg.h>
#include<time.h>
#include<sys/stat.h>
#include "log.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define LOG_BUFFER_SIZE 512

static sConfigStruct * conf = NULL;
static FILE *logfile = NULL;

int logg(char * type, char *message, va_list args);

int log_init(sConfigStruct * config)
{
	static time_t time_ptr;
	static struct tm t;
	static char nameLogFile[LOG_BUFFER_SIZE];
	struct stat st = {0};
	conf = config;
	time(&time_ptr);
	t = *gmtime(&time_ptr);

	/* check if log folder exists and create if not */
	if (stat("logs/", &st) == -1) {
		mkdir("logs", 0700);
	}

	snprintf(nameLogFile, LOG_BUFFER_SIZE, "logs/log_%04d_%02d_%02d_%02d_%02d.txt",
		t.tm_year + 1900, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min);

	logfile = fopen(nameLogFile, "a");
	if (NULL == logfile) {
		return 1;
	}

	log_message("*******************************************");
	log_message("*                                         *");
	log_message("*       SO2-Camera Control Software       *");
	log_message("*                                         *");
	log_message("*******************************************");
	log_message("*                                         *");
	log_message("* A control program for SO2 measuring     *");
	log_message("* camera based on two Hamamatsu           *");
	log_message("* C8484-16 CCD Cameras.                   *");
	log_message("*                                         *");
	log_message("* written by Johann Jacobson              *");
	log_message("* and Morten Harms                        *");
	log_message("*                                         *");
	log_message("* johann.jacobson(at)zmaw.de              *");
	log_message("* morten.harms(at)zmaw.de                 *");
	log_message("*                                         *");
	log_message("*******************************************");
	log_message("Today is %02d.%02d.%04d %02d:%02d",
		t.tm_mday, t.tm_mon, t.tm_year + 1900, t.tm_hour, t.tm_min);

	return 0;
}

int log_message(char *message, ...)
{
	va_list args;
	va_start(args, message);
	return logg("INFO ", message, args);
	va_end(args);
}

int log_error(char *message, ...)
{
	va_list args;
	va_start(args, message);
	return logg("ERROR", message, args);
	va_end(args);
}

int log_debug(char *message, ...)
{
	va_list args;

	if(!conf || conf->debug) return 0;

	va_start(args, message);
	return logg("DEBUG", message, args);
	va_end(args);
}

int logg(char * type, char *message, va_list args)
{
	static time_t time_ptr;
	static struct tm t;
	static char buffer[LOG_BUFFER_SIZE];
	static char buffer2[LOG_BUFFER_SIZE];
	time(&time_ptr);
	t = *gmtime(&time_ptr);

	// resolve placeholders message
	vsnprintf(buffer, LOG_BUFFER_SIZE, message, args);

	// generate final log string
	snprintf(buffer2, LOG_BUFFER_SIZE, "%02d:%02d:%02d | %s | %s\n",
		t.tm_hour, t.tm_min, t.tm_sec, type, buffer);

	if(strcmp(type, "ERROR"))
		fprintf(stdout, "%s", buffer2);
	else
		fprintf(stderr, ANSI_COLOR_RED "%s" ANSI_COLOR_RESET, buffer2);

	if(logfile){
		fprintf(logfile, "%s", buffer2);
	}

	return 0;
}

int log_uninit(void)
{
	va_list args;
	logg("EXIT ", "The program exited this log file ends here", args);
	if(logfile){
		fclose(logfile);
	}
	return 0;
}
