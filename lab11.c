//SO2 IS1 211A LAB11
//Sebastian Stolarski
//sstolarski@wi.zut.edu.pl

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <wait.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT 255

static int callback(const char *fpath, const struct stat *sb, int typeflag) {
	char *s = (char*)malloc(sizeof(char)*DEFAULT);
	memset(s,0,DEFAULT);
	struct passwd *u = getpwuid(sb->st_uid);
	struct group *g = getgrgid(sb->st_gid);
	time_t _time;
	char time[20];
	strftime(time,20,"%b %d %H:%M", localtime(&(sb->st_mtime)));
	mode_t m = sb->st_mode;
	sprintf(s, "%s%c", s ,S_ISDIR(m)?'d':'-');
	sprintf(s, "%s%c", s ,(S_IRUSR & m)?'r':'-');
	sprintf(s, "%s%c", s ,(S_IWUSR & m)?'w':'-');
	sprintf(s, "%s%c", s ,(S_IXUSR & m)?'x':'-');
	sprintf(s, "%s%c", s ,(S_IRGRP & m)?'r':'-');
	sprintf(s, "%s%c", s ,(S_IWGRP & m)?'w':'-');
	sprintf(s, "%s%c", s ,(S_IXGRP & m)?'x':'-');
	sprintf(s, "%s%c", s ,(S_IROTH & m)?'r':'-');
	sprintf(s, "%s%c", s ,(S_IWOTH & m)?'w':'-');
	sprintf(s, "%s%c\t", s ,(S_IXOTH & m)?'x':'-');
	sprintf(s, "%s%d\t", s,(int)sb->st_nlink);
	sprintf(s, "%s%s\t", s,u->pw_name);
	sprintf(s, "%s%s\t", s,g->gr_name);
	sprintf(s, "%s%d\t", s,(int)sb->st_size);
	sprintf(s, "%s%s\t", s,time);
	sprintf(s, "%s%s",s,fpath);
	printf("%s\n", s);
	return 0;
}

int pid, depth;
DIR* accdir;
struct dirent* info;
char* path2, *value;

void disp(char * path) {
	struct stat stats;
	stat(path,&stats);
	callback(path,&stats,0);
}

int main(int argc, char **argv) {
	char * path;
	if (argc<3) {
		fprintf(stderr, "potrzeba 2 parametrow");
		return(-1);
	} else {
		path=(char*)malloc(sizeof(char)*strlen(argv[1]));
		sprintf(path, "%s", argv[1]);
		depth=atoi(argv[2]);
	}
	accdir = opendir(path);

	while(info = readdir(accdir)) {
		if(strcmp(info->d_name, ".") != 0 && strcmp(info->d_name, "..") != 0) {
			path2 = (char*)malloc(sizeof(char)*DEFAULT);
			sprintf(path2,"%s%c%s",path,'/',info->d_name);
			if(info->d_type == DT_DIR) {
				disp(path);
				pid = fork();
				switch (pid) {
				case 0:
					if(--depth > 0) {
						value = (char*)malloc(sizeof(char)*DEFAULT/8);
						sprintf(value, "%d", depth);
						execl("./p1", argv[0], path2, value , NULL);
					} else { 
						return(0);
					}
					break;
				case -1:
					fprintf(stderr, "fork result : -1");
					return(-1);
					break;
				default:
					waitpid(pid, NULL, 0); 
					break;
				}
			} else {
				disp(path);
			}
		}
	}
	return(0);
}
