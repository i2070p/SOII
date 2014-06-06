//SO2 IS1 211A LAB08
//Sebastian Stolarski
//sstolarski@wi.zut.edu.pl

#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int n, type, srv_qid, clt_qid;

struct package {
	long mtype;
	int value;
	int qid;
};

int main(int argc, char *argv[]) {
	if (argc!=4) {
		printf("error\n");
		return(-1);
	}

	srv_qid=atoi(argv[1]);
	n=atoi(argv[2]);
	type=atoi(argv[3]);
	clt_qid = msgget(getpid(), IPC_CREAT|0666);

	if (clt_qid >= 0) {
		printf("Client Queue id %d: \n", clt_qid);
	} else {
		printf("Create error\n");
		return(-1);
	}

	struct package* P1 = (struct package*) malloc (sizeof(struct package));
	struct package* P2 = (struct package*) malloc (sizeof(struct package));

	P1->mtype=type;
	P1->value=n;
	P1->qid=clt_qid;

	if (msgsnd(srv_qid, P1, sizeof(struct package), 0)!=-1) {
		printf("Sending to %d succesfull\n", srv_qid);
	} else {
		fprintf(stderr,"sending error %d\n", srv_qid);
		return(-1);
	}
	free(P1);

	if (msgrcv(clt_qid, P2, sizeof(struct package),-5 ,0)) {
		printf("Recv value: %d\n", P2->value);                    
	} else {
		fprintf(stderr,"Recv. error\n");
		return(-1);
	}  
	free(P2);

	if (msgctl(srv_qid, IPC_RMID, NULL)!=-1) {
		printf("server queue has been deleted\n");
	} else {
		fprintf(stderr,"error, server queue still exist\n");
		return(-1);                        
	}

	if (msgctl(clt_qid, IPC_RMID, NULL)!=-1) {
		printf("client queue has been deleted\n");
	} else {
		fprintf(stderr,"error, client queue still exist\n");
		return(-1);                        
	}	

	return(0);
}


//SO2 IS1 211A LAB08
//Sebastian Stolarski
//sstolarski@wi.zut.edu.pl



#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <errno.h>
int srv_qid;

struct package {
	long mtype;
	int value;
	int qid;
};


int main(int argc, char *argv[]) {
	srand(time(0));
	srv_qid = msgget(getpid(), IPC_CREAT|0666);
	if (srv_qid >= 0) {
		printf("Server Queue id: %d\n", srv_qid);
	} else {
		fprintf(stderr,"Create error\n");
		return(-1);
	}

	struct package * P1 = (struct package*)malloc(sizeof(struct package));

	while (msgrcv(srv_qid, P1, sizeof(struct package),-5 ,0)) {
		printf("something...\n");
		struct package * P2 = (struct package*)malloc(sizeof(struct package));     
		P2->mtype=P1->mtype;
		P2->qid=srv_qid;
		P2->value=rand()%P1->value;

		if (msgsnd(P1->qid, P2, sizeof(struct package), 0)!=-1) {
			printf("sending to %d succesfull\n",P1->qid); 
            break;                   
		} else {
			fprintf(stderr,"sending error\n");
			return(-1);
		}
		free(P2);
	}

	free(P1);
	printf("bye...\n");
	return(0);
}
