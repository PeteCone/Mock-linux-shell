/*Peter Conant Red ID 821518133*/
/*Professor Carroll CS 570*/
/*10/9/2020*/
/*Program2/4*/
/*
p2/p4 is a mock shell created by a student to practice the workings of operatings systems. 
P2/p4 uses a former project getword.c to extract distinct words from user input. Pointers 
to words and flags for various cases are set in parse(). main() uses execvp() to call
functions with arguments given by pointer_array[](also known as newargv[]), inFile, outFile and other variables. 

*/
#include <stdio.h>
#include <stdlib.h>
#include "getword.h"
#include "p2.h"
#include <math.h>



char word_array[MAXSTORAGE];//Array passed to and written to getword
char *newargv[MAXITEM];//Array of pointers pointering to the word
int non_meta_meta_flag; //Variable passed from getword.c to make diference between meta characters


int inFlag=0, inFile_descr = NULL, outFlag=0, outFile_descr = 0,//in and out varibles 
    pipeFlag=0, pid = 0, kidpid = 0,  childPid = 0, gcPid = 0, //PID variables & pipe variables
    amperFlag=0, eof=0, word_len = 0, hereisFlag = 0, enverror = 0; //misc variabels

char *inFile=NULL, *outFile=NULL; //In and Out File set in parse, checked in the beginning of main
char *prmad, *prmtmp, *buffer;

int pipefd[10][2]; // array of pipes, each pipe has an in pipe[x][1] and an out pipe[x][0]
int pipe_index[10];
FILE *tempfile; // temporary file for '<<'  "hereis" condition

/*sighandler()
Takes: void
Returns:void
an empty function. Needed as an argument in signal(SIGTERM, sighandler); at the beginning of main. 
*/
void sighandler(){ 
}

/*forkhelper()
Takes: void
Retuens: void
Forks a child to execute a single command. Command must be at newargv[0] and addtional arguments must follow in newargv.
*/
void forkhelper(){
    fflush(stderr);
    fflush(stdout);
    if(-1 == (kidpid = fork())){
        fprintf(stderr,"Fork Error");
        exit(1);
    }
    //CHILD
    else if(kidpid == 0){
        if(inFlag == 2){
            dup2(inFile_descr, STDIN_FILENO); //set standard in to specified file
            if(close(inFile_descr) == -1){
                fprintf(stderr, "Error! Could not close file descriptor.\n");
            }
        }
        if(outFlag == 2){
            dup2(outFile_descr, STDOUT_FILENO); //set start out to specified file
            if(close(outFile_descr) == -1){
                fprintf(stderr, "Error! Could not close file descriptor.\n");
                exit(2);
            }
        }
        if(execvp(newargv[0], newargv) == -1){ // Execute command in newargv
            fprintf(stderr, "Error! %s is not an executable command", newargv[0]);
            exit(1);
        }
        exit(0);
    }
    //PARENT: fork returns the child pid if the current process is the parent
    else if(amperFlag == 1){//if there is an & at end of line, we do not wait for the child
        printf("%s [%d]\n",newargv[0], kidpid); 
        return; //IF INPUTS WITHOUT A PIPE BREAK CHECK HERE! IT WAS ORIGINALLY A 'CONTINUE;' in the for loop BEFORE THIS WAS A FUNCTION 
    }
    else{// wait for the child to finish
        for(;;){
            pid = wait(NULL);//wait returns the child pid once the child has closed
            if(pid == kidpid){
                break;
            }
        } 
    }  
}

