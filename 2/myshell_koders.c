/*
 *CSci4061 S2007 Assignemnt2
 *name: Todd Lanis, Chaoxing D. Fu
 *section:
 *id:1778760, 3059455
 *NOTE: cd is implemented for extra credit (also, prompt prints current working directory)
 */
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_INPUT_SIZE 512
#define BUFF_SIZE 1024
#define TRUE 1
#define FALSE 0

extern char **environ;

//global variables to keep track of background processes
int bgPids[128];
char *bgCmds[128];

int makeargv(const char *s, const char *delimiters, char ***argvp);
void printArgArray(char **myargv, int numtokens);
void logger(char *errorMsg);
void addbgPid(int pid, char *cmd);
void deletebgPid(int pid);
char* getbgCmd(int pid);

int main(int argc, char *argv[])
{
    char delim[] = " \t";
    int i;
    char **myargv;
    int numtokens;
    char *prompt = "mysh";
    int promptLen = strlen(prompt);
    char pathBuf[BUFF_SIZE];
    int pathBufLen;
    char buf[BUFF_SIZE];
    //int bufLen;
    //int bytesread, byteswritten;
    //ssize_t bytesread, byteswritten;
    char *exitStr = "exit";
    pid_t pid;
    pid_t childpid;
    int status;
    int inbackground;
    char *errorMsg;
    char *logMsg;
    /* int bgPids[128]; */
    /*   char *bgCmds[128]; */

    i = 0;
    /* initialize background process to 0 */
    for(i = 0; i < 128; i++)
    {
        bgPids[i] = 0;
    }

    do
    {
        write(STDOUT_FILENO,prompt,promptLen);  /* print out the prompt */
        memset(buf,'\0',BUFF_SIZE); /* clear the buffer with NULL */
        memset(pathBuf,'\0',BUFF_SIZE); /* clear the path buffer with NULL */
        /* print the cwd after the prompt*/
        if(getcwd(pathBuf,(size_t)BUFF_SIZE) == NULL)
        {
            fprintf(stderr, "get current working directory failed: %s\n",
                    strerror(errno));
        }
        pathBufLen = strlen(pathBuf);
        if(pathBufLen > 0)
        {
            write(STDOUT_FILENO," (",2);
            write(STDOUT_FILENO,pathBuf,pathBufLen);  /* print out the cwd */
            write(STDOUT_FILENO,")\n",2);
            write(STDOUT_FILENO,"> ",2);
        }
        else
        {
            write(STDOUT_FILENO,"> ",2);
        }
        read(STDIN_FILENO,buf,BUFF_SIZE);  /* get first argument */
    }
    while(strcmp(buf,"\n") == 0);  /* re-prompt if just a return */

    /* log the input message NOT AN ERROR!!!*/
    logMsg = malloc(sizeof(char)*BUFF_SIZE);
    memset(logMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
    sprintf(logMsg, "MYSH INPUT FROM THE PROMPT: %s",buf);
    logger(logMsg);
    free(logMsg);

    /* put in argument array */
    if ((numtokens = makeargv(buf, delim, &myargv)) == -1)
    {
        errorMsg = malloc(sizeof(char)*BUFF_SIZE);
        memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
        sprintf(errorMsg, "MYSH ERROR: Failed to construct an argument array for %s\n", buf);
        logger(errorMsg);
        fprintf(stderr, "%s",errorMsg);
        free(errorMsg);
        return 1;
    }
    
    

    //continue to read in until exit is incountered or fatal error
    while(strcmp(myargv[0],exitStr) != 0)
    {

        /* setenv */
        if(strcmp(myargv[0],"setenv")==0)
        {
            if (numtokens != 3)
            {
                errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                sprintf(errorMsg, "MYSH ERROR: Usage: %s name value\n", myargv[0] );
                logger(errorMsg);
                fprintf(stderr, "%s",errorMsg);
                free(errorMsg);
            }

            else
            {
                char temp[512];
                /*add the absolute path to any dubug file */
                if(strcmp(myargv[1],"MYSH_DEBUG")==0)
                {
                    sprintf(temp,"%s=%s/%s",myargv[1],pathBuf,myargv[2]);
                }
                else
                {
                    sprintf(temp,"%s=%s",myargv[1],myargv[2]);
                }
                if(putenv(temp) != 0)
                {
                    errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                    memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                    sprintf(errorMsg, "MYSH ERROR: %s: environment variable failed to set: %s\n",
                            myargv[1],strerror(errno));
                    logger(errorMsg);
                    fprintf(stderr, "%s",errorMsg);
                    free(errorMsg);
                }
            }
        }

        /* printenv */
        else if(strcmp(myargv[0],"printenv")==0)
        {
            if (numtokens != 2)
            {
                errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                sprintf(errorMsg, "MYSH ERROR: Usage: %s name\n", myargv[0]);
                logger(errorMsg);
                fprintf(stderr, "%s",errorMsg);
                free(errorMsg);
            }
            else
            {
                /* check if already set */
                if(getenv(myargv[1]) == NULL)
                {
                    errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                    memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                    sprintf(errorMsg, "MYSH ERROR: %s: environment variable is undefined\n",myargv[1]);
                    logger(errorMsg);
                    fprintf(stderr, "%s",errorMsg);
                    free(errorMsg);
                }
                /* else print out the value of the variable */
                else
                {
                    printf("%s\n",getenv(myargv[1]));

                    /* log the message */
                    logMsg = malloc(sizeof(char)*BUFF_SIZE);
                    memset(logMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                    sprintf(logMsg, "MYSH OUTPUT FROM PRINTENV: %s\n",getenv(myargv[1]));
                    logger(logMsg);
                    free(logMsg);
                }
            }
        }

        /* unsetenv */
        else if(strcmp(myargv[0],"unsetenv")==0)
        {
            if (numtokens != 2)
            {
                errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                sprintf(errorMsg, "MYSH ERROR: Usage: %s name\n", myargv[0]);
                logger(errorMsg);
                fprintf(stderr, "%s",errorMsg);
                free(errorMsg);
            }
            else
            {
                /* check if set */
                if(getenv(myargv[1]) == NULL)
                {
                    errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                    memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                    sprintf(errorMsg,
                            "MYSH ERROR: %s: environment variable is undefined and can not be unset\n"
                            ,myargv[1]);
                    logger(errorMsg);
                    fprintf(stderr, "%s",errorMsg);
                    free(errorMsg);
                }
                /* else unset the variable */
                else
                {

                    if(unsetenv(myargv[1]) == -1)
                    {
                        errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                        memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                        sprintf(errorMsg,
                                "MYSH ERROR: %s: environment variable unset: %s\n",
                                myargv[1],strerror(errno));
                        logger(errorMsg);
                        fprintf(stderr, "%s",errorMsg);
                        free(errorMsg);

                    }
                }
            }
        }

        /* cd (change directory) */
        else if(strcmp(myargv[0],"cd")==0)
        {
            if (numtokens != 2)
            {
                errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                sprintf(errorMsg,
                        "MYSH ERROR: Usage: %s path\n", myargv[0]);
                logger(errorMsg);
                fprintf(stderr, "%s",errorMsg);
                free(errorMsg);
            }
            /* change to directory specified */
            else
            {
                if(chdir(myargv[1]) != 0)
                {
                    errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                    memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                    sprintf(errorMsg,
                            "MYSH ERROR: change directory to: %s failed: %s\n",
                            myargv[1],strerror(errno));
                    logger(errorMsg);
                    fprintf(stderr, "%s",errorMsg);
                    free(errorMsg);
                }
            }
        }


        else
        { /* we need to exicute a command */
            /*  printf("myargv[numtokens-1]=%s\n",myargv[numtokens-1]); */
            /*       printf("strcmp(myargv[numtokens-1],& = %d\n",strcmp(myargv[numtokens-1],"&")); */
            //check if background and handle it!!!!!!!!!!!!
            if(strcmp(myargv[numtokens-1],"&")==0)
            {
                /* 	printf("background Process!!!\n"); */
                inbackground = TRUE;
                myargv[numtokens-1]= NULL; /* remove the & from the args list */
                numtokens--;
            }
            else
            {
                inbackground = FALSE;
            }


            /*following code modified from class code 3-forkexec.c */
            //unneccessay I think
            if (numtokens < 1)
            {
                errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                sprintf(errorMsg, "MYSH ERROR: Usage: cmd ...\n");
                logger(errorMsg);
                fprintf(stderr, "%s",errorMsg);
                free(errorMsg);
                return 1;
            }

            pid = fork();

            // check for failure in fork
            if (pid == -1)
            {
                errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                sprintf(errorMsg, "MYSH ERROR: Failed to fork\n");
                logger(errorMsg);
                free(errorMsg);
                perror("Failed to fork\n");
                return 1;
            }

            if (pid == 0)
            { // child


                /* print out the id of the background process */
                if(inbackground)
                    printf("MYSH BGPID: %ld\n",(long)getpid());

                status = execvp(myargv[0], &myargv[0]);
                fflush(stdout);
                errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                sprintf(errorMsg, "MYSH ERROR: %s: Command not found.\n",myargv[0]);
                logger(errorMsg);
                fprintf(stderr, "%s",errorMsg);
                free(errorMsg);
                //	perror("Exec failed\n");
                return 1;
            }

            else if (pid > 0)
            { // parent
                /* wait for the child to finish */

                //method borrowed from Robbins program 11.13 ush7.c
                if(!inbackground)
                {
                    waitpid(pid, &status, 0); /* wait explicityly for the foreground process */
                    printf("\n"); /* print a blank line for formating purposes */
                }
                else
                {

                    /* log the message NOT AN ERROR!!!*/
                    logMsg = malloc(sizeof(char)*BUFF_SIZE);
                    memset(logMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                    sprintf(logMsg,
                            "MYSH OUTPUT FROM BACKGROUND PROCESS BEING CREADED: MYSH BGPID: %ld\n",
                            (long)pid);
                    logger(logMsg);
                    free(logMsg);

                    addbgPid(pid, buf); /* add this process to the list of processes in th b.g. */
                }

                while((childpid = waitpid(-1,&status,WNOHANG)) > 0)
                { /* wait for background process */
                    /* taken from robbins showreturnstatus.c modified to fit my program*/
                    if (childpid == -1)
                    { /* failed to wait for the child */
                        perror("MYSH ERROR: Failed to wait for child\n");
                        errorMsg = malloc(sizeof(char)*BUFF_SIZE);
                        memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                        sprintf(errorMsg,"MYSH ERROR: Failed to wait for child\n");
                        logger(errorMsg);
                        free(errorMsg);
                    }
                    else if (WIFEXITED(status) && !WEXITSTATUS(status))
                    { /*process finished normally*/
                        printf("MYSH DONE PID %ld: %s", (long)childpid, getbgCmd(childpid) );
                        /* log the message NOT AN ERROR!!!*/
                        logMsg = malloc(sizeof(char)*BUFF_SIZE);
                        memset(logMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                        sprintf(logMsg,
                                "MYSH OUTPUT FROM BACKGROUND PROCESS FINISHING: MYSH DONE: %s",
                                getbgCmd(childpid) );
                        logger(logMsg);
                        free(logMsg);
                    }
                    else if (WIFEXITED(status))
                    { /*process exited for some reason */
                        printf("MYSH EXIT %d: %s",WEXITSTATUS(status),getbgCmd(childpid));
                        /* log the message NOT AN ERROR!!!*/
                        logMsg = malloc(sizeof(char)*BUFF_SIZE);
                        memset(logMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                        sprintf(logMsg,
                                "MYSH OUTPUT FROM BACKGROUND PROCESS FINISHING: MYSH EXIT %d: %s",
                                WEXITSTATUS(status),getbgCmd(childpid));
                        logger(logMsg);
                        free(logMsg);
                    }
                    else if (WIFSIGNALED(status))
                    {/* process killed */
                        printf("MYSH TERMINATED: %s",getbgCmd(childpid) );
                        /* log the message NOT AN ERROR!!!*/
                        logMsg = malloc(sizeof(char)*BUFF_SIZE);
                        memset(logMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                        sprintf(logMsg,
                                "MYSH OUTPUT FROM BACKGROUND PROCESS FINISHING: MYSH TERMINATED: %s",
                                getbgCmd(childpid) );
                        logger(logMsg);
                        free(logMsg);
                    }
                    else if (WIFSTOPPED(status))
                    {/* process stopped for some reason */
                        printf("MYSH STOPPED: %s",getbgCmd(childpid));
                        /* log the message NOT AN ERROR!!!*/
                        logMsg = malloc(sizeof(char)*BUFF_SIZE);
                        memset(logMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
                        sprintf(logMsg,
                                "MYSH OUTPUT FROM BACKGROUND PROCESS FINISHING: MYSH STOPPED: %s",
                                getbgCmd(childpid));
                        logger(logMsg);
                        free(logMsg);
                    }
                    /* end robbins code */

                    deletebgPid(childpid);/* delete this pid from the list */
                }
            }
        }


        do
        {
            write(STDOUT_FILENO,prompt,promptLen);  /* print out the prompt */
            memset(buf,'\0',BUFF_SIZE); /* clear the buffer with NULL */
            memset(pathBuf,'\0',BUFF_SIZE); /* clear the path buffer with NULL */
            /* print the cwd after the prompt*/
            if(getcwd(pathBuf,(size_t)BUFF_SIZE) == NULL)
            {
                fprintf(stderr, "get current working directory failed: %s\n",
                        strerror(errno));
            }
            pathBufLen = strlen(pathBuf);
            if(pathBufLen > 0)
            {
                write(STDOUT_FILENO," (",2);
                write(STDOUT_FILENO,pathBuf,pathBufLen);  /* print out the cwd */
                write(STDOUT_FILENO,")\n",2);
                write(STDOUT_FILENO,"> ",2);
            }
            else
            {
                write(STDOUT_FILENO,"> ",2);
            }
            read(STDIN_FILENO,buf,BUFF_SIZE);  /* get first argument */
        }
        while(strcmp(buf,"\n") == 0);  /* re-prompt if just a return */



        /* log the input message NOT AN ERROR!!!*/
        logMsg = malloc(sizeof(char)*BUFF_SIZE);
        memset(logMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
        sprintf(logMsg, "MYSH INPUT FROM THE PROMPT: %s",buf);
        logger(logMsg);
        free(logMsg);

        /* put in argument array */
        if ((numtokens = makeargv(buf, delim, &myargv)) == -1)
        {
            errorMsg = malloc(sizeof(char)*BUFF_SIZE);
            memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
            sprintf(errorMsg,
                    "MYSH ERROR: Failed to construct an argument array for %s\n", buf);
            logger(errorMsg);
            fprintf(stderr, "%s",errorMsg);
            free(errorMsg);
            return 1;
        }

    }  /* end  while(strcmp(myargv[0],exitStr) != 0) */

    //redundant check but...
    if(strcmp(myargv[0],exitStr) == 0)
    {
        if(numtokens == 1)
        {
            //printf("exiting 0\n");
            exit(0);
        }
        else
        {
            //printf("exiting with second argument\n");
            exit(atoi(myargv[1]));
        }
    }
    //end check if exit

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    printf("SHOULD NEVER REACH THIS CODE!!!!!!!!!11\n");
    errorMsg = malloc(sizeof(char)*BUFF_SIZE);
    memset(errorMsg,'\0',sizeof(char)*BUFF_SIZE); /* clear the buffer with NULL */
    sprintf(errorMsg,
            "MYSH ERROR: Code should never reach this message at the end of main\n");
    logger(errorMsg);
    fprintf(stderr, "%s",errorMsg);
    free(errorMsg);

    return 0;


} /* end main() */

/* ************************* makeargv ********************* */
/*  taken from Robbins text program 2.2 but modified where indicated*/
int makeargv(const char *s, const char *delimiters, char ***argvp)
{
    int error;
    int i;
    int numtokens;
    const char *snew;
    char *t;

    /* added by Todd Lanis */
    //check if *s is a single argument with no delimiters
    int sLen = strlen(s);

    /* end add */

    if ((s == NULL) || (delimiters == NULL) || (argvp == NULL))
    {
        errno = EINVAL;
        return -1;
    }
    *argvp = NULL;
    snew = s + strspn(s, delimiters);         /* snew is real start of string */
    if ((t = malloc(strlen(snew) + 1)) == NULL)
        return -1;
    strcpy(t, snew);
    numtokens = 0;
    if (strtok(t, delimiters) != NULL)     /* count the number of tokens in s */
        for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++)
            ;

    /* create argument array for ptrs to the tokens */
    if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL)
    {
        error = errno;
        free(t);
        errno = error;
        return -1;
    }
    /* insert pointers to tokens into the argument array */
    if (numtokens == 0)
        free(t);
    else
    {
        strcpy(t, snew);
        **argvp = strtok(t, delimiters);
        for (i = 1; i < numtokens; i++)
            *((*argvp) + i) = strtok(NULL, delimiters);
    }
    *((*argvp) + numtokens) = NULL;             /* put in final NULL pointer */

    /* Todd added this to remove cariage return from last element */
    char *temp =  *((*argvp) + numtokens - 1); //argvp[numtokens - 1];
    sLen = strlen(temp);
    temp[sLen - 1] = '\0'; //remove cariage return
    //argvp[numtokens - 1]
    *((*argvp) + numtokens - 1) = temp;
    /* end of add */



    /* fix the ending-with-spaces problem */
    while(strcmp(*((*argvp) + numtokens - 1), "") == 0){
    	*((*argvp) + numtokens - 1) = NULL;
    	numtokens--;
    }

    return numtokens;
}

/* ************************ prints the argument array ************************  */
void printArgArray(char **myargv, int numtokens)
{
    int i;
    printf("The argument array for this buf contains:\n");
    for (i = 0; i < numtokens; i++)
        printf("%d:%s\n", i, myargv[i]);
}

/* **************************** logger *************************************** */
/* used to log errors if the file is specified by the user by
 *   setting the environment variale MYSH_DEBUG 
 */
void logger(char *logMsg)
{
    if(getenv("MYSH_DEBUG") == NULL)
    {
        /* do nothing, the debug file is not set */
        return;
    }
    /* else unset the variable */
    else
    {
        char *fileName = getenv("MYSH_DEBUG");

        int errorSize = strlen(logMsg);
        char* path = malloc(sizeof(char)*1024);
        sprintf(path,"%s%c",fileName,'\0');
        FILE* error_log = fopen(path,"a"); // "w" or open with "a" for append?????s
        if(error_log == NULL)
        {
            fprintf(stderr,"MYSH ERROR: logger file %s\nfailed to open: %s\n",
                    fileName,strerror(errno));
            /* unset the MYSH_DEBUG variable */
            if(unsetenv("MYSH_DEBUG") == -1)
            {
                fprintf(stderr,
                        "MYSH ERROR: failed to unset MYSH_DEBUG variable after the file failed to open\n");
            }
            return;
        }

        else
        {
            fwrite(logMsg,sizeof(char),errorSize,error_log);
            fflush(error_log);
            fclose(error_log);
            free(path);
            return;
        }
    }
    fprintf(stderr,"MYSH ERROR: returning from logger from a point that should never be reached!\n");
    return;
}


/* **************************** addbgPid *************************************** */
/* adds the background process id and what command it had to arrays for future reference */
void addbgPid(int pid, char *cmd)
{
    int i;
    char *temp;
    temp = malloc(sizeof(char)*strlen(cmd));
    strcpy(temp,cmd);
    for(i = 0; i < 128; i++)
    {
        if(bgPids[i] == 0)
        {
            bgPids[i] = pid;
            bgCmds[i] = temp;
            return;
        }
    }
}

/* **************************** deletebgPid *************************************** */
/* deletes the background process id and what command it had to arrays */
void deletebgPid(int pid)
{
    int i;
    for(i = 0; i < 128; i++)
    {
        if(bgPids[i] == pid)
        {
            bgPids[i] = 0;
            free(bgCmds[i]);
            return;
        }
    }
}

/* **************************** getbgCmd *************************************** */
/* returns the command of the process with the id passed in */
char* getbgCmd(int pid)
{
    int i;
    for(i = 0; i < 128; i++)
    {
        if(bgPids[i] == pid)
        {
            return(bgCmds[i]);
        }
    }
    return NULL;
}
