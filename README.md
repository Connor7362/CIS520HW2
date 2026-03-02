# Project 2

Milestone 0: CMake and Unit Testing filled out

Milestone 1: PCB file loading and First Come First Served

Milestone 2: Shortest Job First, Shortest Remaining Time First, Round Robin, and analysis of algorithms

Note:
You can manually copy the time analysis from console and paste it to this file, but directly output from your program is strongly recommended.
---------------------------------------------------------------------------
Add your scheduling algorithm analysis below this line in a readable format.
---------------------------------------------------------------------------



NOTE** the command time ./analysis ../pcb.bin <algorithm> [quantum]     <--- will always work since pcb is in parent 
NOTE** the command time ./analysis pcb.bin <algorithm> [quantum]     <--- REQUIRES pcb.bin to be in the build directory
First Come First Serve --------------------------

Average waiting time: 16.00
Average turnaround time: 28.50
Total run time: 50

real    0m0.041s
user    0m0.001s
sys     0m0.005s

Shortest Job First ------------------------------------

Average waiting time: 14.75
Average turnaround time: 27.25
Total run time: 50

real    0m0.004s
user    0m0.000s
sys     0m0.003s

Priority -----------------------------------------------------

Average waiting time: 16.00
Average turnaround time: 28.50
Total run time: 50

real    0m0.012s
user    0m0.001s
sys     0m0.005s

Round Robin ----------------------------------------------------
THIS IS RUN WITH QUANTUM 4 AS IN THE EXAMPLE
[connor72@icr-clymene build]$ time ./analysis pcb.bin RR 4
Average waiting time: 24.00
Average turnaround time: 36.50
Total run time: 50

real    0m0.004s
user    0m0.000s
sys     0m0.004s

Shortest Remaining Time First ---------------------------------

Average waiting time: 11.75
Average turnaround time: 24.25
Total run time: 50

real    0m0.018s
user    0m0.000s
sys     0m0.006s
