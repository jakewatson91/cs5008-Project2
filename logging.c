#include <stdio.h>   // For printf()
#include <time.h>    // For time-related functions

/*
All entries should be prefixed with the time and client IP address 
in YYYY-MM-DD HH:mm:ss xxx.xxx.xxx.xxx format, where 
1) “YYYY-MM-DD” is the year, month, and day, 
2) “HH:mm:ss” is the time of day in 24 hour format (HH can take values from 00-23), and 
3) “xxx.xxx.xxx.xxx” represents the IP address of the client in dotted decimal format. 
Valid QR code interactions and invalid actions from clients should be logged. 
The log file must also contain information about server start-ups, connections, and disconnections from users, 
events as described by the RATE/MAX USERS instructions, 
and any other information regarding user behavior or system events.
*/

int logging(char *hostname) {
    char buff[20];        // Buffer to store the formatted date and time string
    struct tm *sTm;       // Pointer to a struct tm to hold the time components

    FILE *fp;
    fp = fopen("x.log", "a+"); // a+ (create + append) option will allow appending which is useful in a log file
    if (fp == NULL) {
        fprintf(stderr, "Error opening file.");
        }
    
    fprintf(fp, "Logging ...\n");

    time_t now = time(0); // Get the current time as a time_t object
    sTm = gmtime(&now);   // Convert the time to UTC (Coordinated Universal Time)

    // Format the time into a readable string (e.g., "YYYY-MM-DD HH:MM:SS")
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);

    // Print the formatted date and time along with a message
    fprintf(fp, "%s %s\n", buff, hostname);

    fclose(fp);

    return 0; // Indicate successful execution
}
