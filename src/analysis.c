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
	size_t  algorithmLen = strnlen(algorithm, 5);
	bool completed = false;
	size_t quant = 0;
	ScheduleResult_t result;

	if(strncmp(algorithm , P, 1) && algorithmLen == 1 == 0 ){
		completed = priority(pcb, &result);
	}else if(strncmp(algorithm, RR, 2) == 0 && algorithmLen == 2){
		char extra;
		if(sscanf(argv[3], "%zu%c", &quant, &extra) != 1) return EXIT_FAILURE;
		completed = round_robin(pcb , &result, quant);
	}else if(strncmp(algorithm, SJF, 3) == 0 && algorithmLen == 3){
		completed = shortest_job_first(pcb, &result);
	}else if(strncmp(algorithm, SRT, 3) == 0 && algorithmLen == 3){
		completed = shortest_remaining_time_first(pcb, &result);
	}else if(strncmp(algorithm, FCFS, 4) == 0 && algorithmLen == 4){
		completed = first_come_first_serve(pcb, &result);
	}else{
		fprintf(stderr, "Unknown scheduling algorithm: %s\n", algorithm);
    	return EXIT_FAILURE;
	}
	


	//abort();		// REPLACE ME with implementation.

	if(completed){
		printf("Average waiting time: %.2f\n", result.average_waiting_time);
   	 	printf("Average turnaround time: %.2f\n", result.average_turnaround_time);
   	 	printf("Total run time: %lu\n", result.total_run_time);

		 return EXIT_SUCCESS;
	}
	
	return EXIT_FAILURE;
}
