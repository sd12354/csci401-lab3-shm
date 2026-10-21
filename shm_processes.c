// shm_processes.c
// Lab 3 Part 2 - Dear Old Dad / Poor Student with shared memory
// BankAccount = ShmPTR[0], Turn = ShmPTR[1]  (0 = dad, 1 = student)
// Sameer Dhanda

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

void DearOldDad(int *ShmPTR);
void PoorStudent(int *ShmPTR);

int main()
{
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    // get shared memory for 2 ints
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }
    printf("Server has received a shared memory of two integers...\n");

    ShmPTR = (int *) shmat(ShmID, NULL, 0);
    if ((long) ShmPTR == -1) {
        printf("*** shmat error (server) ***\n");
        exit(1);
    }
    printf("Server has attached the shared memory...\n");

    ShmPTR[0] = 0;   // BankAccount
    ShmPTR[1] = 0;   // Turn
    printf("Server has filled 0 and 0 in shared memory...\n");

    printf("Server is about to fork a child process...\n");
    pid = fork();
    if (pid < 0) {
        printf("*** fork error (server) ***\n");
        exit(1);
    }
    else if (pid == 0) {
        PoorStudent(ShmPTR);
        exit(0);
    }

    DearOldDad(ShmPTR);

    wait(&status);
    printf("Server has detected the completion of its child...\n");
    shmdt((void *) ShmPTR);
    printf("Server has detached its shared memory...\n");
    shmctl(ShmID, IPC_RMID, NULL);
    printf("Server has removed its shared memory...\n");
    printf("Server exits...\n");
    return 0;
}

// parent
void DearOldDad(int *ShmPTR)
{
    int account, balance;
    srand(getpid());

    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6);          // sleep 0-5 sec

        account = ShmPTR[0];

        while (ShmPTR[1] != 0);     // wait for my turn

        if (account <= 100) {
            balance = rand() % 101;
            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            }
            else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        }
        else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        ShmPTR[0] = account;
        ShmPTR[1] = 1;              // students turn
    }
}

// child
void PoorStudent(int *ShmPTR)
{
    int account, balance;
    srand(getpid());

    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6);          // sleep 0-5 sec

        account = ShmPTR[0];

        while (ShmPTR[1] != 1);     // wait for my turn

        balance = rand() % 51;
        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        }
        else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        ShmPTR[0] = account;
        ShmPTR[1] = 0;              // dads turn
    }
}
