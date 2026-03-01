#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include "gtest/gtest.h"
#include "../include/processing_scheduling.h"

// Using a C library requires extern "C" to prevent function mangling
extern "C"
{
#include <dyn_array.h>
#include <unistd.h>     
#include <errno.h>      

}

#define NUM_PCB 30
#define QUANTUM 5 // Used for Robin Round for process as the run time limit

/*
unsigned int score;
unsigned int total;

class GradeEnvironment : public testing::Environment
{
	public:
		virtual void SetUp()
		{
			score = 0;
			total = 210;
		}

		virtual void TearDown()
		{
			::testing::Test::RecordProperty("points_given", score);
			::testing::Test::RecordProperty("points_total", total);
			std::cout << "SCORE: " << score << '/' << total << std::endl;
		}
};
*/

TEST(FCFSTest, HandleEmptyQueue) {
	dyn_array_t *test_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t* result{};
	bool success = first_come_first_serve(test_queue, result);

	EXPECT_FALSE(success);

	dyn_array_destroy(test_queue);
}

TEST(FCFSTest, ThreeProcessesDifferentArrival) {
    dyn_array_t *queue = dyn_array_create(8, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(queue, nullptr);

    ProcessControlBlock_t p1 = { .remaining_burst_time = 5, .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t p2 = { .remaining_burst_time = 3, .priority = 0, .arrival = 2, .started = false };
    ProcessControlBlock_t p3 = { .remaining_burst_time = 6, .priority = 0, .arrival = 4, .started = false };

    ASSERT_TRUE(dyn_array_push_back(queue, &p1));
    ASSERT_TRUE(dyn_array_push_back(queue, &p2));
    ASSERT_TRUE(dyn_array_push_back(queue, &p3));

    ASSERT_EQ(dyn_array_size(queue), 3u);

    ScheduleResult_t result = { .average_waiting_time = 0.0f, .average_turnaround_time = 0.0f, .total_run_time = 0UL };

    bool success = first_come_first_serve(queue, &result);

    EXPECT_TRUE(success) << "FCFS should succeed on valid input";
    const float EPS = 0.001f;
    EXPECT_NEAR(result.average_waiting_time, 2.333333f, EPS);
    EXPECT_NEAR(result.average_turnaround_time, 7.0f, EPS);
    EXPECT_EQ(result.total_run_time, 14u);

    dyn_array_destroy(queue);
}

TEST(LoadPCBTest, LoadsCorrectNumberAndValues) {

   const char* test_file = "test_pcb.bin";
    FILE* fp = fopen(test_file, "wb");
    ASSERT_NE(fp, nullptr) << "Failed to create test file";

    uint32_t num_processes = 3;
    fwrite(&num_processes, sizeof(uint32_t), 1, fp);

    uint32_t p1[] = {8, 5, 0};   // burst=8, priority=5, arrival=0
    fwrite(p1, sizeof(uint32_t), 3, fp);
    uint32_t p2[] = {4, 2, 3};   // burst=4, priority=2, arrival=3
    fwrite(p2, sizeof(uint32_t), 3, fp);
    uint32_t p3[] = {5, 7, 10};  // burst=5, priority=7, arrival=10
    fwrite(p3, sizeof(uint32_t), 3, fp);

    fclose(fp);

    dyn_array_t* queue = load_process_control_blocks(test_file);
    ASSERT_NE(queue, nullptr) << "Failed to load PCBs from file";

    EXPECT_EQ(dyn_array_size(queue), 3u) << "Should load exactly 3 processes";

    ProcessControlBlock_t* pcb0 = (ProcessControlBlock_t*)dyn_array_at(queue, 0);
    ProcessControlBlock_t* pcb1 = (ProcessControlBlock_t*)dyn_array_at(queue, 1);
    ProcessControlBlock_t* pcb2 = (ProcessControlBlock_t*)dyn_array_at(queue, 2);

    ASSERT_NE(pcb0, nullptr);
    ASSERT_NE(pcb1, nullptr);
    ASSERT_NE(pcb2, nullptr);

    EXPECT_EQ(pcb0->arrival, 0u);
    EXPECT_EQ(pcb0->remaining_burst_time, 8u);
    EXPECT_EQ(pcb0->priority, 5u);
    EXPECT_FALSE(pcb0->started);

    EXPECT_EQ(pcb1->arrival, 3u);
    EXPECT_EQ(pcb1->remaining_burst_time, 4u);
    EXPECT_EQ(pcb1->priority, 2u);
    EXPECT_FALSE(pcb1->started);

    EXPECT_EQ(pcb2->arrival, 10u);
    EXPECT_EQ(pcb2->remaining_burst_time, 5u);
    EXPECT_EQ(pcb2->priority, 7u);
    EXPECT_FALSE(pcb2->started);

    dyn_array_destroy(queue);
    unlink(test_file);
}


TEST(LoadPCBTest, ReadsBinaryPCBFileCorrectly)
{
    // Creates a temp binary file 
    char pathTemplate[] = "/tmp/pcb_test_XXXXXX";
    int fd = mkstemp(pathTemplate);
    ASSERT_NE(fd, -1) << "mkstemp failed, errno=" << errno;

    // THis file data is the same as the pcb.bin file in the root
    const uint32_t data[] = {
        4,
        15, 3, 0,
        10, 1, 1,
        5,  2, 2,
        20, 4, 3
    };

    const uint8_t *bytes = reinterpret_cast<const uint8_t*>(data); // this tells the compiler to treat data as a uint8 array so it writes to the file correctly
    size_t toWrite = sizeof(data);
    size_t written = 0;

	// this writes to the temp file that gets tested
    while (written < toWrite) {
        ssize_t w = write(fd, bytes + written, toWrite - written);
        ASSERT_GT(w, 0) << "write failed, errno=" << errno;
        written += static_cast<size_t>(w);
    }

    close(fd);

    
    dyn_array_t *arr = load_process_control_blocks(pathTemplate);
    ASSERT_NE(arr, nullptr) << "load_process_control_blocks returned NULL";

    // makes sure count is 4
    ASSERT_EQ(dyn_array_size(arr), (size_t)4);

    // Cchecks the order to make sure it was pushed from front to back
    const ProcessControlBlock_t *p0 =
        (const ProcessControlBlock_t*)dyn_array_at(arr, 0);
    const ProcessControlBlock_t *p1 =
        (const ProcessControlBlock_t*)dyn_array_at(arr, 1);
    const ProcessControlBlock_t *p2 =
        (const ProcessControlBlock_t*)dyn_array_at(arr, 2);
    const ProcessControlBlock_t *p3 =
        (const ProcessControlBlock_t*)dyn_array_at(arr, 3);

	// makes sure none fo the pcb blocks are null
    ASSERT_NE(p0, nullptr);
    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);


	// checks expected values
    EXPECT_EQ(p0->remaining_burst_time, (uint32_t)15);
    EXPECT_EQ(p0->priority,            (uint32_t)3);
    EXPECT_EQ(p0->arrival,             (uint32_t)0);
    EXPECT_FALSE(p0->started);

    EXPECT_EQ(p1->remaining_burst_time, (uint32_t)10);
    EXPECT_EQ(p1->priority,            (uint32_t)1);
    EXPECT_EQ(p1->arrival,             (uint32_t)1);
    EXPECT_FALSE(p1->started);

    EXPECT_EQ(p2->remaining_burst_time, (uint32_t)5);
    EXPECT_EQ(p2->priority,            (uint32_t)2);
    EXPECT_EQ(p2->arrival,             (uint32_t)2);
    EXPECT_FALSE(p2->started);

    EXPECT_EQ(p3->remaining_burst_time, (uint32_t)20);
    EXPECT_EQ(p3->priority,            (uint32_t)4);
    EXPECT_EQ(p3->arrival,             (uint32_t)3);
    EXPECT_FALSE(p3->started);

    
    dyn_array_destroy(arr);
    unlink(pathTemplate); // destroys the temp file
}

TEST(SRTF, CheckEmptyQueue)
{
    dyn_array_t *test_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t* result{};
	bool success = shortest_remaining_time_first(test_queue, result);

	EXPECT_FALSE(success);

	dyn_array_destroy(test_queue);
}

TEST(SRTF, CorrectResultValues){
    dyn_array_t *queue = dyn_array_create(8, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(queue, nullptr);

    ProcessControlBlock_t p1 = { .remaining_burst_time = 15, .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t p2 = { .remaining_burst_time = 10, .priority = 0, .arrival = 1, .started = false };
    ProcessControlBlock_t p3 = { .remaining_burst_time = 5, .priority = 0, .arrival = 2, .started = false };
    ProcessControlBlock_t p4 = { .remaining_burst_time = 20, .priority = 0, .arrival = 3, .started = false };
    ASSERT_TRUE(dyn_array_push_back(queue, &p1));
    ASSERT_TRUE(dyn_array_push_back(queue, &p2));
    ASSERT_TRUE(dyn_array_push_back(queue, &p3));
    ASSERT_TRUE(dyn_array_push_back(queue, &p4));

    ASSERT_EQ(dyn_array_size(queue), 4u);

    ScheduleResult_t result = { .average_waiting_time = 0.0f, .average_turnaround_time = 0.0f, .total_run_time = 0UL };


    bool success = shortest_remaining_time_first(queue, &result);

    ASSERT_TRUE(success);

    EXPECT_EQ(result.average_waiting_time,               (float)11.75);
    EXPECT_EQ(result.average_turnaround_time,            (float)24.25);
    EXPECT_EQ(result.total_run_time,                     (unsigned long)50);
    
    dyn_array_destroy(queue);

}

TEST(PriorityTest, HigherPriorityRunsFirstSameArrival) {
    dyn_array_t* queue = dyn_array_create(8, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(queue, nullptr);

    // lower number = higher priority
    ProcessControlBlock_t p1 = { .remaining_burst_time = 8, .priority = 5, .arrival = 0, .started = false };
    ProcessControlBlock_t p2 = { .remaining_burst_time = 2, .priority = 2, .arrival = 0, .started = false }; // highest priority
    ProcessControlBlock_t p3 = { .remaining_burst_time = 6, .priority = 4, .arrival = 0, .started = false };

    ASSERT_TRUE(dyn_array_push_back(queue, &p1));
    ASSERT_TRUE(dyn_array_push_back(queue, &p2));
    ASSERT_TRUE(dyn_array_push_back(queue, &p3));
    ASSERT_EQ(dyn_array_size(queue), 3u);

    ScheduleResult_t result = {};
    bool success = priority(queue, &result);

    EXPECT_TRUE(success) << "priority() should succeed";

    // Execution order should be: p2 (pri=2) → p3 (pri=4) → p1 (pri=5)
    // Times:
    // p2: wait=0, turnaround=2
    // p3: wait=2, turnaround=2+6=8
    // p1: wait=8, turnaround=8+8=16

    const float EPS = 0.001f;
    EXPECT_NEAR(result.average_waiting_time, (0 + 2 + 8) / 3.0f, EPS);   // 20/3 = 3.333...
    EXPECT_NEAR(result.average_turnaround_time, (2 + 8 + 16) / 3.0f, EPS);  // 26/3 = 8.666...
    EXPECT_EQ(result.total_run_time, 16ul);

    dyn_array_destroy(queue);
}

TEST(PriorityTest, DifferentArrivalTimesAndPriorities) {
    dyn_array_t* queue = dyn_array_create(8, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(queue, nullptr);

    ProcessControlBlock_t p1 = { .remaining_burst_time = 5, .priority = 4, .arrival = 0, .started = false };
    ProcessControlBlock_t p2 = { .remaining_burst_time = 8, .priority = 2, .arrival = 2, .started = false };
    ProcessControlBlock_t p3 = { .remaining_burst_time = 3, .priority = 6, .arrival = 0, .started = false };
    ProcessControlBlock_t p4 = { .remaining_burst_time = 4, .priority = 4, .arrival = 1, .started = false };

    ASSERT_TRUE(dyn_array_push_back(queue, &p1));
    ASSERT_TRUE(dyn_array_push_back(queue, &p2));
    ASSERT_TRUE(dyn_array_push_back(queue, &p3));
    ASSERT_TRUE(dyn_array_push_back(queue, &p4));
    ASSERT_EQ(dyn_array_size(queue), 4u);

    ScheduleResult_t result = {};
    bool success = priority(queue, &result);
    EXPECT_TRUE(success) << "priority() should succeed";

    const float EPS = 0.001f;
    EXPECT_NEAR(result.average_waiting_time, (0 + 3 + 12 + 17) / 4.0f, EPS);
    EXPECT_NEAR(result.average_turnaround_time, (5 + 11 + 16 + 20) / 4.0f, EPS);
    EXPECT_EQ(result.total_run_time, 20ul);

    dyn_array_destroy(queue);
}

TEST(RoundRobinTest, SameArrivals) {
    dyn_array_t* queue = dyn_array_create(8, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(queue, nullptr);

    ProcessControlBlock_t p1 = { .remaining_burst_time = 10, .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t p2 = { .remaining_burst_time = 5,  .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t p3 = { .remaining_burst_time = 8,  .priority = 0, .arrival = 0, .started = false };

    ASSERT_TRUE(dyn_array_push_back(queue, &p1));
    ASSERT_TRUE(dyn_array_push_back(queue, &p2));
    ASSERT_TRUE(dyn_array_push_back(queue, &p3));
    ASSERT_EQ(dyn_array_size(queue), 3u);

    ScheduleResult_t result = {};
    size_t quantum = 4;
    bool success = round_robin(queue, &result, quantum);
    EXPECT_TRUE(success) << "round_robin should succeed";

    // Expected execution (quantum=4):
    // t=0-4:  p1 (rem 6) → back to queue
    // t=4-8:  p2 (rem 1) → back to queue
    // t=8-12: p3 (rem 4) → back to queue
    // t=12-16: p1 (rem 2) → back
    // t=16-17: p2 (rem 0) → finishes
    // t=17-21: p3 (rem 0) → finishes
    // t=21-23: p1 (rem 0) → finishes

    // Completion times: p2=17, p3=21, p1=23
    // Turnaround: p2=17-0=17, p3=21-0=21, p1=23-0=23
    // Wait times: TAT - burst
    // p1: 23-10 = 13
    // p2: 17-5  = 12
    // p3: 21-8  = 13

    const float EPS = 0.001f;
    EXPECT_NEAR(result.average_waiting_time, (13 + 12 + 13) / 3.0f, EPS);     // 38/3 ≈ 12.6667
    EXPECT_NEAR(result.average_turnaround_time, (23 + 17 + 21) / 3.0f, EPS);  // 61/3 ≈ 20.3333
    EXPECT_EQ(result.total_run_time, 23ul);  // total CPU time = sum of bursts = 23

    dyn_array_destroy(queue);
}

TEST(RoundRobinTest, DifferentArrivals) {
    dyn_array_t* queue = dyn_array_create(8, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(queue, nullptr);

    ProcessControlBlock_t p1 = { .remaining_burst_time = 8, .priority = 0, .arrival = 0, .started = false };
    ProcessControlBlock_t p2 = { .remaining_burst_time = 4, .priority = 0, .arrival = 2, .started = false };
    ProcessControlBlock_t p3 = { .remaining_burst_time = 9, .priority = 0, .arrival = 4, .started = false };

    ASSERT_TRUE(dyn_array_push_back(queue, &p1));
    ASSERT_TRUE(dyn_array_push_back(queue, &p2));
    ASSERT_TRUE(dyn_array_push_back(queue, &p3));
    ASSERT_EQ(dyn_array_size(queue), 3u);

    ScheduleResult_t result = {};
    size_t quantum = 3;
    bool success = round_robin(queue, &result, quantum);
    EXPECT_TRUE(success);

    // Expected schedule (quantum=3):
    // t=0-3: p1 (rem 5)
    // t=3-6: p1 (rem 2)   ← p2 not arrived yet
    // t=6-8: p1 (rem 0)   ← finishes p1
    // t=8-11: p2 (rem 1)  ← p2 arrived at 2, but waited until p1 finished
    // t=11-12: p2 (rem 0) → finishes p2
    // t=12-15: p3 (rem 6) ← p3 arrived at 4
    // t=15-18: p3 (rem 3)
    // t=18-21: p3 (rem 0) → finishes

    // Completion times: p1=8, p2=12, p3=21
    // Turnaround:
    // p1: 8-0 = 8
    // p2: 12-2 = 10
    // p3: 21-4 = 17
    // Wait = TAT - burst
    // p1: 8-8 = 0
    // p2: 10-4 = 6
    // p3: 17-9 = 8

    const float EPS = 0.001f;
    EXPECT_NEAR(result.average_waiting_time, (0 + 6 + 8) / 3.0f, EPS);     // 14/3 ≈ 4.6667
    EXPECT_NEAR(result.average_turnaround_time, (8 + 10 + 17) / 3.0f, EPS); // 35/3 ≈ 11.6667
    EXPECT_EQ(result.total_run_time, 21ul);  // 8+4+9 = 21

    dyn_array_destroy(queue);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	// ::testing::AddGlobalTestEnvironment(new GradeEnvironment);
	return RUN_ALL_TESTS();
}
