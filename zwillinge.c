#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <sqlite3.h>

#include "lib/sha1.h"

long int filesProcessed = 0;
long int totalFiles = 0;

int count_file_in_folder(char *dir);
void scan_folder(char *dir);
char *get_binary_of_file(char *pathToFile);
char *get_sha1(char *binary);
void write_uri_file(char *urifile);
void write_log(char *filenameerror);
static inline void loadBar(int x, int n, int r, int w);

int main(int argc, char const *argv[]) {

	if (count_file_in_folder((char*)argv[1]) == 1) {
		printf("\n\n\033[22;32mOK !");
		printf("\n\n\033[22;32mAnalyzed Files: %ld\n\n", totalFiles);
		
		/* printf("\n\n\033[22;31mSearching equals..."); */
		
		printf("\033[22;31mSearching equal files...\n\n");
		
		scan_folder((char*)argv[1]);
		
		printf("\n\n\033[22;32mOK !");
		printf("\n\n\033[22;32mAnalyzed Files: %ld\n", totalFiles);
	}
				
	return 0;
}

char *get_binary_of_file(char *pathToFile)
{
	FILE *fp;
	char *buffer;
	size_t size, result;
	
	if ((fp = fopen(pathToFile, "rb")) == NULL) {
		printf("file: Could not open the file - %s\n", pathToFile);
		write_log(pathToFile);
	} else {
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		rewind(fp);
					
		buffer = (char *)malloc(size * sizeof(char));
		result = fread(buffer, 1, size, fp);
		
		if (result != size) {
			printf("file: An error occurred while trying to read the file - %s.\n", pathToFile);
			write_log(pathToFile);
		}
		fclose(fp);
		
		return buffer;
	}	
}

char *get_sha1(char *binary)
{
	SHA1Context sha;
 	char buffer[100];
	char *result;
	int i;
	
	SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char *) binary, strlen(binary));

    if (!SHA1Result(&sha)) {
        fprintf(stderr, "sha1: Could not compute message digest\n");
    }
    else {
        sprintf(buffer, "%x%x%x%x%x",	sha.Message_Digest[0],
										sha.Message_Digest[1],
										sha.Message_Digest[2],
										sha.Message_Digest[3],
										sha.Message_Digest[4]);
										
		strcpy(result, buffer);
		return result;
    }
}

int count_file_in_folder(char *dir)
{
	DIR *dp;
	struct dirent *ep;
	char path[PATH_MAX +1];
	char msgError[PATH_MAX +1];
	
	printf("\033[22;31mAnalyzing folders...");
	printf("\r");
	fflush(stdout);
	
	dp = opendir(dir);
	if (dp != NULL) {
		while (ep = readdir (dp)) {
			if (ep->d_type == DT_REG) {
				totalFiles++;
			}
			if (ep->d_type == DT_DIR) {
				if ((strcmp(ep->d_name, ".") != 0) && (strcmp(ep->d_name, "..") != 0)) {
					memset(path, 0, sizeof(path));
					
					sprintf(path, "%s/%s", dir, ep->d_name);
					count_file_in_folder(path);
				}
			}
		}
		(void) closedir (dp);
		return 1;
	}
	else {
		sprintf(msgError,"Could't open the directory - %s", dir);
		perror(msgError);
	}	
}

void scan_folder(char *dir)
{
	DIR *dp;
	struct dirent *ep;
	char *binaryfile;
	char *sha1;
	char path[PATH_MAX +1];
	char fullpath[PATH_MAX +1];
	char msgError[PATH_MAX +1];
	
	dp = opendir(dir);
	if (dp != NULL) {
		while (ep = readdir (dp)) {
			
			if (ep->d_type != DT_DIR) {
				if (ep->d_type == DT_REG) {
					filesProcessed++;
					loadBar(filesProcessed,totalFiles,50,50);
					/* printf("%ld-%ld\n", filesProcessed, totalFiles); */
				
					sprintf(fullpath,"%s/%s", dir, ep->d_name);
					/* write_uri_file(fullpath); */
					binaryfile = get_binary_of_file(fullpath);
					sha1 = get_sha1(binaryfile);
					/* printf("%s - %s\n", fullpath, sha1); */
				}	
			}
			
			if (ep->d_type == DT_DIR) {
				if ((strcmp(ep->d_name, ".") != 0) && (strcmp(ep->d_name, "..") != 0)) {
					strcpy(path, "");
					strcat(path, dir);
					strcat(path, "/");
					strcat(path, ep->d_name);
					scan_folder(path);
				}
			}
		}
		(void) closedir (dp);
	}
	else {
		sprintf(msgError,"Could't open the directory - %s", dir);
		perror(msgError);
	}	
}

void write_uri_file(char *urifile)
{
	FILE *urifp;
		
	if ((urifp = fopen("uri", "a+")) == NULL) {
		printf("write uri: Could not open the file");
		exit(1);
	} else {
		fprintf(urifp, "%s\n", urifile);
		fclose(urifp);
	}
}

void write_log(char *filenameerror)
{
	FILE *fp;
	time_t rawtime;
	char filename[PATH_MAX + 1];
	
	time(&rawtime);
	sprintf(filename, "LOG-%s", ctime(&rawtime));
		
	if ((fp = fopen(filename, "a+")) == NULL) {
		printf("file: Could not open the file - %s\n", filename);
		exit(1);
	} else {
		fprintf(fp, "%s\n", filenameerror);
	}
	
	fclose(fp);
}

/* 
		SQLITE3 INTERFACE
		
 */
void open_sqlite3_connection()
{
	const char *dbname = "files.sqlite3";
	sqlite3 *objconnection;
	int status;
	
	status = sqlite3_open_v2(dbname, &objconnection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	
	if (status != SQLITE_OK) {
		printf("db: An error occurred while trying to open connection: \x0a\x09%s\x0a", sqlite3_errmsg(objconnection));
	 }
}

int insert(sqlite3 *objconnection, char *sha1, char *filepath)
{
	int status;
	char *sql;

	return 1;
}

// Process has done i out of n rounds,
// and we want a bar of width w and resolution r.
static inline void loadBar(int x, int n, int r, int w)
{;
    // Only update r times.
    if ( x % (n/r) != 0 ) return;
 
    // Calculuate the ratio of complete-to-incomplete.
    float ratio = x/(float)n;
    int   c     = ratio * w;
 
    // Show the percentage complete.
    printf("%3d%% [", (int)(ratio*100) );
 
    // Show the load bar.
    for (x=0; x<c; x++)
       printf("=");
 
    for (x=c; x<w; x++)
       printf(" ");

	if(x==n)
		printf("OKOKOKO");
 
    // ANSI Control codes to go back to the
    // previous line and clear it.
    /* printf("]\n\033[F\033[J"); */
	printf("\r");
	fflush(stdout);
}