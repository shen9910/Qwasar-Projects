#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <time.h>
#define MAX_STR_LENGTH 1024
#define MAX_TIME_LENGTH 20
struct fileDescription{
    char* name;
    struct stat* attr;
};
typedef struct fileDescription fileDescription;

void listDir(char*, bool, bool, bool);
void sortList(fileDescription list[1024], int s);
void sortListLex(fileDescription list[1024], int s);
void listFile(char*, char*, bool);
void getPermissions(struct stat, char*);
void getUserName(struct stat, char*);
void getGroupName(struct stat, char*);
void getModTime(struct stat, char*);


//extract the file's type and permissions from the file's stats and formats the string to encode it
void getPermissions(struct stat sb, char* permissionString){

	//determine the filetype and add the corresponding character
	permissionString[0] = '-';
	if(S_ISDIR(sb.st_mode)){
		permissionString[0] = 'd';
	}
	if(S_ISCHR(sb.st_mode)){
		permissionString[0] = 'r';
	}
	if(S_ISBLK(sb.st_mode)){
		permissionString[0] = 'b';
	}
	if(S_ISFIFO(sb.st_mode)){
		permissionString[0] = 'p';
	}
	if(S_ISLNK(sb.st_mode)){
		permissionString[0] = 'l';
	}
	if(S_ISSOCK(sb.st_mode)){
		permissionString[0] = 's';
	}


	//check each permission and set the appropriate character
	permissionString[1] = (sb.st_mode & S_IRUSR) == S_IRUSR ? 'r': '-';
	permissionString[2] = (sb.st_mode & S_IWUSR) == S_IWUSR ? 'w': '-';
	permissionString[3] = (sb.st_mode & S_IXUSR) == S_IXUSR ? 'x': '-';

	permissionString[4] = (sb.st_mode & S_IRGRP) == S_IRGRP ? 'r': '-';
	permissionString[5] = (sb.st_mode & S_IWGRP) == S_IWGRP ? 'w': '-';
	permissionString[6] = (sb.st_mode & S_IXGRP) == S_IXGRP ? 'x': '-';

	permissionString[7] = (sb.st_mode & S_IROTH) == S_IROTH ? 'r': '-';
	permissionString[8] = (sb.st_mode & S_IWOTH) == S_IWOTH ? 'w': '-';
	permissionString[9] = (sb.st_mode & S_IXOTH) == S_IXOTH ? 'x': '-';

	permissionString[10] = '\0';
}

//list a file's info (just the filename if listLong is false)
void listFile(char* dirname, char* filename, bool listLong){
	struct stat sb;

	char fullPath[2*MAX_STR_LENGTH];
	char permissionString[11]; 

	char username[MAX_STR_LENGTH], groupname[MAX_STR_LENGTH];

	char timestring[MAX_TIME_LENGTH];



	//check if the -l option was used
	if(listLong){
		strcpy(fullPath, dirname);
		strcat(fullPath, "/");
		strcat(fullPath, filename);

		if(stat(fullPath, &sb) == -1){
			perror(filename);
			return;
		}
		getPermissions(sb, permissionString);

		getUserName(sb, username);
		getGroupName(sb, groupname);

		getModTime(sb, timestring);

		printf("%s %i %s %s %i %s %s\n",permissionString, (int)sb.st_nlink, username, groupname, (int)sb.st_size, timestring, filename);
	}
	else{
		printf("%s  ",filename);
	}
}

//extracts the last modified time from the file stats
void getModTime(struct stat sb, char* timestring){
	struct tm  *timestamp;
	struct tm *currentTime;
	time_t currentTimeRaw;

	int currentYear, fileYear;


	//get the current year
	time(&currentTimeRaw);
	currentTime = localtime(&currentTimeRaw);
	currentYear = (int)currentTime->tm_year;

	timestamp = localtime((time_t*)&sb.st_mtim); //time the file was last modified
	fileYear = (int)timestamp->tm_year; //year the file was last modified
	//if the file was last modified during the current year, show the time, else show the year
	if(currentYear == fileYear){
		strftime(timestring, MAX_TIME_LENGTH, "%b %d %R", timestamp);
	}
	else{
		strftime(timestring, MAX_TIME_LENGTH, "%b %d %Y", timestamp);
	}
}

