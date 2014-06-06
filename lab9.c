//SO2 IS1 211A LAB07
//Sebastian Stolarski
//sstolarski@wi.zut.edu.pl

#include <stdio.h>
#include <malloc.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>

char *filename, *fileData, searchChar;

int procCount, charCount=0, searchCharCount=0, i=0,j=0,pid;
int **socks;
int *counts;

struct reach {
	int from, count;
	char *fdata, *sCharCount;
	char sChar;
};

struct reach * rTab;

int main(int argc, char *argv[]) {
	filename=argv[1];
	searchChar=argv[2][0];
	procCount=atoi(argv[3]);

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

	socks= (int**)malloc(sizeof(int*)*procCount);
	rTab=(struct reach*)malloc(sizeof(struct reach)*procCount);
	int len=charCount/procCount;
	if (len > 0) {
		for (i=0; i<procCount; i++) {
			struct reach r;
			r.fdata=fileData;
			r.sChar=searchChar;
			r.from=i*len;
			if (i!=procCount-1) {
				r.count=len;
			} else {
				r.count=charCount-len*i;
			}

			rTab[i]=r;

			int *sd=(int*)malloc(sizeof(int)*2);
			if(socketpair(AF_UNIX, SOCK_STREAM, 0, sd) == -1)	{
				fprintf(stderr,"socketpair error");
				return(-1);
			}
			socks[i]=sd;
			//printf("%d, %d\n",rTab[i].from, rTab[i].count);
		}

		for (i=0; i<procCount; i++) {
			struct reach buf;
			pid = fork();
			switch (pid) {
			case 0:
				if(read(socks[i][1], &buf, sizeof(struct reach), 0) == -1) {
					fprintf(stderr, "recv error\n");
				}

				int * count = (int*) malloc(sizeof(int));
				*count=0;
				for (j=buf.from; j<(buf.from+buf.count); j++) {
					if (buf.fdata[j]==buf.sChar) (*count)++;
				}
				if(write(socks[i][0], count, sizeof(int)) == -1) {
					fprintf(stderr, "send error\n");
				}
				return(0);
				break;
			case -1:
				fprintf(stderr, "fork result: -1\n");
				return(-1);
				break;
			default:
				if(write(socks[i][0], &rTab[i], sizeof(struct reach)) == -1){
					fprintf(stderr, "send error\n");
				}	
				break;
			}
		}

		usleep(1000);

		for (i=0; i<procCount; i++) {
			int * count=(int*)malloc(sizeof(int));
			if(read(socks[i][1], count, sizeof(int), 0) == -1) {
				fprintf(stderr, "recv error\n");
			} else {
				searchCharCount+=*count;
			}
		}

		for (i=0; i<procCount; i++) {
			close(socks[i][0]);
			close(socks[i][1]);
		}

		printf("Found: %d of %c\n", searchCharCount, searchChar);
	} else fprintf(stderr, "Characters count / procs count = 0\n");

	close(file);
	return(0);
}
