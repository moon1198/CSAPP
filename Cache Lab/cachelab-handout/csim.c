#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <math.h>








void usage(){
	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("  -h         Print this help message.\n");
	printf("  -v         Optional verbose flag.\n");
	printf("  -s <num>   Number of set index bits.\n");
	printf("  -E <num>   Number of lines per set.\n");
	printf("  -b <num>   Number of block offset bits.\n");
	printf("  -t <file>  Trace file.\n\n");
	printf("Examples:\n");
	printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
	printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

bool printTraceInof = 0;
int number_s, number_e, number_b;
FILE * file = NULL;
void parse_option(int argc, char *argv[]){
	int option;
	while((option = getopt(argc, argv, "hvs:E:b:t:")) != -1){
		switch(option){
		case 'h':
			usage();
			break;
		case 'v':
			printTraceInof = 1;
			break;
		case 's':
			number_s = atoi(optarg);
			break;
		case 'E':
			number_e = atoi(optarg);
			break;
		case 'b':
			number_b = atoi(optarg);
			break;
		case 't':
			file = fopen(optarg, "r");
			if (file == NULL) {
				printf("Failed to open the file.\n");
			}
			break;
		default:
			usage();
			printf("Invaild option.\n");
			break;
		}
	}
	if (number_s < 0 || number_b < 0 || number_e < 0 || file == NULL){
		usage();
	}
} 

/*
cache_line * ptr_set[number_s]; 
for (i=0; i<number_s; i++){
	ptr_set[i] = (cache_line*)calloc(number_e, sizeof(cache_line));
}
*/


typedef struct Node {
	bool isvaild;
	int tag_e;
	int set_s;
	int index;
}cache;
cache ** myCache;

cache ** initCache(){

	int S = pow(2, number_s);
	int E = number_e;
	myCache = (cache**) calloc(S, sizeof(cache *));
	for (int i=0; i<S; i++){
		myCache[i] = (cache*) calloc(E, sizeof(cache));
		for (int j=0; j<E; j++){
			myCache[i][j].set_s = i;
		}

	}
	return myCache;
}

void freeCache(cache ** myCache){
	
	int S = pow(2, number_s);
	for (int i=0; i<S; i++){
		free(myCache[i]);
	}
	free(myCache);
}


int miss_count=0, hit_count=0, eviction_count=0;


int cache_OP(long unsigned address, int size){
	unsigned long addr_rest,tag,set;
	bool isFound = 0;
	int B = pow(2, number_b);
	int S = pow(2, number_s);
	int E = number_e;
	while (size>0){isFound = 0;
	addr_rest = address / B;
	tag = addr_rest / S;
	set = addr_rest % S;
	for (int i=0; i<E; i++){
		if (!myCache[set][i].isvaild)continue;
		if ((myCache[set][i].tag_e == tag) && (myCache[set][i].set_s == set)){
			printf("hit ");
			hit_count ++;
			isFound =1;
			for (int k=0; k<E; k++){
				if (myCache[set][k].isvaild){
					if(myCache[set][k].index < myCache[set][i].index)
						myCache[set][k].index += 1;
				}
			}
			myCache[set][i].index = 1;
			break;
		}
	}
	if(!isFound){
		printf("miss ");
		miss_count ++;
//has space been rest
		for (int j=0;j<E;j++){
			if (!myCache[set][j].isvaild){
				for (int k=0; k<E; k++){
					if (myCache[set][k].isvaild){
						myCache[set][k].index += 1;
					}
				}
				myCache[set][j].isvaild = 1;
				myCache[set][j].tag_e = tag;
				myCache[set][j].set_s = set;
				myCache[set][j].index = 1;
				return 0;
			}
		}
//no space rest,should eviction
		for (int j=0; j<E; j++){
			if (myCache[set][j].index == E){
				myCache[set][j].tag_e = tag;
				myCache[set][j].index = 1;
				printf("eviction ");
				eviction_count ++;
				continue;
			}
		myCache[set][j].index += 1;
		}
	}
	size -= B;
	address += B;
	}
	return 0;
}

char identifier;	//type of memory access
unsigned long address;	//memory access address
int size;	//memory access size

void readTraceFile(FILE* file){
	while(fscanf(file, " %c  %lx,%d", &identifier, &address, &size) != -1){
		if (printTraceInof){
			printf("%c %lx,%d ", identifier, address, size);
		}

		switch (identifier){
			case 'M':
				cache_OP(address, size);
			case 'L':
			case 'S':
				cache_OP(address, size);
				printf("\n");
				break;
			default:
				printf("fscanf error\n");			
		}
	}
	fclose(file);
}


int main(int argc, char *argv[])
{

	parse_option(argc, argv);			//parse the option argument
										//
	initCache();		//init the cache structure
	readTraceFile(file);				//read the operation file and exec
	freeCache(myCache);					//free cache
    //printSummary(0, 0, 0);				//output result
    printSummary(hit_count, miss_count, eviction_count);	//output result
	
    return 0;
}
