#include<unistd.h>
#include<sys/syscall.h>
#include"include/common.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<time.h>

#define __USE_GNU
#include<sched.h>

#define SIGUSR3 SIGXFSZ
#include<signal.h>

#define MAX_PROC 10

typedef struct{
    char name[64]; 
    int R, T;
    // For: take turns, burst counts, check 1st-time, check running:
    int turn, cnt, first, running;
    // For: record theoretical units (not necessary):
    int other_cnt, start_cnt, end_cnt;
    pid_t pids;
}Process;

Process *proc; // Information of each child.
char policy[10]; 
int N, arrived = -1;
int RR_Queue[MAX_PROC]; // RR

void Unit(){
    volatile unsigned long i;
    for(i = 0; i < 1000000UL; i++);
    return;
}

static void sig_usr(int signo){
    // A child exit / pause.
    if(signo == SIGUSR1){
        /* DO NOT DECLARE VARIABLES IN A SWITCH STATEMENT */
        int i, p; // Iterator
        int min_T = 1e9, min_idx, remain; // SJF && PSJF
	int RR_now, RR_new, RR_idx = -1, RR_newQueue[10]; // RR
        switch(policy[0]){
            // FIFO
            case 'F': 
                for(i = 0; i <= arrived; i++){
                    if(proc[i].cnt < proc[i].T){
		        // Get start_cnt
		        for(p = 0; p <= arrived; p++)
			    if(proc[i].start_cnt < proc[p].end_cnt)
			        proc[i].start_cnt = proc[p].end_cnt;
		    
		        while(proc[i].running == 0) 
			    kill(proc[i].pids, SIGUSR2);
		        break;
                    }
                }
                break;
	    
	    // RR
	    case 'R':
	        RR_now = RR_Queue[0];
                // Round each process.
                for(i = 1; i <= arrived; i++){
                    if(proc[RR_Queue[i]].cnt < proc[RR_Queue[i]].T){
		        RR_idx = i;
		        RR_new = RR_Queue[i];
		        break;
		    }
                }
	        if(RR_idx != -1){
		    // Update queue
		    for(i = 0; i <= arrived; i++){
		        RR_newQueue[i] = RR_Queue[RR_idx];
			RR_idx = (RR_idx + 1) % (arrived + 1);
		    }
		    for(i = 0; i <= arrived; i++)
		        RR_Queue[i] = RR_newQueue[i];

		    // Get start_cnt
		    if(proc[RR_new].first == 1){
		        for(p = 0; p <= arrived; p++)
		            if(proc[RR_new].start_cnt < proc[p].end_cnt)
			        proc[RR_new].start_cnt = proc[p].end_cnt;
		    }
	    
		    while(proc[RR_new].running == 0) 
		        kill(proc[RR_new].pids, SIGUSR2);
	        }else if(proc[RR_now].cnt < proc[RR_now].T){
		    while(proc[RR_now].running == 0) 
		        kill(proc[RR_now].pids, SIGUSR2);
		}
	        break;
            
            // SJF & PSJF
            case 'S': case 'P':
                for(i = 0; i <= arrived; i++){
                    if(proc[i].cnt < proc[i].T){
                        remain = proc[i].T - proc[i].cnt;
                        if(min_T > remain){
                            min_T = remain;
                            min_idx = i;
                        }
                    }
                }
                if(min_T != 1e9){
                    // Get start_cnt
                    if(proc[min_idx].first == 1)
                        for(p = 0; p <= arrived; p++)
                            if(proc[min_idx].start_cnt < proc[p].end_cnt)
                                proc[min_idx].start_cnt = proc[p].end_cnt;
                    while(proc[min_idx].running == 0)
                        kill(proc[min_idx].pids, SIGUSR2);
                }
                break;
        }
    }
    // Wake a child to run.
    else if(signo == SIGUSR2) proc[arrived].turn = 1;
    // Let a child pause.
    else if(signo == SIGUSR3) proc[arrived].turn = 0;
    return;
}

void PRINT_THEORETICAL(){
    FILE *fptr = fopen("Theo_unit.txt", "a");
    fprintf(fptr, "%d %s %d %d\n", getpid(), proc[arrived].name, proc[arrived].start_cnt, proc[arrived].end_cnt);
    fclose(fptr);
    return;
}
    
int compare(const void *a, const void *b){
    Process *A = (Process*)a;
    Process *B = (Process*)b;
    if(A->R != B->R) return (A->R - B->R);
    else{
        switch(policy[0]){
            // FIFO & RR
            case 'F': case 'R':
                return (atoi(&(A->name[1])) - atoi(&(B->name[1])));

            // SJF & PSJF
            case 'S': case 'P':
                return (A->T - B->T);
        }
    }
}

