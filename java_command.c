#include <stdlib.h>
#include <stdio.h>

int run_command(char *filename) {
    int SIZE = 20;
    const char *java_command = "java -cp  javase.jar:core.jar com.google.zxing.client.j2se.CommandLineRunner";
    
    if (fgets(filename, SIZE, stdin) == NULL) {
        fprintf(stderr,"Failed to read filename.\n");
        exit(-1);
    };

    // strip newline
    if (filename[strcspn(filename, '\n')] = '\0');

    if (strlen(filename) >= 20) {
        fprintf(stderr, "Filename too long.\n");
        exit(-1);
    }

    // command string
    char command[SIZE + 256]; // command buffer with enough space for full command
    snprintf(command, sizeof(command), "%s %s", java_command, filename); // concatenate and store in command

    // Execute the Java command
    int ret = system(command);
    
    if (ret == -1) {
        perror("system call failed");
        return 1;
    }
    
    printf("Java command executed with return code: %d\n", ret);
    return 0;
}
