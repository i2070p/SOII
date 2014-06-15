//SO2 IS1 211A LAB12
//Sebastian Stolarski
//sstolarski@wi.zut.edu.pl

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#define DEFAULT 128
#define H 3
#define W 3
#define PLAYER1 1
#define PLAYER2 2
#define DRAW 3
#define GAME 0

struct gameData {
	char map[3][3];
	char fin;
	char mov;
	char pCount;
	char state;
	int w,h;
};

void sopsModify(struct sembuf * buf, int p1, int p2) {
	buf->sem_num=p1;
	buf->sem_op=p2;
	buf->sem_flg=0;
}

char checkInput(char * s, int len) {
	int i;
	for (i=0; i<len; i++) {
		if (s[i] > 57 || s[i] < 48) return(0);
	}
	if (atoi(s) > 3 || atoi(s) < 1) return(0);
	return(1);
}

int input(char *text) {
	char val[DEFAULT];
	do {
		printf("%s", text);
		scanf("%s",val);
	} while (!checkInput(val,strlen(val)));
	return(atoi(val));
}

void displayMap(struct gameData *gData) {
	int i, j;
	char z;
	printf("Aktualny stan: \n");
	for (i=0; i<gData->h; i++) {
		for (j=0; j<gData->w; j++) {
			switch (gData->map[i][j]) {
			case 1:
				z='O';	
				break;
			case 2:
				z='X';
				break;
			default:
				z='+';
				break;
			}
			printf("%c", z);
		}
		printf("\n");
	}
}

char move(int x, int y, int k, struct gameData *gData) {
	if (gData->map[x][y]==0) {
		gData->map[x][y]=k;
		return(1);
	} 
	printf("To pole jest juz zajete.\n");
	return(0);
}


//dopasowana do 3x3
int checkState(struct gameData *gData, int player) {
	int i, j;	

	for (i=0; i<3; i++) {
		if ((gData->map[i][0] == gData->map[i][1]) && (gData->map[i][1] == gData->map[i][2])) {
			if (gData->map[i][0] == PLAYER1 || gData->map[i][0] == PLAYER2) return(gData->map[i][0]);
		}
		if ((gData->map[0][i] == gData->map[1][i]) && (gData->map[1][i] == gData->map[2][i])) {
			if (gData->map[0][i] == PLAYER1 || gData->map[0][i] == PLAYER2) return(gData->map[0][i]);
		}
	}

	if (((gData->map[0][0] == gData->map[1][1]) && ( gData->map[1][1] == gData->map[2][2]))|| 
		((gData->map[0][2] == gData->map[1][1]) && ( gData->map[1][1] == gData->map[2][0]))) {
		if (gData->map[1][1] == PLAYER1 || gData->map[1][1] == PLAYER2) return(gData->map[1][1]);
	}


	for (i=0; i<gData->h; i++) 
		for (j=0; j<gData->w; j++)
			if (gData->map[i][j]==0) return(GAME);


	return(DRAW);
}

void action(int player, int semId, struct sembuf *sops, struct gameData *gData) {
	char result[DEFAULT];
	int x, y;
	while (gData->state==GAME) {
		sopsModify(sops, 0, -1);
		semop(semId, sops, 1); 
		while (gData->pCount<2);
		if (gData->state!=GAME) break;
		displayMap(gData);
		do {
			x=input("Podaj x: ");
			y=input("Podaj y: ");
		} while (!move(x-1, y-1, player, gData));
		displayMap(gData);
		gData->state=checkState(gData, player);
		sopsModify(sops, 0, 1);
		semop(semId, sops, 1); 
	}

	switch(gData->state) {
	case PLAYER1:
		printf("Wygral gracz 1.\n");
		break;
	case PLAYER2:
		printf("Wygral gracz 2.\n");
		break;
	case DRAW:
		printf("Remis!\n");
		break;
	}

	shmdt(gData);
}

int semKey = 0;
int memKey = 0;
int segId;

struct gameData *gData;
struct sembuf sops;
int semId;

int main(int argc, char **argv) {

	if (argc < 3) {
		fprintf(stderr, "Musisz podac 2 parametry.\n");
		return(-1);
	} else {
		memKey=atoi(argv[1]);
		semKey=atoi(argv[2]);
	}

	segId = shmget(memKey, sizeof(struct gameData), 0777);

	if (segId < 0) {
		segId = shmget(memKey, sizeof(struct gameData), IPC_CREAT | 0777);
		gData = shmat(segId, NULL, 0);
		gData->pCount++;
		gData->h=H;
		gData->w=W;
		gData->state=GAME;
		memset(gData->map, 0, W*H);
		semId = semget(semKey, 1, IPC_CREAT | 0600);
		semctl( semId, 0, SETVAL, 1);
		printf("Czekaj na przeciwnika...\n");
		action(PLAYER1, semId, &sops, gData);
		semctl(semId, 0, IPC_RMID);
	} else {
		semId = semget(semKey, 1, 0600);
		gData = shmat(segId, NULL, 0);
		gData->pCount++;
		if (gData->pCount < 3) {
			action(PLAYER2, semId, &sops, gData);
			semctl(semId, 0, IPC_RMID);
		} else {
			printf("Komplet graczy.\n");
		}
	}

	getchar();
	return(0);
}
