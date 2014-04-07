//SO2 IS1 211A LAB07
//Sebastian Stolarski
//sstolarski@wi.zut.edu.pl

#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

char *filename, *fileData, searchChar;

int threadCount, charCount=0, searchCharCount=0, i=0;
pthread_t *threadIDs;

struct reach {
	int from, count;
	char *fdata;
	char sChar;
};

void* search(void* data) {
	struct reach *r = data; 
	int l;
	for (l=r->from; l<(r->from+r->count); l++) {
		pthread_mutex_lock(&mymutex);
		if (r->fdata[l]==r->sChar) searchCharCount++;
		pthread_mutex_unlock(&mymutex);
	}
	return(NULL);
}

int main(int argc, char *argv[]) {
	filename=argv[1];
	searchChar=argv[2][0];
	threadCount=atoi(argv[3]);

	int file = open(filename, O_RDONLY);
	if (file == -1) return(-1);

	char *readChar;
	while (read(file, readChar, sizeof(char))!=0) {
		charCount++;
	}

	fileData=(char*)malloc(sizeof(char)*charCount);

	lseek(file, 0, SEEK_SET);
	while (read(file, readChar, sizeof(char))!=0) {
		strcat(fileData, readChar);
	}

	threadIDs=(pthread_t*)malloc(sizeof(pthread_t)*charCount);
	int len=charCount/threadCount;
	if (len > 0) {
		for (i=0; i<threadCount; i++) {
			struct reach r;
			r.fdata=fileData;
			r.sChar=searchChar;
			r.from=i*len;
			if (i!=threadCount-1) {
				r.count=len;
			} else {
				r.count=charCount-len*i;
			}
			pthread_create(&threadIDs[i], NULL, search, &r);
		}

		for (i=0; i<threadCount; i++) pthread_join(threadIDs[i], NULL);

		printf("Found: %d of %c\n", searchCharCount, searchChar);
	} fprintf(stderr, "Characters count / threads count = 0\n");
	close(file);
	return(0);
}
