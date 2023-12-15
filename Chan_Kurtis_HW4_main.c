/**************************************************************
* Class:  CSC-415-03 Fall 2022
* Name:Kurtis Chan
* Student ID:918319175 
* GitHub ID:kchanw
* Project: Assignment 4 – Word Blast
*
* File: Chan_Kurtis_HW3_main.c
*
* Description: Write a program to read and tally the occurrences of words within a given file.
* This is to be done with threads for reading with mutex locks around critical sections.
* Once tallying is done, display the top 10 most common words and how many times they occur.
*
**************************************************************/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

// You may find this Useful
char * delim = "\"\'->“”‘’?:;-,—*($%)! \t\n\x0A\r";
 struct Word{
    char word[30];//word stored in char array, unlikely to have words longer than 29 chars
    int count; //how many times the word occurs
    };//end of struct Word
struct Word* wordList;//holds structs of words
volatile int listSize;//how many words are in the pointer

int threads; //number of threads given by command line
off_t fileSize; //size of file to divide by threads
off_t block; //size of block for each thread to read
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //initalizing the mutex lock
int fileDesc; //how to identify what file is opened and needed to be closed


//bubble sort
void bubbleSort(struct Word list[], int size){
    struct Word temp;
    for(int i = 0; i < size-1;i++){
        for(int j = 0; j < size - i - 1; j++){
            if(list[j].count > list[j+1].count){
                temp = list[j];
                list[j] = list[j+1];
                list[j+1] = temp;
            }
        }
    }
}



//function to add words to list and increase frequency when found same word
void addWord(char* tok){
    for(int i = 0; i <= listSize; i++){
        //printf("%s", word);
        if(i == listSize){//when you searched whole list, add entry of word, need to reallocate more memory to wordList
    
        pthread_mutex_lock(&mutex);
        
        strcpy(wordList[i].word, tok);
        wordList[i].count = 1;
        //listSize++;
        wordList = realloc(wordList, ((sizeof(struct Word))+ sizeof(wordList))); //reallocate memory so it can handle another entry

        pthread_mutex_unlock(&mutex);
        
        break;
        }
        else if(strcmp(wordList[i].word, tok) == 0){//when you find a match +1 to count
          pthread_mutex_lock(&mutex);
            wordList[i].count+=1;
            //printf("%n", wordList[i].count);
            pthread_mutex_unlock(&mutex);
            
        break;
        }

    }
}

//function for the pthread create to use to read
void* readFile(void* arg){

    
    char *buffer = malloc(block);
    char *saveptr;

    off_t *end = (off_t*) arg;//get the end of the block
    off_t  start = *end - block;//start at end - block
    printf("starting point: %ld\n", start);

    pread(fileDesc, buffer, block, start);//read file at an offset
    

    char *token = strtok_r(buffer, delim, &saveptr);//need to use strtok_r because strtok is not thread safe -> from man page
    while(token != NULL){
        if(strlen(token) >= 6){
            addWord(token);
            }
        token = strtok_r(NULL,delim,&saveptr);//get next token
    }

    free(buffer);
    buffer = NULL;

    pthread_exit(NULL); //exit current thread
}//end of readFile




int main (int argc, char *argv[]){
    //***TO DO***  Look at arguments, open file, divide by threads
    //             Allocate and Initialize and storage structures

    wordList = malloc(sizeof(struct Word));

    listSize = 0;

    if(argv[1] == NULL || argv[2] == NULL){
        printf("File or # of threads not specified");
        return 0;
    }

    fileDesc = open(argv[1],O_RDONLY); //open file in read only mode
    threads = atoi(argv[2]);//getting # of threads
    printf("%d", threads);
    off_t offset = lseek(fileDesc, 0, SEEK_CUR); //initializing offset
    fileSize = lseek(fileDesc,0,SEEK_END);//get the size of the file
    block = (fileSize/threads); //how big each block is
    printf("\nFile Size: %ld\n blocks: %ld\n",fileSize,block);
    printf("offset: %ld\n",offset);
    lseek(fileDesc,offset,SEEK_SET); //go to beginning of file

    off_t* buffPositions[threads];


    for(int i = 0; i < threads; i++){
        buffPositions[i] = (off_t*) (block *(i+1));//the end position is set for each thread
        printf("position: %ld \n", buffPositions[i]);
    }

    //close(fileDesc);
    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************
    // *** TO DO ***  start your thread processing
    //                wait for the threads to finish

    pthread_t tids[threads];//a tid for each thread
    int retVal;
    for(int j = 0; j < threads; j++){

        retVal = pthread_create(&tids[j],NULL,readFile, &buffPositions[j]);//passing in each thread with their buffer positions at 4th arg
        if(retVal < 0){
            perror("ERROR: ");
            exit(-1);
        }
    }
    //waiting for each thread to finish
    for(int i = 0; i < threads; i++){
        pthread_join(tids[i],NULL);
    }
    // ***TO DO *** Process TOP 10 and display
    bubbleSort(wordList,listSize);//sort in non decreasing order

    int num = 1;
    //print out top 10 most common words 
    for(int i = listSize; i > listSize - 10; i--){
        printf("Number %d is %s with a count of %d \n", num , wordList[i].word, wordList[i].count);//print out 
        num++;
    }

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
        {
        --sec;
        n_sec = n_sec + 1000000000L;
        }

    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************

    // ***TO DO *** cleanup
    close(fileDesc);
    free(wordList);
    wordList = NULL;
    }
