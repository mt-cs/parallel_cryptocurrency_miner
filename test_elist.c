#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "elist.h"
#include "logger.h"
#include "util.h"
#include "da.h"

struct Etr
{
	int bytes;
    time_t time;
    char *path;
};

int main(void){
	time_t mytime;
	mytime = time(NULL);

	LOG("Size of Entries: %zu\n", sizeof(struct Etr));
	struct elist *list = elist_create(0, sizeof(struct Etr));
	struct Etr entry;
	entry.bytes = 180;
	entry.time = (long long)localtime(&mytime);
	entry.path = "./lodie";
	elist_add(list, &entry);
	LOG("adding: %s\n", entry.path);
	
	struct Etr entry2;
	entry2.bytes = 290;
	entry2.time = (long long)localtime(&mytime) + 5;
	entry2.path = "./nemo";
	elist_add(list, &entry2);
	LOG("adding: %s\n", entry2.path);
	
	struct Etr entry3;
	entry3.bytes = 130;
	entry3.time = (long long)localtime(&mytime) + 10;
	entry3.path = "./marisa";
	elist_add(list, &entry3);
	LOG("adding: %s\n", entry3.path);

	LOG("\nSize of list: %zu\n", elist_size(list));
	
	for (size_t i = 0; i < elist_size(list); i++) {
    	struct Etr *e = elist_get(list, i);
    	printf("Path: %s\n", e->path);	
    	printf("Time: %lld\n", (long long)e->time);
    	printf("Bytes: %d\n", e->bytes);
	}

	elist_sort(list, comparator_time);
	printf("\n\nAFTER SORT\n");
	
	for (size_t i = 0; i < elist_size(list); i++) {
    	struct Etr *e = elist_get(list, i);
    	printf("Path: %s\n", e->path);	
    	printf("Time: %lld\n", (long long)e->time);
    	printf("Bytes: %d\n", e->bytes);
	}
	
	elist_destroy(list);
	return 0;
}


