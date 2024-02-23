#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#define BUFFER_SIZE 10000

int convertStringToInt(char* str){
  uint64_t maxValue = UINT64_MAX;
  uint64_t val;
  int result = 0;
  for(int i = 0; str[i] != '\0'; ++i) {
    if(str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
    } else {
        // Handle invalid characters in the string
        return -1;
    }
  }
  val = result;

  if (val > maxValue) return -1;
  return result;
}

int isSameString(char* str1, char* str2) {
   while (*str1 != '\0' && *str2 != '\0') {
       if (*str1 != *str2) {
           return 0; // Not same
        }
       str1++;
       str2++;
    }
    // Both strings should end at the same time
    return *str1 == '\0' && *str2 == '\0';
}


int myWrite(int fd, const void *buffer, size_t len){
  size_t bytesToBeWritten, bytesWritten, bytesJustWritten;
  ssize_t writeRes;
  char *error;

  // Nothing to write
  if (len < ((size_t) 1)) return 0;
  
  for (bytesToBeWritten = len, bytesWritten = 0; bytesToBeWritten > (size_t) 0;
       bytesToBeWritten -= bytesJustWritten, bytesWritten += bytesJustWritten){
    writeRes = write(fd, &(((char *) buffer)[bytesWritten]), bytesToBeWritten);
    // Failed to Write
    if (writeRes < ((ssize_t) 0)) {
      error = "Error: Failed to write.\n";
      write(1, error, 25);
      return -1;
    }
    bytesJustWritten = (size_t) writeRes;
  }
  return 0;
}

int myRead(int fd, int linesToRead) {
    ssize_t readRes;
    char *error;
    char charBuffer;
    char lineBuffer[BUFFER_SIZE];
    int len = 0;
    int linesRead = 0; // Counter to keep track of lines read
    
    while (linesRead < linesToRead) {
        if ((readRes = read(fd, &charBuffer, 1)) < 0) {
            // Error failed to read
            error = "Error: Failed to read file.\n";
            write(2, error, 29);
            return 1; // Return error code
        } else if (readRes == 0) {
	    lineBuffer[len++] = '\n';
            // End of file reached
	    if (myWrite(1, lineBuffer, len) < 0) {
                // Error failed to write
                error = "Error: Failed to write file.\n";
                write(2, error, 30);
                return 1; // Return error code
            }
            break;
        } else {
	    lineBuffer[len++] = charBuffer;
	    if (charBuffer == '\n'){
	      if (myWrite(1, lineBuffer, len) < 0) {
                // Error failed to write
                error = "Error: Failed to write file.\n";
                write(2, error, 30);
                return 1; // Return error code
	      }
	      linesRead++;
	      len = 0;
	    }
	    // Increment lines counter
        }
    }
    return 0; // Return success code
}


int main(int argc, char ** argv) { 
    int fd;
    int linesToRead = 10;
    char* error;

    if (argc == 1) {
        fd = 0;
        linesToRead = 10;
        myRead(fd, linesToRead); 
    } else if (argc == 2) {
        // Error invalid single argument -
        if (isSameString(argv[1], "-")) {
            fd = 2;
            error = "Error: Invalid argument\n";
            write(2, error, 25);
            return -1;
        } else {
            fd = open(argv[1], O_RDONLY);
            // File not found
            if (fd < 0) {
                error = "Error: File not found\n";
                write(2, error, 22);
            } else {
                myRead(fd, linesToRead);
            }
        }
    } else if (argc == 4) {
        if (isSameString(argv[1], "-n")) {
            if ((linesToRead = convertStringToInt(argv[2])) < 0) {
                error = "Error: Invalid number, must be positive int *-n [num]* \n.";
                write(2, error, 34); 
            }
            fd = open(argv[3], O_RDONLY);
            if (fd < 0) {
                error = "Error: File not found\n";
                write(2, error, 22);
            } else {
                myRead(fd, linesToRead);
            }
        } else if (isSameString(argv[2], "-n")) {
            if ((linesToRead = convertStringToInt(argv[3])) < 0) {
                error = "Error: Invalid number, must be positive int *-n [num]* \n.";
                write(2, error, 34); 
            }
            fd = open(argv[1], O_RDONLY);
            if (fd < 0) {
                error = "Error: File not found\n";
                write(2, error, 22);
            } else {
                myRead(fd, linesToRead);
            }
        } else {
            error = "Error: Invalid argument\n";
            write(2, error, 25);
            return -1;
        }
    } else {
        error = "Error: Invalid arguments\n";
        write(2, error, 26);
        // Error: invalid amount of arguments
        return 1;
    }

    return 0;
}
