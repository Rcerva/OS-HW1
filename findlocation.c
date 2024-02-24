#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  char number[6];
  char location[25];
  char newline;
} entry_t;

static int compare_entries(char *number_a, char *number_b) {
  char *a;
  char *b;

  for (a=number_a, b=number_b;
       ((*a >= '0') &&
	(*a <= '9') &&
	(*b >= '0') &&
	(*b <= '9'));
       a++, b++) {
    if (*a < *b) return -1;
    if (*a > *b) return 1;
  }
  return 0;
}


int lookUpNumber(entry_t *dict, size_t n, char* number){
    ssize_t l, r, m, nn;
  int res;

  nn = (ssize_t) n;
  if (nn < ((ssize_t) 0)) {
    write(1, "The file is too large\n", 23);
    return -1;
  }

  for (l=((ssize_t) 0), r=(nn-((ssize_t) 1)); l<=r;) {
    m = (l + r) >> 1;
    res = compare_entries(dict[m].number, number);
    if (res == 0) {
      write(1,"The location of ", 16);
      write(1,dict[m].number, 6);
      write(1," is ", 4);
      write(1,dict[m].location, 25);
      write(1, "\n",1);
      return 0;
    }
    if (res < 0) {
      l = m + ((ssize_t) 1);
    } else {
      r = m - ((ssize_t) 1);
    }
  }
  write(1,"The 6 digit telephone number prefix %s was not found in the file\n", 64);
  return 0;
}

int lookUpNumberHelper(void *ptr, size_t size, char* number){
  float n;


  n = 1.0 * size / sizeof(entry_t);
  /* Check that the dictionary size is a multiple of full entries */
  if ((size % sizeof(entry_t)) != ((size_t) 0)) {
    write(2, "The file is not properly formatted\n", 35);
    return -1;
  }

  
  return lookUpNumber((entry_t *) ptr, n, number);
}

// Function to check if the file descriptor is seekable
int isSeekable(int fd) {
    off_t offset = lseek(fd, 0, SEEK_CUR);
    return offset != -1;
}

int myMap(int fd, size_t *fileSize, char* number){
  off_t lseekRes;
  char *error;
  void *ptr;
if (!isSeekable(fd)) {
        // Handle unseekable file descriptor
        write(1, "File descriptor is unseekable. Reading file contents...\n", 55);
        entry_t *dict = NULL;
        ssize_t bytes_read;
        size_t capacity = 0;

        // Read file into memory
        while (1) {
            if (*fileSize >= capacity) {
                capacity += 1024; 
                dict = realloc(dict, capacity * sizeof(entry_t));
                if (dict == NULL) {
                    perror("realloc");
                    return 1;
                }
            }
            bytes_read = read(fd, &dict[*fileSize], sizeof(entry_t));
            if (bytes_read == -1) {
                perror("read");
                free(dict);
                return 1;
            }
            if (bytes_read == 0) break; // End of file
            (*fileSize)++;
        }
        // Perform lookup
        int result = lookUpNumber(dict, *fileSize, number);
        free(dict);
        return result;
    }
  
  lseekRes = lseek(fd, ((off_t) 0), SEEK_END);
  if (lseekRes < ((off_t) 0)) {
    error = "Error: at using lseek on.\n";
    write(2, error, 27);
    if (close(fd) < 0) {
      write(2, "Error: closing.\n", 17);
    }
    return 1;
  }

  *fileSize = (size_t) lseekRes;

  /* Handle the special case of a file that is empty */
  if (*fileSize < ((size_t) 1)) {
    if (close(fd) < 0) {
      write(2, "Error: closing.\n", 17);
      return 1;
    }
    return 0;
  }
  
  /* Map the content of the file described by fd into memory */
  ptr = mmap(NULL, *fileSize, PROT_READ, MAP_SHARED, fd, ((off_t) 0));
  if (ptr == MAP_FAILED) {
    write(2, "Error: mapping.\n", 17);
    if (close(fd) < 0) {
      write(2, "Error: closing.\n", 17);
    }
    return 1;    
  }
  else {
    lookUpNumberHelper(ptr, *fileSize, number);
  }
  return 0;
}


int main(int argc, char **argv) {
  int fd;
  char* number;
  char *error;
  char *fileName = "nanpa";
  size_t fileSize;
  
  
  if (argc <= 1) {
    error = "Error: Invalid number of arguments.\n";
    write(2, error, 37);
    // Error: Invalid Number of arguments.
    return 1;
  }else if (argc == 2) {
    number = argv[1];
    fd = open(fileName, O_RDONLY);
    if (fd < 0) {
      error = "Error: File not found\n";
      write(2, error, 22);
    } else {
      // Need to map
      myMap(fd, &fileSize, number);
    }
    // Get Number provided in Nanpa
  }else if (argc == 3) {
    number = argv[1];
    fileName = argv[2];
    fd = open(fileName, O_RDONLY);
    if (fd < 0) {
      error = "Error: File not found\n";
      write(2, error, 22);
    } else {
      // Need to map
      myMap(fd, &fileSize, number);
    }
  }
  else {
    error = "Error: Invalid number of arguments.\n";
    write(2, error, 37);
    // Error: Invalid Number of Arguments.
    return 1;
  }
  
  return 0;
}
