University of Victoria
Shuwen Li
V00024025

Part 1: How to run and build my code
    ---open the terminal
    ---use "make" to compile code
    ---use "./ACS customer.txt" to execute the code
    ---look the code and check the correctness of code

Part 2: The purpose of each function of my code
    ---main function: open file and read data. And this function also create clerk function and customer thread and join thread
    back to main function.
    ---customer function: unsleep to arrive time, and then add and remove customer to specific queue. And also, this function also
    include send and wait and receive signal.
    ---clerk function: Check if there are customers waiting in the two queues. And then for different condition there is different
     way to face. queue is empty -> keep check; queue is not empty -> send signal and wait
    ---servecustomer function: to make sure the specific clerk serve which customer
    ---add queue, remove queue and peek queue: help function -> help to add queue,remove queue and find the peak of queue easily.
Part 3: The overall purpose of this code
    I use the thread, mutex and conditional variable to simulate an airline check-in system. The check-in system includes 2 queues
     and 4 clerks.  One queue (Queue 0) for business class and the other (Queue 1) for economy class.

Part 4: Where I study material
    ---study fopen(),fgets(), atoi() from the connex sample material
    ---use and fix the old code from my code in SENG265 course
    ---study some method from An Airline Check-in System Notes & Tips ppt
    ---get some hint from powerpoint in connex
