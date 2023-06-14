#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS

// data structure use PCB
typedef struct process_control_block {
    int pid;
    double priority;
    int arrival_time;
    int burst_time;
    int original_burst_time;
    int start_time;
    int end_time;
    int waiting_time;
    int response_time;
    int turnaround_time;
    int remain_time;
    int update_priority;
}pcb;

//Make a job queue with array
pcb job_queue[100];
int job_queue_size = 0;

//Make a ready queue with linked-list
typedef struct Node {
    pcb* process;
    struct Node* next;
}node;

node* ready_queue_head = NULL;
node* ready_queue_tail = NULL;

// read data and add to job queue
void add_job(char* in_filename) {
    int pid, priority, arrival_time, burst_time;
    FILE* input_file = fopen(in_filename, "r");

    if (input_file == NULL) {
        printf("Could not open file\n");
        exit(1);
    }

    while (fscanf(input_file, "%d %d %d %d", &pid, &priority, &arrival_time, &burst_time) == 4) {
        if (job_queue_size >= sizeof(job_queue) / sizeof(job_queue[0])) {
            printf("Job queue is full. Increase the size of the job_queue array.\n");
            exit(1);
        }

        pcb data = { pid, priority, arrival_time, burst_time, burst_time, 0 };
        job_queue[job_queue_size++] = data;
    }

    fclose(input_file);
}


// add to ready queue
void add_to_ready(pcb* process) {
    // create a new node for the process
    node* new_node = (node*)malloc(sizeof(node));
    new_node->process = process;
    new_node->next = NULL;

    // if the ready queue is empty, make the new node the head
    if (ready_queue_head == NULL) {
        ready_queue_head = new_node;
        ready_queue_tail = new_node;
    }
    // if not empty, add the new node to the end
    else {
        ready_queue_tail->next = new_node;
        ready_queue_tail = new_node;
    }
}

// sort in FCFS order and add to ready queue
void sort_fcfs() {
    /* sort processes in FCFS order */
    int i, j;
    pcb temp;
    // process with less arrival time are sorted in front
    for (i = 0; i < job_queue_size - 1; i++) {
        for (j = i + 1; j < job_queue_size; j++) {
            if (job_queue[i].arrival_time > job_queue[j].arrival_time) {
                temp = job_queue[i];
                job_queue[i] = job_queue[j];
                job_queue[j] = temp;
            }
        }
    }

    /* add the processes from job queue to ready queue */
    int k;
    for (k = 0; k < job_queue_size; k++) {
        add_to_ready(&job_queue[k]);
    }
}

// return sum of all of process burst time
int total_burst() {
    int total_burst_time = 0;
    for (int i = 0; i < 10; i++) {
        total_burst_time += job_queue[i].burst_time;
    }
    return total_burst_time;
}