// /*PIPE
//     Pipe is a special kind of fork. Two execs must be called and depend on eachother. A child is created and 
//     from that child a grandchild. The child uses the grandchild's results to finish its command. The parent waits
//     for the child to finish before continueing. If there is an lone & at the end of the line the parent will not
//     wait and repromt and parse. pipeFlag is caught and set inside of parse().
// */
void pipehelper(){
    int i =  0, generalPid = 0;
    int pipe_count = pipeFlag;
    fflush(stderr);//clear stderr and stdout
    fflush(stdout);
    //PARENT & CHILD: fork
    if(-1 == (childPid = fork())){
        fprintf(stderr,"Error! Child in pipe could not fork");
        exit(1);
    }
    if(childPid == 0){
        if(pipe(pipefd[0]) == -1){ //create first pipe
            fprintf(stderr, "Error! Pipe broke.");
            exit(1);
        }
        if((gcPid = fork()) == -1){ // create grandchild
            fprintf(stderr,"Error. Grandchild in pipe cannot fork!");
            exit(1);
        }
        //GRANDCHILD
        if(gcPid == 0){
            i = 1;
            while(i < pipe_count){ // while there is more than one command left
                if(pipe(pipefd[i]) == -1){ // Pipe for a new child
                    fprintf(stderr, "Error! Pipe broke.");
                    exit(1);
                }
                if((generalPid = fork()) == -1){ //fork to create new child
                    fprintf(stderr,"Error. Grandchild in pipe cannot fork!");
                    exit(1);
                }
                if(generalPid == 0){ // if you are child of fork increase counter and continue
                    i++;
                }
                else{ //if you are parent of fork execute a command
                    dup2(pipefd[i][0], STDIN_FILENO);
                    dup2(pipefd[i-1][1], STDOUT_FILENO);
                    close(pipefd[i][1]);
                    close(pipefd[i][0]);
                    close(pipefd[i-1][1]);
                    execvp(newargv[pipe_index[pipe_count-i-1] + 1], newargv + pipe_index[pipe_count-i-1] + 1); //execute pipe commands in desending order
                    exit(1);
                }
            }
            dup2(pipefd[i-1][1], STDOUT_FILENO); //there is one more command, set last pipe
            close(pipefd[i][0]);
            close(pipefd[i-1][1]);
            if(inFlag == 2){ // set in file (if there is one) to standard in
                dup2(inFile_descr, STDIN_FILENO);
                close(inFile_descr);
            }
            execvp(newargv[0], newargv); //execute first command in new argv
            exit(1);
        }
        else{//CHILD
            dup2(pipefd[0][0], STDIN_FILENO);
            close(pipefd[0][0]);
            close(pipefd[0][1]);
            if(outFlag == 2){ // change standard out to out file if applicable
                dup2(outFile_descr, STDOUT_FILENO);
                close(outFile_descr);
            }
            execvp(newargv[pipe_index[pipe_count-1] + 1], newargv+pipe_index[pipe_count-1] + 1); // execute last pipe command
            exit(1);
        }
    }
    //PARENT: wait for the child process to finish
    //& Case: we do not wait for the child
    else if(amperFlag == 1){
        printf("%s [%d]\n",newargv[0], childPid);
        return;
    }
    else{
        for(;;){
            pid = wait(NULL);
            if(pid == childPid){
                break;
            }
        }
    }
}

