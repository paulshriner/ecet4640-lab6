#ifndef Logfile_h
#define Logfile_h

#include <stdio.h>

/**
 * \defgroup Logfile
 * \brief Logs server activity to a file, so we can see who has been using the server.
 * @{
*/

/**
 * Sets the name of the logfile, as retrieved from the server-settings.txt file.
 * 
 * This file will be appended to on each log.
*/
void SetLogfileName(char * logfile_name);

/**
 * Logs an error to the logfile. Will be in the format:
 * 
 * MM-DD-HH-MM-SS ERR <message>
 * 
 * @param format A format string, as with printf
 * @param ... Additional args
*/
void LogfileError(const char * format, ...);

/**
 * Logs a message to the logfile. Will be in the format:
 * 
 * MM-DD-HH-MM-SS MSG <message>
 * 
 * @param format A format string, as with printf
 * @param ... Additional args
*/
void LogfileMessage(const char * format, ...);

/**
 * @}
*/
#endif 

