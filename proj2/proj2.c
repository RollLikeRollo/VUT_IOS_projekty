/*============================*
 *======= Jan Zboril =========*
 *=xzbori20@stud.fit.vutbr.cz=*
 *===== PROJECT 2, IOS =======*
 *=== Faneuil Hall Problem ===*
 *====== FIT VUT BRNO ========*
 *=========== 2020 ===========*
 *============================*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>

//============================================= Function declaration: =================================================//
int arg_tests(int argc, char **argv);
int create_processes();
int create_immigrants(int MAX);
int random_number();
void do_immigrant_things();
void do_judge_things();
int cr_al_mem();
int cr_sem();
void mr_proper();

//============================================= Global variables: =================================================//

int PI;     // Number of Immigrant processes generated
int IG;     // Max number of miliseconds after which new Immigrant process is generated
int JG;     // Max number of miliseconds after which the Judge enters court again
int IT;     // Max nuber of miliseonds it takes Immigrant process to take his certificate
int JT;     // Max nuber of miliseonds it takes Judge to make his decision


FILE * output_file = NULL;      // output file

pid_t Judge;
pid_t Immigrants;
pid_t Immigrant;

// Shared memory values and pointers
int shm_global_counter;
int *global_counter;
int shm_imm_counter;
int *imm_counter;
int shm_NE;     // number of immigrants in court, without decision
int *NE;
int shm_NC;     // number of immigrants in court registered with no decision
int *NC;
int shm_NB;     //number of immigrants in court
int *NB;
int shm_HMD;    // how many immigrants processed
int *how_many_done;
int shm_HMC;     // how many immigrants got certificate
int *how_many_certified;

// semaphore declaration
sem_t *sem_file; // for output
sem_t *sem_judge_in;
sem_t *sem_register;
sem_t *sem_deciding;

//============================================= MAIN ==============================================================//
int main(int argc, char **argv)
{

    // calling function to test arguments
    int err = arg_tests(argc, argv);
    if( err == 1){
        return 1;
    }

    // Opening file for output
    output_file = fopen("proj2.out", "w");
    if( output_file == NULL ){
        fprintf(stderr, "Error while opening output file! App terminating!\n");
        mr_proper();
        return 1;
    }

    // printf("Tohle je main %d \n", (int)getpid());

    //create shared memory
    err = cr_al_mem();
    if (err == 1)
    {
        mr_proper();
        return 1;
    }

    //create semaphores
    err = cr_sem();
    if (err == 1)
    {
        mr_proper();
        return 1;
    }

    // Create processes
    err = create_processes();
    if( err == 1 ){
        mr_proper();
        return 1;
    }

    // wait for children
    if ((Judge > 0) && (Immigrant > 0) && (Immigrants > 0))
    {
        while (wait(NULL) > 0) 
            ;
    }
    // Clear all resources
    mr_proper();

    return 0;
}
//============================================= END OF MAIN =======================================================//

//============================================= Argument testing: =================================================//
int arg_tests(int argc, char **argv)
{

    if (argc != 6)
    {
        fprintf(stderr, "Wrong number of arguments! Ecpected 5, entered %d\n", argc - 1);
        return 1;
    }

    //  testing if argv[1] is number
    //      - tests every char in string, if number, add to PI, if not send to stderr
    PI = 0; //value to store current argument
    int size_string = strlen(argv[1]);
    char *arg = argv[1];
    for (int i = 0; i < size_string; i++)
    {
        if (arg[i] >= '0' && arg[i] <= '9')
        {
            PI = 10 * PI + (arg[i] - '0');
        }else{
            fprintf(stderr, "Wrong argument format. Expected int >= 1, entered %s\n", arg);
            return 1;
        }    
    }
    if (PI < 1)
    {
        fprintf(stderr, "Wrong argument format. Expected int >= 1, entered %s\n", arg);
        return 1;
    }

    //  testing if argv[2] is number
    //      - tests every char in string, if number, add to IG, if not send to stderr
    IG = 0; //value to store current argument
    size_string = strlen(argv[2]);
    arg = argv[2];
    for (int i = 0; i < size_string; i++)
    {
        if (arg[i] >= '0' && arg[i] <= '9')
        {
            IG = 10 * IG + (arg[i] - '0');
        }
        else
        {
            fprintf(stderr, "Wrong argument format. Expected int >= 0 && <= 2000, entered %s\n", arg);
            return 1;
        }
    }
    if (IG > 2000)
    {
        fprintf(stderr, "Wrong argument format. Expected int >= 0 && <= 2000, entered %s\n", arg);
        return 1;
    }

    //  testing if argv[3] is number
    JG = 0; //value to store current argument
    size_string = strlen(argv[3]);
    arg = argv[3];
    for (int i = 0; i < size_string; i++)
    {
        if (arg[i] >= '0' && arg[i] <= '9')
        {
            JG = 10 * JG + (arg[i] - '0');
        }
        else
        {
            fprintf(stderr, "Wrong argument format. Expected int >= 0 && <= 2000, entered %s\n", arg);
            return 1;
        }
    }
    if (JG > 2000)
    {
        fprintf(stderr, "Wrong argument format. Expected int >= 0 && <= 2000, entered %s\n", arg);
        return 1;
    }

    //  testing if argv[4] is number
    IT = 0; //value to store current argument
    size_string = strlen(argv[4]);
    arg = argv[4];
    for (int i = 0; i < size_string; i++)
    {
        if (arg[i] >= '0' && arg[i] <= '9')
        {
            IT = 10 * IT + (arg[i] - '0');
        }
        else
        {
            fprintf(stderr, "Wrong argument format. Expected int >= 0 && <= 2000, entered %s\n", arg);
            return 1;
        }
    }
    if (IT > 2000)
    {
        fprintf(stderr, "Wrong argument format. Expected int >= 0 && <= 2000, entered %s\n", arg);
        return 1;
    }

    //  testing if argv[5] is number
    JT = 0; //value to store current argument
    size_string = strlen(argv[5]);
    arg = argv[5];
    for (int i = 0; i < size_string; i++)
    {
        if (arg[i] >= '0' && arg[i] <= '9')
        {
            JT = 10 * JT + (arg[i] - '0');
        }
        else
        {
            fprintf(stderr, "Wrong argument format. Expected int >= 0 && <= 2000, entered %s\n", arg);
            return 1;
        }
    }
    if (JT > 2000)
    {
        fprintf(stderr, "Wrong argument format. Expected int >= 0 && <= 2000, entered %s\n", arg);
        return 1;
    }

    return 0;
}

//============================================= Create + alloc memory =====================================================//
int cr_al_mem(){

    // create shared memory
    shm_global_counter = shm_open("/IOS_global_counter", O_CREAT | O_EXCL | O_RDWR, 0666);
    shm_imm_counter = shm_open("/IOS_imm_counter", O_CREAT | O_EXCL | O_RDWR, 0666);
    shm_NE = shm_open("/IOS_NE", O_CREAT | O_EXCL | O_RDWR, 0666);
    shm_NC = shm_open("/IOS_NC", O_CREAT | O_EXCL | O_RDWR, 0666);
    shm_NB = shm_open("/IOS_NB", O_CREAT | O_EXCL | O_RDWR, 0666);
    shm_HMD = shm_open("/IOS_HMD", O_CREAT | O_EXCL | O_RDWR, 0666);
    shm_HMC = shm_open("/IOS_HMC", O_CREAT | O_EXCL | O_RDWR, 0666);

    // Alloc size of int in shared memory
    ftruncate(shm_global_counter, sizeof(int));
    ftruncate(shm_imm_counter, sizeof(int));
    ftruncate(shm_NE, sizeof(int));
    ftruncate(shm_NC, sizeof(int));
    ftruncate(shm_NB, sizeof(int));
    ftruncate(shm_HMD, sizeof(int));
    ftruncate(shm_HMC, sizeof(int));

    // Map shared memory
    global_counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_global_counter, 0);
    imm_counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_imm_counter, 0);
    NE = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_NE, 0);
    NC = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_NC, 0);
    NB = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_NB, 0);
    how_many_done = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_HMD, 0);
    how_many_certified = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_HMC, 0);

    // errors handling
    if (shm_global_counter == -1 || shm_imm_counter == -1 || shm_NE == -1 || shm_NC == -1 || shm_NB == -1 || shm_HMD == -1 || shm_HMC == -1)
    {
        fprintf(stderr, "Error while creating shared memory! Program terminating!\n");
        return 1;
        }
        if (global_counter == MAP_FAILED || imm_counter == MAP_FAILED || NE == MAP_FAILED || NC == MAP_FAILED || how_many_certified == MAP_FAILED || NB == MAP_FAILED || how_many_done == MAP_FAILED)
        {
            fprintf(stderr, "Error while mapping shared memory! Program terminating!\n");
            return 1;
        }

    return 0;
}

//============================================= Create + alloc semaphores =====================================================//
int cr_sem(){

    sem_file = sem_open("/IOS_file", O_CREAT | O_EXCL, 0666, 1);
    sem_judge_in = sem_open("/IOS_judge_in", O_CREAT | O_EXCL, 0666, 1);
    sem_register = sem_open("/IOS_register", O_CREAT | O_EXCL, 0666, 1);
    sem_deciding = sem_open("/IOS_deciding", O_CREAT | O_EXCL, 0666, 0);

    if (sem_file == SEM_FAILED || sem_judge_in == SEM_FAILED || sem_register == SEM_FAILED || sem_deciding == SEM_FAILED)
    {
        fprintf(stderr, "Error while creating semaphores! Program terminating!\n");
        return 1;
    }

    return 0;
}
//============================================= Random Number generator ========================================//
int random_number(int limit){
    time_t t;
    srand((unsigned)time(&t));
    return (rand() % (limit + 1));
}


//============================================= Process generator: =================================================//
int create_processes()
{
    Judge = fork();     //make new process for Judge
    if (Judge < 0)
    {
        fprintf(stderr, "Fork failed! App terminating!");
        return(1);
    }
    else if (Judge == 0)    //this is Judge body
    {
        do_judge_things();
    }
    if(Judge > 0){
        Immigrants = fork();        //make new process for Immigrants
        if (Immigrants < 0)
        {
            fprintf(stderr, "Fork failed! App terminating!");
            kill(Judge, SIGTERM);
            return (1);
        }
        else if (Immigrants == 0)       //this is Immigrants body
        {
            // printf("Tohle je Immigrants (generator) %d a muj rodic je %d \n", (int)getpid(), (int)getppid());
            int err = create_immigrants(PI);
            if( err == 1){
                kill(Immigrants, SIGTERM);
                kill(Judge, SIGTERM);
                fprintf(stderr, "Fork failed! App terminating!");
                return (1);
            }

            kill(getpid(), SIGTERM);        // ends process for Imm generating
        }
    }
    for (int i = 0; i < PI; i++)
    {
        wait(NULL);
    }

    return 0;
}
//============================================= Immigrant generator: =================================================//
int create_immigrants(int MAX)
{
    int imm_counter = 0;
    if (MAX > 0)
    {
        if( IG != 0 ){
            usleep(random_number(IG) * 1000);
        }
        if ((Immigrant = fork()) < 0)
        {
            fprintf(stderr, "Fork failed! App terminating!");
            return 1;
        }
        else if (Immigrant == 0)
        {
            // child here = Immigrant
            do_immigrant_things(imm_counter);
        }
        else if (Immigrant > 0)
        {
            
            create_immigrants(MAX - 1);
        }
    }
    return 0;
}

//============================================= do_immigrant_things =================================================//
void do_immigrant_things()
{
    int my_number;
    // printf("Jsem Immigrant (child) %d a muj rodic je %d \n", (int)getpid(), (int)getppid());
    //fprintf(output_file, "%d    : IMM %d    : starts\n", 100, 20);
    // printf("IMM jdu se vypsat\n");

    sem_wait(sem_file);
    // printf("IMM vypisuju se\n");
    (*global_counter)++;
    (*imm_counter)++;
    my_number = *imm_counter;
    fprintf(output_file, "%d:    : IMM %d    : starts\n", *global_counter, *imm_counter);
    fflush(output_file);
    sem_post(sem_file);

    // Immigrant enters court
    // printf("IMM jdu do baraku\n");
    sem_wait(sem_judge_in);
    sem_wait(sem_file);
    // printf("IMM jsu v baraku\n");
    (*global_counter)++;
    (*NE)++;
    (*NB)++;
    fprintf(output_file, "%d:    : IMM %d    : enters: %d: %d: %d:\n", *global_counter, my_number, *NE, *NC, *NB);
    fflush(output_file);
    sem_post(sem_file);
    sem_post(sem_judge_in);


    // Immigrant registers
    // printf("IMM jdu se registrovat\n");
   // sem_wait(sem_register);
    sem_wait(sem_file);
    // printf("IMM registoval jsem se\n");
    (*global_counter)++;
    (*NC)++;
    fprintf(output_file, "%d:    : IMM %d    : checks: %d: %d: %d:\n", *global_counter, my_number, *NE, *NC, *NB);
    fflush(output_file);
    sem_post(sem_file);
    //  sem_post(sem_register);

    // Immigrant waits for decision and takes certificate
    sem_wait(sem_deciding);

    sem_wait(sem_file);
    (*how_many_certified)++;
    if(*how_many_certified <= *how_many_done){
        (*global_counter)++;
        fprintf(output_file, "%d:    : IMM %d    : wants certificate: %d: %d: %d:\n", *global_counter, my_number, *NE, *NC, *NB);
        fflush(output_file);
        sem_post(sem_file);
        sem_post(sem_deciding);

        usleep(random_number(IT));

        sem_wait(sem_deciding);
        sem_wait(sem_file);
        (*global_counter)++;
        fprintf(output_file, "%d:    : IMM %d    : got certificate: %d: %d: %d:\n", *global_counter, my_number, *NE, *NC, *NB);
        fflush(output_file);
        sem_post(sem_file);
        sem_post(sem_deciding);
    }
    sem_post(sem_file);

    // Imm leaves court after Judge
    sem_wait(sem_judge_in);
    sem_wait(sem_file);
    (*global_counter)++;
    (*NB)--;
    fprintf(output_file, "%d:    : IMM %d    : leaves: %d: %d: %d:\n", *global_counter, my_number, *NE, *NC, *NB);
    fflush(output_file);
    sem_post(sem_file);
    sem_post(sem_judge_in);

    // Imm ends himself = process immigrants ending here
    // printf("IMM Ukoncil jsem se\n");
    kill(getpid(), SIGTERM);

    return;
 }

//============================================= do_judge_things =========================================================//
void do_judge_things(){
    

    *how_many_done = 0;
    while (*how_many_done < PI){

        usleep((random_number(JG) * 1000));

        //Judge entering
        sem_wait(sem_judge_in);
        sem_wait(sem_file);
        (*global_counter)++;
        fprintf(output_file, "%d:       JUDGE: wants to enter\n", *global_counter);
        fflush(output_file);
        sem_post(sem_file);
        sem_wait(sem_file);
        (*global_counter)++;
        fprintf(output_file, "%d:       JUDGE: enters: %d: %d: %d:\n", *global_counter, *NE, *NC, *NB);
        fflush(output_file);
        sem_post(sem_file);

        // Waits for all inside to registrate
        int written = 0;
        while (*NC != *NE)
        {
            if(written == 0){
                sem_wait(sem_file);
                (*global_counter)++;
                fprintf(output_file, "%d:       JUDGE: waits for imm: %d: %d: %d:\n", *global_counter, *NE, *NC, *NB);
                fflush(output_file);
                sem_post(sem_file);
                written = 1;        // For writing only once
            }
        }

        // deciding
        sem_wait(sem_file);
        (*global_counter)++;
        fprintf(output_file, "%d:       JUDGE: starts confirmation: %d: %d: %d:\n", *global_counter, *NE, *NC, *NB);
        fflush(output_file);
        sem_post(sem_file);

        usleep(random_number(JT) * 1000);

        sem_wait(sem_file);
        *how_many_done = *NE + *how_many_done;
        *NE = 0;
        *NC = 0;
        (*global_counter)++;
        fprintf(output_file, "%d:       JUDGE: ends confirmation: %d: %d: %d:\n", *global_counter, *NE, *NC, *NB);
        fflush(output_file);
        sem_post(sem_file);

        sem_post(sem_deciding);

        usleep(random_number(JT) * 1000);

        // Judge leaving
        sem_wait(sem_file);
        (*global_counter)++;
        fprintf(output_file, "%d:       JUDGE: leaves: %d: %d: %d:\n", *global_counter, *NE, *NC, *NB);
        fflush(output_file);
        sem_post(sem_file);

        sem_post(sem_judge_in);
        // printf("JUD odesel jsem\n");

        
    }
    // printf("JUD Koncim\n");
    sem_wait(sem_file);
    (*global_counter)++;
    fprintf(output_file, "%d:        JUDGE: finishes:\n", *global_counter);
    fflush(output_file);
    sem_post(sem_file);

    // Judge ends himself = process Judge ending here
    // printf("JUD Ukoncil jsem se\n");

    kill(getpid(), SIGTERM);
    return;
}

//============================================= Clean Everything ========================================================//
void mr_proper(){


    // close output file
    fclose(output_file);

    //Unmap the pointer
    munmap(global_counter, sizeof(int));
    munmap(imm_counter, sizeof(int));
    munmap(NE, sizeof(int));
    munmap(NB, sizeof(int));
    munmap(NC, sizeof(int));
    munmap(how_many_done, sizeof(int));
    munmap(how_many_certified, sizeof(int));

    //clear memory and close
    close(shm_global_counter);
    shm_unlink("/IOS_global_counter");
    close(shm_imm_counter);
    shm_unlink("/IOS_imm_counter");
    close(shm_NE);
    shm_unlink("/IOS_NE");
    close(shm_NB);
    shm_unlink("/IOS_NB");
    close(shm_NC);
    shm_unlink("/IOS_NC");
    close(shm_HMD);
    shm_unlink("/IOS_HMD");
    close(shm_HMC);
    shm_unlink("/IOS_HMC");

    //clear semaphores
    sem_close(sem_file);
    sem_unlink("/IOS_file");
    sem_close(sem_judge_in);
    sem_unlink("/IOS_judge_in");
    sem_close(sem_register);
    sem_unlink("/IOS_register");
    sem_close(sem_deciding);
    sem_unlink("/IOS_deciding");

    // printf("MR. PROPER in action!\n");

    return;
}

// ============================================ END OF FILE =================================================================//