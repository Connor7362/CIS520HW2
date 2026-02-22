#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include "gtest/gtest.h"
#include "../include/processing_scheduling.h"

// Using a C library requires extern "C" to prevent function mangling
extern "C"
{
#include <dyn_array.h>
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
    dyn_array_t *queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(queue, nullptr);

    ProcessControlBlock_t p1 = { .arrival = 0,  .remaining_burst_time = 5, .priority = 0 };
    ProcessControlBlock_t p2 = { .arrival = 2,  .remaining_burst_time = 3, .priority = 0 };
    ProcessControlBlock_t p3 = { .arrival = 4,  .remaining_burst_time = 6, .priority = 0 };

    dyn_array_push_back(queue, &p1);
    dyn_array_push_back(queue, &p2);
    dyn_array_push_back(queue, &p3);

    ScheduleResult_t result = {0};

    bool success = first_come_first_serve(queue, &result);
    EXPECT_TRUE(success) << "FCFS should succeed on valid input";

    // Manual calculation:
    //   P1: start  0 -> finish  5    wait = 0     TAT = 5
    //   P2: start  5 -> finish  8    wait = 5-2=3 TAT = 8-2=6
    //   P3: start  8 -> finish 14    wait = 8-4=4 TAT =14-4=10

    // Averages:
    //   Avg Wait = (0 + 3 + 4) / 3 = 7/3 ? 2.333...
    //   Avg TAT  = (5 + 6 +10) / 3 = 21/3 = 7.0

    const float EPS = 0.001f;

    EXPECT_NEAR(result.average_waiting_time,   2.333333f, EPS);
    EXPECT_NEAR(result.average_turnaround_time, 7.0f,      EPS);
    EXPECT_EQ  (result.total_run_time,         14u);

    dyn_array_destroy(queue);
}

TEST(LoadPCBTest, HandleNullFile) {
	dyn_array_t *success = load_process_control_blocks(nullptr);

	EXPECT_EQ(success, nullptr);
}

TEST(LoadPCBTest, LoadsCorrectNumberAndValues) {

    printf "\x00\x00\x00\x00\x08\x00\x00\x00\x05\x00\x00\x00\
    \x03\x00\x00\x00\x04\x00\x00\x00\x02\x00\x00\x00\
    \x0A\x00\x00\x00\x05\x00\x00\x00\x07\x00\x00\x00" > test_pcb.bin

    const char* test_file = "test_pcb.bin";

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

    EXPECT_EQ(pcb1->arrival, 3u);
    EXPECT_EQ(pcb1->remaining_burst_time, 4u);
    EXPECT_EQ(pcb1->priority, 2u);

    EXPECT_EQ(pcb2->arrival, 10u);
    EXPECT_EQ(pcb2->remaining_burst_time, 5u);
    EXPECT_EQ(pcb2->priority, 7u);

    dyn_array_destroy(queue);
}


int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	// ::testing::AddGlobalTestEnvironment(new GradeEnvironment);
	return RUN_ALL_TESTS();
}