/*main()
Takes: NA
Returns: NA
Body of p2. bodies of code worth noting: CD, PIPE, and FORK. Worth noting that PIPE is a special type of fork, much of
the code is similar between these two parts. Main heavily depends on parse to build pointer_array and set flags.
1. CD: Will be activated if the uses enters cd as the first command.
*/
main(){
    int m = 0;
    setpgid(0,0);
    (void) signal(SIGTERM, sighandler);
    for(;;){
        if(prmad != NULL){
            printf("%s:570: ", prmad);
        }
        else{
            printf(":570: ");
        }
        
        m = parse(); // Get commands into newargv and set flags amoung other things

        if(eof == 1){ //When a eof is reached stop looking for
            break;
        }
        if(m == 0){ //if there are no commands, go to next line
            continue;
        }
        if(newargv[0] == NULL){ // second command check but print an error and don't go to next line
            fprintf(stderr, "Error! Invalid NULL command.");
        }
        if(enverror >= 1){
            fprintf(stderr, "Error! Enviornment %s not found.", newargv[enverror]);
            continue;
        }

        if(inFlag != 0){
            if(hereisFlag > 0){
                if((inFile_descr = open("/home/cs/carroll/cssc0017/Two/p4hereis", O_RDONLY)) == -1){ // set in file to temporary file
                    fprintf(stderr, "Error! Could not open here is file \"/home/cs/carroll/cssc0017/Two/p4hereis\"");
                }
            }
            else if(inFile == NULL){
                fprintf(stderr, "Error! Infile not found.");
                continue;
            }
            else if((inFile_descr = open(inFile, O_RDONLY)) == -1){ // set in file to specified file
                fprintf(stderr,"Error! Invalid infile.");
                continue;
            }
        }
        if(outFlag != 0){
            if(outFile == NULL){
                fprintf(stderr, "Error! Outfile not found.");
                continue;
            }
            else if((outFile_descr = open(outFile, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR)) == -1){ // set outfile to specified file
                fprintf(stderr, "Error! Invalid outfile.");
                continue;
            }
        }

        /*
        1. CD: Will be activated if the uses enters cd as the first command. If cd is the only command user wil be taken to the home 
        directory. If cd is followed by am address, user will be take to that address.
        */
        if(strcmp(newargv[0], "cd") == 0){
            if(m == 1){ //cd is the only word, go to the home director
                if(chdir(getenv("HOME")) == -1){
                    fprintf(stderr, "chdir borke. come again another day");
                }
                if(strcmp(getenv("HOME"), "/") == 0){ // special case
                    prmad = "/";
                }
                else{
                    prmtmp = strtok(getenv("HOME"), "/");
                    while((prmtmp = strtok(NULL, "/")) != NULL){
                        prmad = prmtmp;
                    }
                }
            }
            else if(m == 2){//additional address to cd to
                if(chdir(newargv[1])==-1){
                    fprintf(stderr, "Error! could not change directory to %s", newargv[1]);
                }
                else{
                    prmad = strtok(newargv[1], "/");
                    while((prmtmp = strtok(NULL, "/")) != NULL){
                        prmad = prmtmp;
                    }
                }
            }
            else{ // If m is more than 2, there are too man arguments for cd
                fprintf(stderr, "Error, too many arguments for cd.");
            }
            continue;// There are no commands to be sent into execvp, we can call prompt and call parse again
        }

        if(strcmp(newargv[0], "environ") == 0){
            if(m == 2){
                if(getenv(newargv[1]) == NULL){
                    fprintf(stderr, "Enviornment not found.");
                }
                printf(getenv(newargv[1]));
                printf("\n");
            }
            if(m == 3){
                if(setenv(newargv[1], newargv[2], 1) == -1){
                    fprintf(stderr, "Could not set enviornment to %s", newargv[2]);
                }
            }
            continue;
        }

        /*PIPE
        Pipe is a special kind of fork. Two execs must be called and depend on eachother. A child is created and 
        from that child a grandchild. The child uses the grandchild's results to finish its command. The parent waits
        for the child to finish before continueing. If there is an lone & at the end of the line the parent will not
        wait and repromt and parse. pipeFlag is caught and set inside of parse().
        */
        if(pipeFlag > 0){
            pipehelper();
        }

        /*FORK
        Standard fork: does not create a grand child
        */
        else{
            forkhelper();
        }
    } 
    //at the end of file (eof) we kill all extra processies
    remove("/home/cs/carroll/cssc0017/Two/p4hereis");
    killpg(getpgrp(), SIGTERM);
    printf("p2 terminated.\n");//declare program finished
    exit(0);
}

