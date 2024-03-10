#include <stdio.h>
#include <stdlib.h>
#include <time.h>


unsigned long long rand64() {
  unsigned long long r1 = rand();
  unsigned long long r2 = rand();
  return (unsigned long long)((r1 << 32) | r2);
}


void unshuffle(char *array, unsigned long long size, unsigned long long seed) {
    // Initialize the random number generator with the same seed used for shuffling.
    srand(seed);

    // Create an array to store the shuffled indices.
    unsigned long long *indices = (unsigned long long *)malloc(size * sizeof(unsigned long long));
    if (indices == NULL) {
        // Handle memory allocation failure
        return;
    }

    // Generate the same set of random indices as in the shuffle function.
    for (unsigned long long i = 0, v = ((seed >> ((rand64() ^ (seed >> 32)) & 32)) & 63); i != size; i++) {
        indices[i] = ((seed >> v++) & 0xff) % size;
        if (v == 64) {
          v = ((seed >> ((rand64() ^ (seed >> 32)) & 32)) & 63);
        }
    }

    // Reverse the shuffling process.
    for (unsigned long long i = size - 1; i != -1; i--) {
        // Swap the elements at indices[i] and indices[j].
        unsigned long long j = indices[i];
        char temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }

    // Free the allocated memory for the indices array.
    free(indices);
}





void shuffle(char *array, unsigned long long size, unsigned long long seed) {
  // Initialize the random number generator with the seed.
  srand(seed);

  // Shuffle the array.
  for (unsigned long long i = 0, v = ((seed >> ((rand64() ^ (seed >> 32)) & 32)) & 63); i != size; i++) {
    unsigned long long j = ((seed >> v++) & 0xff) % size;
		if (v == 64) {
			v = ((seed >> ((rand64() ^ (seed >> 32)) & 32)) & 63);
		}
    char temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
}



void cry(char* block, unsigned long long bs, unsigned long long key) {
  srand(key >> 32);

	for (unsigned long long i = 0, v = 0; i != bs; i++) {
		block[i] = block[i] ^ (key >> (8*v++)) ^ (key >> 32) ^ rand64();
    srand((key >> (8*v++)) ^ (key >> 32) ^ rand64());
		if (v == 8) {
			v = 0;
		}
	}
}

void encrypt(char*block, unsigned long long sz, unsigned long long key) {
	shuffle(block, sz, key);
	cry(block, sz, key);
	shuffle(block, sz, key);
}

void decrypt(char*block, unsigned long long sz, unsigned long long key) {
	unshuffle(block, sz, key);
	cry(block, sz, key);
	unshuffle(block, sz, key);
}


unsigned long long getFileSize(FILE *file) {
  unsigned long long size;
  fseek(file, 0, SEEK_END); // Move the file pointer to the end of the file
  size = ftell(file); // Get the current position of the file pointer, which is the size of the file
  rewind(file); // Move the file pointer back to the beginning of the file
  return size;
}

int main(int i, char** args) {
  if (i != 4) {
    printf("Usage: coolcrypt <64 bit HEX key> <file> <Direction of encyrption(+/- use inverse to decrypt)>\n");
    return 1;
  }

  for (int i = 0; args[1][i] != '\0'; i++) {
    if (i == 0x10) {args[1][i] = '\0'; printf("\nKey to long, trunceting!\n"); break;}
  }
	unsigned long long key = strtoull(args[1], (char **)NULL, 0x10);

  FILE *file = fopen(args[2], "r");
  if (file == NULL) {
      printf("Error opening the file.\n");
      return 1;
  }

	unsigned long long sz = getFileSize(file);
	unsigned char* block = (unsigned char*)malloc(sizeof(unsigned char) * sz);
	fread(block, sizeof(unsigned char), sz, file);
  fclose(file);

  if (args[3][0] == '+')
	  encrypt(block, sz, key);

  else if (args[3][0] == '-')
    decrypt(block, sz, key);
  
  else {
    printf("No operation!\n");
    free(block);
    return -1;
  }

  file = fopen(args[2], "w");
  fwrite(block, sizeof(unsigned char), sz, file);
  fclose(file);

  free(block);

	return 0;
}

