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

	size_t processes = dyn_array_size(ready_queue);
	if (processes == 0) {
		result->average_waiting_time = 0;
		result->average_turnaround_time = 0;
		return true;
	}
	
	// types from ProcessControlBlock_t
	float cur_time = 0;
	float total_waiting_time = 0; 
	float total_turnaround_time = 0;
	unsigned long total_run_time = 0;

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

		total_run_time = completion_time; // should be last completion_time

		cur_time = completion_time; // needed to update cur_time to completion_time after each loop
	}

	result->average_waiting_time = total_waiting_time / processes;
	result->average_turnaround_time = total_turnaround_time / processes;
	result->total_run_time = total_run_time;
	
	return true;
}

//https://www.geeksforgeeks.org/dsa/program-for-shortest-job-first-or-sjf-cpu-scheduling-set-1-non-preemptive/
/// @brief Non-preimptive sjf that starts the process with the fastest time.
/// @param ready_queue dyn_array of processcontrolblock_t
/// @param result stat tracking
/// @return true if succeeded, otherwise false
bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	if (ready_queue == NULL || result == NULL) return false;
	if (dyn_array_empty(ready_queue)) return false;

	size_t processes = dyn_array_size(ready_queue);

	float cur_time = 0;
    float total_waiting_time = 0;
    float total_turnaround_time = 0;
    unsigned long total_run_time = 0;

	// bool ptr to keep track of the processes that complete
	bool *done = (bool*)malloc(sizeof(processes));
	for (size_t i = 0; i < processes; i++) { // initially false
		*(done + i) = false;
	}

	// for the while loop to see if we finished them all
	size_t finished_processes = 0;

	while (finished_processes < processes) {
		uint32_t shortest_burst = UINT32_MAX;
		int shortest_index = -1;

		for (size_t i = 0; i < processes; i++) {
			if (*(done + i)) continue; // if process is done it skips to next iteration

			ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);

			// helps with cpu idle
			if (pcb->arrival <= cur_time && pcb->remaining_burst_time < shortest_burst) {
				shortest_burst = pcb->remaining_burst_time;
				shortest_index = i;
			}
		}

		// there were no arrival times at cur_time
		if (shortest_index == -1) {
			cur_time++;
			continue;
		}

		ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, shortest_index);

		// calculations from geekforgeeks
		float waiting_time = cur_time - pcb->arrival;
        float completion_time = cur_time + pcb->remaining_burst_time;
        float turnaround_time = completion_time - pcb->arrival;

        total_waiting_time += waiting_time;
        total_turnaround_time += turnaround_time;

        cur_time = completion_time;
        total_run_time = completion_time;

		// most recent shortest_index updated to completed, so it skips in the for loop
        *(done + shortest_index) = true;
        finished_processes++;
	}

	// finds our avgs
	result->average_waiting_time = total_waiting_time / processes;
	result->average_turnaround_time = total_turnaround_time / processes;
	result->total_run_time = total_run_time;

	return true;
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
	if(*input_file == '\n' || *input_file == '\0') return NULL;

	int fd = open(input_file, O_RDONLY);
	if (fd == -1) return NULL;

	size_t totalRead = 0; //total amount of bytes read so far
	uint32_t n; // the number of PCB structs needed to read in
	while (totalRead < sizeof(uint32_t)) {
		ssize_t readIn = read(fd, ((uint8_t*)&n) + totalRead, sizeof(uint32_t) - totalRead);
		if (readIn <= 0) { close(fd); return NULL; }
		totalRead += (size_t)readIn; 
	}
	//                                          0 minimum cappacity , sizeof(struct) , no destructor
	dyn_array_t* dynamicArray = dyn_array_create(0 , sizeof(ProcessControlBlock_t), NULL);
	if(!dynamicArray) return NULL;

	//bool dyn_array_push_front(dyn_array_t *const dyn_array, const void *const object);
	size_t sectionNum = 0;

	uint32_t burstTime;
	uint32_t priority;
	uint32_t arrivalTime;

	uint32_t data;
	size_t readBytes = 0;

	totalRead = 0;
	size_t totalBytes = 3 * n;
	while(totalRead < totalBytes){

		ssize_t readIn = read(fd, ((uint8_t*)&data) + readBytes, sizeof(uint32_t) - readBytes);
		if (readIn <= 0) { close(fd); return NULL; }
		readBytes += (size_t)readIn; 

		if(readBytes == 4){
			switch(sectionNum)
			{
				case 0:
					burstTime = data;
					break;
				case 1:
					priority = data;
					break;
				case 2: 
					arrivalTime = data;
					break;
				default:
					return NULL;
			}
			readBytes = 0;
			sectionNum++;
			totalRead ++;
		}

		if(sectionNum == 3){
			ProcessControlBlock_t pcb;
			pcb.remaining_burst_time = burstTime;
			pcb.priority = priority;
			pcb.arrival = arrivalTime;
			pcb.started = false;
			sectionNum = 0;

			if(!dyn_array_push_back(dynamicArray , &pcb)) return NULL;
		}


	}

	//ProcessControlBlock_t *pcb; // the file is containing PCBs

	


	close(fd);
	// UNUSED(input_file);
	// return NULL;
	return dynamicArray;
}

