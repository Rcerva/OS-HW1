#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#define BUFFER_SIZE (((size_t)1024))
#define DEFAULT_LINES 10

int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}
void readstdin(int lines) {
    char **buffer = NULL; 
    int line_count = 0;
    // Read lines from stdin until EOF is encountered
    while (1) {
        char *line = malloc(BUFFER_SIZE);
        if (line == NULL) {
            perror("Error allocating memory");
            return;
        }
        int index = 0;
        char ch;
        while (read(STDIN_FILENO, &ch, 1) == 1 && ch != '\n') {
            // Append character to the line
            line[index++] = ch;
            if (index >= BUFFER_SIZE) {
                char *temp = realloc(line, index * 2); 
                if (temp == NULL) {
                    perror("Error reallocating memory");
                    free(line); 
                    return; 
                }
                line = temp;
            }
        }
        line[index] = '\0';
        if (index == 0) {
            free(line);
            break;
        }
        char **temp = realloc(buffer, (line_count + 1) * sizeof(char *));
        if (temp == NULL) {
            perror("Error reallocating memory");
            free(line); 
            return; 
        }
        buffer = temp;
        buffer[line_count] = line;
        line_count++;
    }
    // Print the last 10 lines (or specified number of lines) from the buffer
int start_line = (line_count > lines) ? line_count - lines : 0;
for (int i = start_line; i < line_count; i++) {
    int j = 0;
    while (buffer[i][j] != '\0') {
        if (write(STDOUT_FILENO, &buffer[i][j], 1) != 1) {
            perror("Error writing to stdout");            
        }
        j++;
    }
    // Write a newline character
    if (write(STDOUT_FILENO, "\n", 1) != 1) {
        perror("Error writing to stdout");       
    }
    free(buffer[i]); 
}
} 
void readfromfile(const char *filename, int lines) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    // Determine the file size
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Error seeking end of file");
        close(fd);
        return;
    }
    char *buffer = (char *)malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Error allocating memory for buffer");
        close(fd);
        return;
    }
    // Move the file pointer to the end of the file
    off_t position = lseek(fd, -BUFFER_SIZE, SEEK_END);
    if (position == -1) {
        perror("Error seeking to end of file");
        close(fd);
        free(buffer);
        return;
    }
    int bytes_read;
    int newline_count = 0;
    // Read chunks of data until the desired number of lines is reached
    while (position >= 0 && newline_count < lines) {
        bytes_read = read(fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Error reading from file");
            close(fd);
            free(buffer);
            return;
        }
        // Process the data in reverse order
        for (int i = bytes_read - 1; i >= 0; i--) {
            if (buffer[i] == '\n') {
                newline_count++; 
                if (newline_count >= lines) {
                    // Output or store the lines as needed
                    write(STDOUT_FILENO, buffer + i + 1, bytes_read - i - 1);
                    break;
                }
            }
        }
        // Move to the previous position in the file
        position -= bytes_read;
        lseek(fd, position, SEEK_SET);
    }
    free(buffer);
    close(fd);  
    }
// Function to check if a character is a digit
int is_digit(char c) {
    return c >= '0' && c <= '9';
    }
// Function to parse an integer from a string
int parse_int(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;
    while (str[i] == ' ' || str[i] == '\t') {
        i++;
    }
    // Handle sign
    if (str[i] == '-') {
        sign = -1;
        i++;
    }
    // Parse digits
    while (is_digit(str[i])) {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return result * sign;
}

// Error handling function
void error(const char *msg) {
    perror(msg);
}
void my_strcpy(char *dest, const char *src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}
/*  Main will determine if the program is reading a file or stdin and call the appropriate function */ 
int main(int argc, char **argv) {
    int deflines = DEFAULT_LINES;
    int n_lines = deflines;
    if (argc == 1) {
        // No filenames provided, read from standard input
        readstdin(deflines);
    } else {
        // Iterate through command-line arguments
        int has_filenames = 0;
        for (int i = 1; i < argc; i++) {
            if (my_strcmp(argv[i], "-n") == 0) {
                // Check if there's a value after -n
                if (i + 1 < argc) {
                    n_lines = parse_int(argv[i + 1]);
                    if (n_lines <= 0) {
                        error("Invalid value for -n option.");
                    }
                    i++; 
                } else {
                    error("-n option requires a value.");
                }
            } else {
                // Assume it's a filename
                has_filenames = 1;
                readfromfile(argv[i], n_lines+1);
            }
        }
        if (!has_filenames) {
            readstdin(n_lines);
        }
    }
    return 0;
}
