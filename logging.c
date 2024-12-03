#include <stdio.h>   // For printf()
#include <time.h>    // For time-related functions

// create the file:
FILE *f;
f = fopen("x.log", "a+"); // a+ (create + append) option will allow appending which is useful in a log file
if (f == NULL) { /* Something is wrong   */}
fprintf(f, "I'm logging something ...\n");

int main(void) {
    char buff[20];        // Buffer to store the formatted date and time string
    struct tm *sTm;       // Pointer to a struct tm to hold the time components

    time_t now = time(0); // Get the current time as a time_t object
    sTm = gmtime(&now);   // Convert the time to UTC (Coordinated Universal Time)

    // Format the time into a readable string (e.g., "YYYY-MM-DD HH:MM:SS")
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);

    // Print the formatted date and time along with a message
    printf("%s %s\n", buff, "Event occurred now");

    return 0; // Indicate successful execution
}