// Runs the preemptive Shortest Remaining Time First Process Scheduling algorithm over the incoming ready_queue
// \param ready queue a dyn_array of type ProcessControlBlock_t that contain be up to N elements
// \param result used for shortest job first stat tracking \ref ScheduleResult_t
// \return true if function ran successful else false for an error
// There is no guarantee that the passed dyn_array_t will be the result of your implementation of load_process_control_blocks
bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if (!ready_queue || !result) return false;
    if (dyn_array_empty(ready_queue)) return false;

    size_t array_size = dyn_array_size(ready_queue);

    // original burst and arrival times for the function
    uint32_t *original_burst_times   = calloc(array_size, sizeof(uint32_t));
    uint32_t *original_arrival_times = calloc(array_size, sizeof(uint32_t));
    if (!original_burst_times || !original_arrival_times) {
        free(original_burst_times);
        free(original_arrival_times);
        return false;
    }

	// set orignal values to the pcb values initally
    for (size_t i = 0; i < array_size; i++) {
        ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);
        if (!pcb) { 
            free(original_burst_times);
            free(original_arrival_times);
            return false; 
        }
        original_burst_times[i]   = pcb->remaining_burst_time;
        original_arrival_times[i] = pcb->arrival;
        pcb->started = false;
    }

    float total_waiting_time = 0.0f;
    float total_turnaround_time = 0.0f;
    unsigned long clock = 0;

	// loops until the array is empty
    while (!dyn_array_empty(ready_queue)) 
    {
        size_t curr_size = dyn_array_size(ready_queue);

        // found an index with 0 arrival time
        bool found_arrived = false;
        size_t best_index = 0; // the index with arrival time 0 and the least burst time
        uint32_t best_remaining = 0; // the burst time for ht ebest index

		// finds the index with the lowest burst time that has arrival time = 0
        for (size_t i = 0; i < curr_size; i++) {
            ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);
            if (!pcb) { // one of the dynamic arrays are null return false
                free(original_burst_times);
                free(original_arrival_times);
                return false;
            }

            if (pcb->arrival == 0) {
                if (!found_arrived || pcb->remaining_burst_time < best_remaining) {
                    found_arrived = true;
                    best_index = i;
                    best_remaining = pcb->remaining_burst_time;
                }
            }
        }

        // if nothing has arrived yet, CPU is idle for 1 tick
        if (!found_arrived) {
            for (size_t i = 0; i < curr_size; i++) {
                ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);
                if (!pcb) {
                    free(original_burst_times);
                    free(original_arrival_times);
                    return false;
                }
                if (pcb->arrival > 0) pcb->arrival--;
            }
            clock++;
            continue; // jumps to top of the while loop
        }

        
        ProcessControlBlock_t *curr = (ProcessControlBlock_t*)dyn_array_at(ready_queue, best_index);
        if (!curr) {
            free(original_burst_times);
            free(original_arrival_times);
            return false;
        }

        curr->started = true;
        virtual_cpu(curr);
        clock++;

        // decrement arrival time for proccess with arrival > 0
        for (size_t i = 0; i < curr_size; i++) {
            ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);
            if (!pcb) {
                free(original_burst_times);
                free(original_arrival_times);
                return false;
            }
            if (pcb->arrival > 0) pcb->arrival--;
        }

        // if the proccess has finished
        if (curr->remaining_burst_time == 0) 
        {
		
            uint32_t turnaround = (uint32_t)(clock - original_arrival_times[best_index]); // the turnaround for the proccess
            uint32_t waiting    = turnaround - original_burst_times[best_index]; // the waiting time for the proccess 

            total_turnaround_time += (float)turnaround;
            total_waiting_time    += (float)waiting;

			// erases the index of the finished proccess
            if (!dyn_array_erase(ready_queue, best_index)) {
                free(original_burst_times);
                free(original_arrival_times);
                return false;
            }

            // shifts hte original arrays since the removed pcb object is no longer in the dynamic array
            for (size_t j = best_index; j + 1 < curr_size; j++) {
                original_burst_times[j]   = original_burst_times[j + 1];
                original_arrival_times[j] = original_arrival_times[j + 1];
            }
        }
    }

    result->average_waiting_time = total_waiting_time / (float)array_size;
    result->average_turnaround_time = total_turnaround_time / (float)array_size;
    result->total_run_time = clock;

    free(original_burst_times);
    free(original_arrival_times);
    return true;
}
	