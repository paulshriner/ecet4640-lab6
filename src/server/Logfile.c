/**
 * \addtogroup Logfile
 * @{
*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "Util.h"

char * logfile_name;

char* months[12] = {
    "jan",
    "feb",
    "mar",
    "apr",
    "may",
    "jun",
    "jul",
    "aug",
    "sep",
    "oct",
    "nov",
    "dec"
};

void logline(const char * logtype, char * logmsg) {
    time_t current_time;
    struct tm *timeinfo;
    time(&current_time);
    timeinfo=localtime(&current_time);

    FILE * file = fopen(logfile_name, "a");
    if(file == NULL) {
        printRed("Failed to open logfile %s.\n", logfile_name);
        return;
    }

    fprintf(file, "%s-%02d %02d:%02d:%02d %s %s\n", months[timeinfo->tm_mon], timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, logtype, logmsg);

    fclose(file);
}

void SetLogfileName(char * logfile_name_param) {
    if(logfile_name != NULL) {
        free(logfile_name);
    }
    logfile_name = malloc(strlen(logfile_name_param)+1);
    strcpy(logfile_name, logfile_name_param);
}

void LogfileError(const char * format, ...) {
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);
    
    char * file_print_string = (char*) malloc((size+1)*sizeof(char));
    
    vsprintf(file_print_string, format, args);
    va_end(args);
    logline("ERR", file_print_string);
    free(file_print_string);
}

void LogfileMessage(const char * format, ...) {
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    char * file_print_string = (char*) malloc((size+1)*sizeof(char));
    
    vsprintf(file_print_string, format, args);
    va_end(args);
    logline("MSG", file_print_string);
    free(file_print_string);
}

/**
 * @}
*/