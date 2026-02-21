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

TEST(LoadPCBTest, HandleNullFile) {
	dyn_array_t *success = load_process_control_blocks(nullptr);

	EXPECT_EQ(success, nullptr);
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




int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	// ::testing::AddGlobalTestEnvironment(new GradeEnvironment);
	return RUN_ALL_TESTS();
}
