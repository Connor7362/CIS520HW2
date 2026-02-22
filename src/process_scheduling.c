#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dyn_array.h"
#include "processing_scheduling.h"


// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
	// decrement the burst time of the pcb
	--process_control_block->remaining_burst_time;
}


//https://www.geeksforgeeks.org/dsa/first-come-first-serve-cpu-scheduling-non-preemptive/ for help
bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	if (ready_queue == NULL || result == NULL) return false;

	size_t processes = dyn_array_data_size(ready_queue);
	if (processes == 0) {
		result->average_waiting_time = 0;
		result->average_turnaround_time = 0;
		return true;
	}
	
	// types from ProcessControlBlock_t
	float cur_time = 0;
	float total_waiting_time = 0; 
	float total_turnaround_time = 0;

	for (size_t i = 0; i < processes; i++) {
		ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i); // puts process i at pcb

		if (cur_time < pcb->arrival) {
			cur_time = pcb->arrival;
		}

		// found calculation on geeksforgeeks
		float waiting_time = cur_time - pcb->arrival;
		float completion_time = cur_time + pcb->remaining_burst_time;
		float turnaround_time = completion_time - pcb->arrival;

		// we calculate averages outside the for loop
		total_waiting_time += waiting_time;
		total_turnaround_time += turnaround_time;
	}

	result->average_waiting_time = total_waiting_time / processes;
	result->average_turnaround_time = total_turnaround_time / processes;
	result->total_run_time = (unsigned long)total_turnaround_time;
	
	return true;
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	UNUSED(quantum);
	return false;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
	if (input_file == NULL) return NULL;

	int fd = open(input_file, O_RDONLY);
	if (fd == -1) return NULL;

	ProcessControlBlock_t *pcb; // the file is containing PCBs

	close(fd);
	// UNUSED(input_file);
	// return NULL;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}
