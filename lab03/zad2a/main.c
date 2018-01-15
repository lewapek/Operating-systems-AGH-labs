#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#define MAX_PATH_LENGTH 1024

long count(char  *path, long perm_model);

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf (stderr, "Usage:\n\t%s path permissions\n", argv[0]);
		return 1;
	}
	
	char *start_path = argv[1];
	long perm_model = strtol(argv[2], NULL, 8);
	long counter = count(start_path, perm_model);
	printf("Counter = %ld\n", counter);
	
	return 0;
}

long count(char *path, long perm_model)
{
	long internal_counter = 0;
	
	struct dirent *dp;
	DIR *dir = opendir(path);
	if (dir == NULL)
	{
		fprintf (stderr, "%s\n", path);
		perror ("opendir()");
		return internal_counter;
	}

	struct stat *buffer = malloc(sizeof(struct stat));
	
	errno = 0;
	int nerr;
	
  	while ((dp = readdir(dir)) != NULL)
  	{
    	nerr = errno;
	    char npath[MAX_PATH_LENGTH];
	    sprintf (npath, "%s/%s", path, dp->d_name);
	    
	    if (stat(npath, buffer) == -1)
	    {
	    	printf ("%s\n", npath);
	    	perror ("stat");
	    }
	    
	    int counter_inc = 0;
	    
	    if(perm_model == (buffer->st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)))
	    {
	    	counter_inc = 1;
		}

	    if(counter_inc)
	    {
	    	++internal_counter;
	    }

	    if(S_ISDIR(buffer->st_mode))
	    {
	  		if (strcmp (dp->d_name, "..") && strcmp (dp->d_name, "."))
	  		{
	  			internal_counter += count(npath, perm_model);
	    	}
	    }
    	errno = nerr;
  	}
  	if (errno != 0 && (!dp))
  	{
  		perror("readdir");
  	}

  	closedir(dir);
  	free(buffer);

	return internal_counter;
}
