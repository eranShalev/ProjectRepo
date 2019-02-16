// C program to implement one side of FIFO 
// This side reads first, then reads 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 

int main() 
{ 
    int fd1; 

    // FIFO file path 
    char * myfifo = "/tmp/myfifo"; 

    // Creating the named file(FIFO) 
    // mkfifo(<pathname>,<permission>) 
    //mkfifo(myfifo, 0666); f1
     
    // First open in read only and read 
    while (1)
    {
    
    char str1[80] = "";
    fd1 = open(myfifo,O_RDONLY); 
    read(fd1, str1, 80); 

    // Print the read string and close 
    printf("User1: %s\n", str1); 
    close(fd1);

    if (strcmp(str1, "01") == 0)
    {
        return 0;
    }
    }
    return 0; 
} 