//returns the group name of the file's owner from the stats
void getGroupName(struct stat sb, char* groupname){
	struct group *groupInfo;

	groupInfo = getgrgid(sb.st_gid);	

	strncpy(groupname, groupInfo->gr_name, MAX_STR_LENGTH);
}

//returns the username of the file's owner from the file's stats
void getUserName(struct stat sb, char* username){
	struct passwd *userInfo;

	userInfo = getpwuid(sb.st_uid);	
	strncpy(username, userInfo->pw_name, MAX_STR_LENGTH);
}

//sorts files inside fileDescription array
void sortList(fileDescription list[1024], int s){
    int i, j;
    fileDescription temp;
    
    for (i = 0; i < s - 1; i++)
    {
        for (j = 0; j < (s - 1-i); j++)
        {
            if (list[j].attr->st_mtim.tv_sec < list[j + 1].attr->st_mtim.tv_sec)
            {
                //printf("%l\n", list[j].attr->st_mtim.tv_nsec);
                temp = list[j];
                list[j] = list[j + 1];
                list[j + 1] = temp;
            }else if (list[j].attr->st_mtim.tv_sec == list[j + 1].attr->st_mtim.tv_sec){
                if (list[j].attr->st_mtim.tv_nsec < list[j + 1].attr->st_mtim.tv_nsec)
                {
                    //printf("%l\n", list[j].attr->st_mtim.tv_nsec);
                    temp = list[j];
                    list[j] = list[j + 1];
                    list[j + 1] = temp;
                }else if (list[j].attr->st_mtim.tv_nsec == list[j + 1].attr->st_mtim.tv_nsec){
                   if (strcmp(list[j].name, list[j+1].name) > 0)
                {
                    temp = list[j];
                    list[j] = list[j + 1];
                    list[j + 1] = temp;
                }  
                }
            }


        }
    }

}

//sorts files inside fileDescription array
void sortListLex(fileDescription list[1024], int s){
    int i, j;
    fileDescription temp;
    
    for (i = 0; i < s - 1; i++)
    {
        for (j = 0; j < (s - 1-i); j++)
        {
            if (strcmp(list[j].name, list[j+1].name) > 0)
            {
                temp = list[j];
                list[j] = list[j + 1];
                list[j + 1] = temp;
            } 
        }
    }

}
//reads a directory to read each file it contains
void listDir(char* dirname, bool listAll, bool listLong, bool listSorted){
    
    fileDescription fs[1024];
    int num = 0; //number of files we goin to print
    DIR* dirp;
	struct dirent *dirEntry;
	char* filename;

	//check if the directory opens successfully
	if((dirp = opendir(dirname)) == NULL){
		perror(dirname);
	}
	else{
		//go through each item in the directory
		while((dirEntry = readdir(dirp)) != NULL){
			filename = dirEntry->d_name;

			//only list hidden files (. files) if the -a option was used (ie listAll is true)
			if(filename[0] == '.'){
				if(listAll){
					char *fullPath = malloc(1024);
                    strcpy(fullPath, dirname);
                    strcat(fullPath, "/");
                    strcat(fullPath, filename);
                    struct stat* sb = malloc(sizeof(struct stat));

                    if(stat(fullPath, sb) == -1){
                        perror(filename);
                        return;
                    }

                    fs[num].name = (char*)malloc(strlen(filename)+1);
                    strcpy(fs[num].name, filename);
                    fs[num].attr = sb;
                    num++;
                    free(fullPath);

				}
			}else{
				char *fullPath = malloc(1024);
                strcpy(fullPath, dirname);
                strcat(fullPath, "/");
                strcat(fullPath, filename);
                struct stat* sb = malloc(sizeof(struct stat));

                if(stat(fullPath, sb) == -1){
                    perror(filename);
                    return;
                }

                fs[num].name = (char*)malloc(strlen(filename)+1);
                strcpy(fs[num].name, filename);
                fs[num].attr = sb;
                num++;
                free(fullPath);
			}

		}
		//cleanup
        if (listSorted){
            sortList(fs, num);
        }else{
            sortListLex(fs, num);
        }

        for (int i = 0; i<num; i++){
            listFile(dirname, fs[i].name, listLong);
            free(fs[i].name);
            free(fs[i].attr);
        }

		closedir(dirp);
	}
}