int main(){
    // Affinity (0)
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask); 
    sched_setaffinity(0, sizeof(mask), &mask);

    // Signal
    signal(SIGUSR1, sig_usr);
    signal(SIGUSR2, sig_usr);
    signal(SIGUSR3, sig_usr);

    // Create shared memory.
    scanf("%s%d", policy, &N);
    key_t key = 10023; // Just a casual number
    int shmid = shmget(key, N * sizeof(Process), IPC_CREAT | 0666);
    proc = shmat(shmid, NULL, 0);

    // Input
    for(int i = 0; i < N; i++){
        scanf("%s%d%d", proc[i].name, &proc[i].R, &proc[i].T);
        proc[i].turn = proc[i].cnt = proc[i].running = 0;
        proc[i].first = 1;
        proc[i].other_cnt = proc[i].start_cnt = proc[i].end_cnt = 0;
    }

    qsort(proc, N, sizeof(Process), compare);
    int cnt = 0; // Time unit.
    
    // Affinity (1) (for child) (line 232)
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    
    for(int i = 0; i < N; i++){
        // Ready to run.
        while(cnt < proc[i].R){
            Unit(); cnt++;
        }
        arrived++; // A process has arrived.
	RR_Queue[arrived] = i;
        
        int j; // Iterator
        int remain, preempted = -1; // PSJF
        switch(policy[0]){
            // FIFO & RR & SJF
            case 'F': case 'R': case 'S':
                for(j = 0; j <= arrived; j++){
                    if(j == arrived){
                        proc[j].turn = 1;
                        proc[j].start_cnt = proc[j].R; // Get start_cnt.
                    }
                    if(proc[j].turn == 1) break;
                }
                break;
            
            // PSJF
            case 'P':
                for(j = 0; j <= arrived; j++){
                    if(j == arrived){
                        proc[j].turn = 1;
                        proc[j].start_cnt = proc[j].R; // Get start_cnt.
                    }
                    remain = proc[j].T - proc[j].cnt;
                    if(remain > 0 && remain <= proc[arrived].T) break;
                    if(proc[j].turn == 1) preempted = j;
                }
                
                if(preempted != -1){
                    // Preempt
                    proc[preempted].running = 0;
                    kill(proc[preempted].pids, SIGUSR3); 
                }
                break;
        }

        pid_t tmp = fork();        
        if(tmp == 0){
            sched_setaffinity(0, sizeof(mask), &mask); // Affinity
            
            long start_time, end_time;
            // Running
            while(1){
                // A paused process would not be context switched.
                // => No need to call sched_setscheduler().
                if(proc[arrived].turn == 0) pause(); // Wait to be waken.
                proc[arrived].running = 1; // Make sure to be waken up.
                
                int now = proc[arrived].cnt; // RR

                if(proc[arrived].first == 1){
                    proc[arrived].first = 0;
                    start_time = syscall(451); // Get start_time (1st executed).
                    for(int p = 0; p <= N; p++) proc[arrived].other_cnt += proc[p].cnt;
                }

                switch(policy[0]){
                    // FIFO & SJF && PSJF
                    case 'F': case 'S': case 'P':
                        while(proc[arrived].cnt < proc[arrived].T 
                              && proc[arrived].turn == 1){
                            Unit();
                            proc[arrived].cnt++;
                        }
                        break;
                    
                    // RR
                    case 'R':
                        while(proc[arrived].cnt < proc[arrived].T
                              && proc[arrived].cnt < now + 500){
                            Unit();
                            proc[arrived].cnt++;
                        }
                        break;
                }

                // Update end_cnt.
                int tmp = 0;
                for(int p = 0; p <= N; p++) tmp += proc[p].cnt;
                proc[arrived].end_cnt = proc[arrived].start_cnt + (tmp - proc[arrived].other_cnt);
                
                // RR
                if(policy[0] == 'R' && proc[arrived].cnt >= now + 500 && proc[arrived].cnt < proc[arrived].T){
                    proc[arrived].turn = 0;
                    proc[arrived].running = 0;
                    kill(getppid(), SIGUSR1);
                }
                if(proc[arrived].cnt >= proc[arrived].T) break; // Burst finished.
            }
           
            // End of a child.
            proc[arrived].turn = 0;
            fprintf(stdout, "%s %d\n", proc[arrived].name, proc[arrived].pids);
            fflush(stdout);
            end_time = syscall(451);
	    PRINT_THEORETICAL();
            syscall(452, pid, start_time, end_time);
	    kill(getppid(), SIGUSR1); // Run another process.
            exit(0);    
        }else proc[arrived].pids = tmp;
    }
    
    for(int i = 0; i < N; i++) wait(NULL);

    // Delete shared memory.
    shmdt(proc);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
