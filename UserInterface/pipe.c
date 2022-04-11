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
    
    char str1[80] = "";
    fd1 = open(myfifo,O_WRONLY); 
    write(fd1, "00", 2); 
     
    close(fd1);

    return 0; 
} 