/*PARSE()
Takes: No arguments, requries pointer_array[]
RETURNS: int word_count, a total count of words documented in pointer_array[]
Parse passes points in an array to getword which in turn writes 
"words" (defined in p1 getword.h) to word_array. Parse saves the point in that array
as a word in pointer_array, or sets a flag for "words" or characters such as <, >, 
&, and |. Flags and indexs of these special characters are saved for use in main.
*/
int parse(){
    int i = 0, abs_word_len = 0, offset = 0, word_count = 0, tmpflag = 0,
        temp = 0, tildaFlag = 0, tilerrFlag = 0, strsize = 0; //declareations
    char *delimDes = NULL, *tmpbuffer =NULL, *nxttok = NULL;
    size_t bufsize = 32;   
    FILE *filelocation; 
    buffer = NULL;
    inFlag=0, outFlag=0, pipeFlag=0, eof=0, amperFlag=0, enverror = 0;//reset flags
    for(;;){
        word_len = getword(word_array + offset);

        if(word_len == 0 && hereisFlag ==0){
            break;
        }
        if(word_len == 0 && hereisFlag == 1){ // at end of a line with a << continue reading until delimiter line
            if(word_count == 0){
                break;
            }
            if(delimDes == NULL){
                fprintf(stderr,"Error! Invalid NULL deliminator.");
                hereisFlag = 0;
                inFlag = 0;
            }
            else{
                if((tempfile = fopen("/home/cs/carroll/cssc0017/Two/p4hereis","w")) == NULL){
                    fprintf(stderr,"Error. Unable to access 'p4hereis' in 'Two' directory.");
                }
                for(;;){ //write all lines between first line and delimiter line to the above address
                    strsize = getline(&buffer,&bufsize,stdin);
                    if(strcmp(buffer,delimDes) == 0){
                        break;
                    }
                    else{
                        fwrite(buffer,strsize,1,tempfile);
                    }
                }
                fclose(tempfile);
            }
            break;
        }

        if(word_len == -255){
            eof = 1;
            break;
        }
        //BACKGROUND &
        if(amperFlag == 1){ //& was not the last word
            newargv[i] = word_array + temp; //save & in pointer_array
            i++;
            amperFlag = 0;//set to 0
        }

        //INFILE the prevoius word was <, the current word is the infile
        if(inFlag == 1){
            if(hereisFlag == 1){
                *(word_array + offset + word_len) = '\n';
                word_len++;
                delimDes = word_array+offset;
            }
            else{
                inFile = word_array + offset; //set the file               
            }
            inFlag++;
        }
        //OUTFILE the prevoius word was >, the current word is the outfile
        else if(outFlag == 1){
            if(word_len < 0){
                outFile = getenv(word_array + offset);
            }
            else
            {
                outFile = word_array + offset;
            }
            outFlag++;
        }

        else if(tildaFlag == 1){
            tildaFlag = 0;
            tmpbuffer = strtok(word_array + offset, "/");
            if(strtok(NULL,"/")){
                tmpflag = 1;
            }
            if((filelocation = fopen("/etc/passwd", "r"))==NULL){//find path name inside /ect/passwd
                fprintf(stderr, "Could not open /etc/passwd");
            }
            while(getline(&buffer,&bufsize,filelocation) != -1){ //search each line in /ect/passwd
                nxttok = strtok(buffer,":");
                if(strcmp(nxttok, tmpbuffer) == 0){
                    strtok(NULL,":");
                    strtok(NULL,":");
                    strtok(NULL,":");
                    strtok(NULL,":");
                    buffer = strtok(NULL,":");
                    if(tmpflag == 1){
                        strcat(buffer, "/");
                        strcat(buffer, (word_array+offset+(strlen(tmpbuffer))+1));
                    }
                    newargv[i] = buffer; // assign path name to rightful place in newargv
                    i++;
                    word_count++;
                    tilerrFlag = 0;
                    break;
                }
                else{ // if path name if no found set error flag to 1
                    tilerrFlag = 1;
                }                
            }
            if(tilerrFlag == 1){
                fprintf(stderr,"Could not find directory %s.", word_array+offset);
                exit(1);
            }
        }
        //INFILE the current word is <, set flag and keep index
        else if(*(word_array + offset) == '<' && non_meta_meta_flag != 1){
            if(inFlag == 2){//there are mutiple < in this line, throw error
                fprintf(stderr, "Error! More than one file redirect.\n");
            }
            else{
                if(*(word_array+offset+1) == '<'){
                    hereisFlag = 1;
                }
                inFlag = 1;
            }
        }
        //OUTFILE the current word is >, set flag and keep index
        else if(*(word_array + offset) == '>' && non_meta_meta_flag != 1){
            if(outFlag == 2){//there are mutiple > in this line, throw error
                fprintf(stderr, "Error! More than one file redirect.\n");
            }
            else{
                outFlag = 1;
            }
        }

        else if(*(word_array + offset) == '~' && non_meta_meta_flag != 1){
            if(inFlag == 2){//there are mutiple < in this line, throw error
                fprintf(stderr, "Error! More than one file redirect.\n");
            }
            else{
                tildaFlag = 1;
            }
        }

        //PIPE
        else if(*(word_array + offset) == '|' && non_meta_meta_flag != 1){
            if(pipeFlag == 10){
                fprintf(stderr, "Error! This shell only supports up to ten pipes.\n");
                exit(1);
            }
            newargv[i] = NULL;
            pipe_index[pipeFlag] = i++;
            pipeFlag++;
        }
        
        //BACKGROUND &
        else if(*(word_array + offset) == '&' && non_meta_meta_flag != 1){
            amperFlag = 1; //if & is the last word amperFlag will not be rest to 0
            temp = offset;
        }
        else if(word_len < 0){
            if((newargv[i] = getenv(word_array + offset)) == NULL){
                enverror = i;
                newargv[i] = word_array + offset;
            }
            i++;
            word_count++;
        }
        //NO SPECIAL CASES, this is a normal word
        else{
            newargv[i] = word_array + offset;//save the word
            i++;
            word_count++; //increase the word count
        }
        abs_word_len = abs(word_len); //get absolute value of word_len (return can be negative)
        offset = offset + abs_word_len + 1; //offset = previous offset + length of current word + Terminator 0
    }
    newargv[i] = 0;
    return word_count;
}