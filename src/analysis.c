#include <stdio.h>
#include <stdlib.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

#define FCFS "FCFS" // First come first serve
#define P "P"  // Priority
#define RR "RR" // Round Robin
#define SJF "SJF" // Shortest Job First
#define SRT "SRT" // Shortest remaining Time

// Add and comment your analysis code in this function.
// THIS IS NOT FINISHED.
int main(int argc, char **argv) 
{
	if (argc < 3) 
	{
		printf("%s <pcb file> <schedule algorithm> [quantum]\n", argv[0]);
		return EXIT_FAILURE;
	}

	// Checking if any arguments are NULL
	if(!argv[0] || !argv[1] || !argv[2]) return EXIT_FAILURE;


	// get a dyn array from the pcb file using load proccess control blcok function
	dyn_array_t* pcb = load_process_control_blocks(argv[1]);

	 
	// if else for the names of the algorithms 

	const char *algorithm = argv[2];
	const char *algorithmLen = strnlen(algorithm, 4);
	bool completed = false;
	size_t quant = NULL;
	

	if(strncmp(algorithm , P, 1 && algorithmLen == 1) == 0 ){
		completed = Priority(pcb, NULL);
	}else if(strncmp(algorithm, RR, 2) == 0 && algorithmLen == 2){
		char extra;
		if(sscanf(argv[3], "%zu%c", &quant, &extra) != 1) return EXIT_FAILURE;
		completed = round_robin(pcb , NULL, quant);
	}else if(strncmp(algorithm, SJF, 3) == 0 && algorithmLen == 3){
		completed = shortest_job_first(pcb, NULL);
	}else if(strncmp(algorithm, SRT, 3) == 0 && algorithmLen == 3){
		completed = shortest_remaining_time_first(pcb, NULL);
	}else if(strncmp(algorithm, FCFS, 4) == 0 && algorithmLen == 4){
		completed = first_come_first_serve(pcb, NULL);
	}else{
		fprintf(stderr, "Unknown scheduling algorithm: %s\n", algorithm);
    	return EXIT_FAILURE;
	}
	


	//abort();		// REPLACE ME with implementation.

	if(completed) return EXIT_SUCCESS;
	return EXIT_FAILURE;
}