//RR scheduling
void rr(int* tum, char* out_filename) {
    sort_fcfs(); //sort processes in FCFS order
    int current_time = 0;
    int total_burst_time = total_burst();
    int total_waiting_time = 0;
    int total_response_time = 0;
    int total_turnaround_time = 0;
    int remain_num = 10;
    int quantum = *tum;

    FILE* output_file = fopen(out_filename, "w");
    // fopen returns 0, fail to open correctly
    if (output_file == NULL) {
        printf("Could not open file\n");
        exit(1);
    }
    fprintf(output_file, "Scheduling : RR\n");
    fprintf(output_file, "=====================================\n");

    while (remain_num > 0) { //when scheduling is not over
        node* current_node = ready_queue_head;
        while (current_node != NULL) {
            pcb* current_process = current_node->process;
            // if process is not arrive, print idle
            while (current_process->arrival_time > current_time) {
                fprintf(output_file, "<time %d> ---- system is idle ----\n", current_time);
                current_time++;
            }
            /* 1. When the burst time is less than a quantum */
            if (current_process->burst_time <= quantum) {
                current_process->start_time = current_time;
                // When the process is first run, response time store
                if (current_process->burst_time == current_process->original_burst_time) {
                    current_process->response_time = current_process->start_time - current_process->arrival_time;
                    total_response_time += current_process->response_time;
                }
                current_process->end_time = current_process->start_time + current_process->burst_time;
                // store turnaround time, and calculate total turnaround time
                current_process->turnaround_time = current_process->end_time - current_process->arrival_time;
                total_turnaround_time += current_process->turnaround_time;
                // store waiting time, and calculate total waiting time
                current_process->waiting_time = current_process->turnaround_time - current_process->original_burst_time;
                total_waiting_time += current_process->waiting_time;
                // exclude finished process
                remain_num--;

                /* Run process until end time */
                while (current_time < current_process->end_time) {
                    // when other process is arrived, print
                    for (int k = 0; job_queue[k].arrival_time <= current_time; k++) {
                        if (job_queue[k].arrival_time == current_time) {
                            fprintf(output_file, "<time %d> [new arrival] process %d\n", current_time, job_queue[k].pid);
                        }
                    }
                    fprintf(output_file, "<time %d> process %d is running\n", current_time, current_process->pid);
                    current_time++;
                }

                /* print message that running is over */
                if (current_node->next != NULL) {
                    //not end of process, print message context switch 
                    fprintf(output_file, "<time %d> process %d is finished\n", current_time, current_process->pid);
                    fprintf(output_file, "-------------------------- (Context-Switch)\n");
                }
                else {
                    //end of process, print message all finish
                    fprintf(output_file, "<time %d> all processes finish\n", current_time);
                }
                current_node = current_node->next; //move to next process
            }

            /* 2. When the burst time is over a quantum */
            else {
                current_process->start_time = current_time;
                // When the process is first run
                if (current_process->burst_time == current_process->original_burst_time) {
                    current_process->response_time = current_process->start_time - current_process->arrival_time;
                    total_response_time += current_process->response_time;
                }
                current_process->end_time = current_process->start_time + quantum;//run as many quantum
                current_process->remain_time = current_process->burst_time - quantum; //save the remaining burst time after execution
                /* Run a process for quantum time */
                while (current_time < current_process->end_time) {
                    // when other process is arrived, print
                    for (int k = 0; job_queue[k].arrival_time <= current_time; k++) {
                        if (job_queue[k].arrival_time == current_time) {
                            fprintf(output_file, "<time %d> [new arrival] process %d\n", current_time, job_queue[k].pid);
                        }
                    }
                    fprintf(output_file, "<time %d> process %d is running\n", current_time, current_process->pid);
                    current_time++;
                }

                /* print message that running is over */
                if (current_node->next != NULL) {
                    //not end of process, print message context switch 
                    fprintf(output_file, "<time %d> process %d is finished\n", current_time, current_process->pid);
                    fprintf(output_file, "-------------------------- (Context-Switch)\n");
                }
                else {
                    //end of process, print message all finish
                    fprintf(output_file, "<time %d> all processes finish\n", current_time);
                }
                current_process->end_time = current_time;
                add_to_ready(current_process); //add unfinished processes to ready queue
                current_process->burst_time = current_process->remain_time; //update burst time with remaining time
                ready_queue_head = current_node->next; // update ready_queue_head to the next process

                current_node = current_node->next; // move to the next process
            }
        }
    }
    fprintf(output_file, "=================================================\n");

    /* calculate average data to print */
    double avg_cpu_usage = (double)total_burst_time / current_time * 100;
    double avg_waiting_time = (double)total_waiting_time / 10;
    double avg_response_time = (double)total_response_time / 10;
    double avg_turnaround_time = (double)total_turnaround_time / 10;

    /*print in the output txt file*/
    fprintf(output_file, "Average CPU usage: %.2f%%\n", avg_cpu_usage);
    fprintf(output_file, "Average waiting time: %.1f\n", avg_waiting_time);
    fprintf(output_file, "Average response time: %.1f\n", avg_response_time);
    fprintf(output_file, "Average turnaround time: %.1f\n", avg_turnaround_time);

    fclose(output_file);
}



int main(int argc, char* argv[])
{
    if (argc != 5) /* argc should be 5 for correct execution */
    {
        /* print argv[0] assuming it is the program name */
        printf("usage: %s [input_filename] [output_filename] [time_quantum_for_RR] [alpha_for_PRIO]", argv[0]);
    }
    else /* when argc is 5(correct) */
    {
        // Save input, outfile name
        char* input_file = argv[1];
        char* output_file = argv[2];

        // convert data type to int and double
        int time_quantum_ptr = atoi(argv[3]);
        int* time_quantum = &time_quantum_ptr;
        double alpha_ptr = atof(argv[4]);
        double* alpha = &alpha_ptr;

        add_job(input_file);

        //Round Robin(RR)
        rr(time_quantum, output_file);
    }
}