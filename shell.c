//Current version is not having support of pipe , However i have intiated it by mentioning some function but it is not completely Done 
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

enum
BUILTIN_COMMANDS { NO_SUCH_BUILTIN=0, EXIT,JOBS};

void printDir() {
    char pwd[1024];
    getcwd(pwd, sizeof(pwd));
    printf("%s ", pwd);
}


struct history {
  char * command  ; 
  struct history * next ;  
  struct history * previous  ; 
};

int parseString(char * cmdLine , char **parsedArgs , char **parsedArgsPiped , struct history * current , struct history * head) ; 
struct history *  runCommand(char * cmdLine ,struct history * head , struct history * current  ) ;
int parse(char * cmdLine , char **parsedArgs)  ;
struct history * getNode(char * cmdLine){
    int i ; 
    struct history * historyNode  = (struct history *)malloc(sizeof(struct history)) ; 
    historyNode->command = (char *)malloc(strlen(cmdLine)*sizeof(char));
    for (i = 0; i < strlen(cmdLine) ; ++i) {
           historyNode->command[i] = cmdLine[i] ;
    } 
    fprintf(stdout, "%s\n",historyNode->command );
    historyNode->next = historyNode->previous = NULL ; 
    return historyNode ; 
}

char * returnDir() {
    char* pwd ;
    pwd = (char * )malloc(1024*sizeof(char))  ;  
    getcwd(pwd , sizeof(pwd));
    return pwd ;  
}


char *
buildPrompt() { 
    char * buf ; 
    printDir(); 
    buf  = readline(">>> ");
    if (strlen(buf) != 0 ) {
        return buf ; 
    }
    else {
      return NULL ; 
    }
}
 
int 
checkParsePiped(char * cmdLine , char **cmdLineParsed) {
    int i;
    for (i = 0; i < 2; i++) {
        cmdLineParsed[i] = strsep(&cmdLine, "|");
        if (cmdLineParsed[i] == NULL)
            break;
    }
 
    if (cmdLineParsed[1] == NULL) // No pipe found b**chhh 
        return 0; 
    else {
        return 1;
    }
}

void
rmtree(const char path[])
{
    size_t path_len;
    char *full_path;
    DIR *dir;
    struct stat stat_path, stat_entry;
    struct dirent *entry;

    stat(path, &stat_path);

    if (S_ISDIR(stat_path.st_mode) == 0) {
        fprintf(stderr, "%s: %s\n", "Is not directory", path);
        exit(-1);
    }

  
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
        exit(-1);
    }

    path_len = strlen(path);

    while ((entry = readdir(dir)) != NULL) {

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        full_path = calloc(path_len + strlen(entry->d_name) + 1, sizeof(char));
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);

        stat(full_path, &stat_entry);

        if (S_ISDIR(stat_entry.st_mode) != 0) {
            rmtree(full_path);
            continue;
        }

        if (unlink(full_path) == 0)
            printf("Removed a file: %s\n", full_path);
        else
            printf("Can`t remove a file: %s\n", full_path);
    }

    if (rmdir(path) == 0)
        printf("Removed a directory: %s\n", path);
    else
        printf("Can`t remove a directory: %s\n", path);

    closedir(dir);
}

int
checkParseAndBuffer(char **parsedArgs , char * buffer) {
    char * newParsedArgs[1000];
    parse(buffer , newParsedArgs) ; 
    int i = 0 ; 
    int pole = 1 ;
    while(newParsedArgs[i] != NULL && parsedArgs[i] != NULL ){
      if (strcmp(newParsedArgs[i] , parsedArgs[i]) != 0 ){
        pole = 0 ;
      }
      i++;
    }
    return pole  ;
}


int 
inBuiltCmd(char **parsedArgs  , struct history * current , struct history * head) {
    int noOfInbuiltCommand = 7 , i ,j = -1 , pole  ; 
    char * pwd ; 
    char buf[1024];
    DIR *dp;
    struct dirent *sd;
    char * inBuiltCommands [8] ;
    struct history * temp  ; 
    int historyTraverse = 0 , k  , x ;
    char * buffer , * buffer2; 
    char * newParsedArgs[1000] , *newParsedArgsPiped[1000] ; 
    inBuiltCommands[0]= (char *)"cd" ; 
    inBuiltCommands[1] = (char *)"ls" ; 
    inBuiltCommands[2] = (char *)"rm" ;
    inBuiltCommands[3] = (char *)"history" ;
    inBuiltCommands[4] = (char *)"issue" ; 
    inBuiltCommands[5] = (char *)"exit" ;
    inBuiltCommands[6] = (char *)"help" ; 
    inBuiltCommands[7] = (char *)"rmexcept" ; 
    for ( i = 0; i < 8; ++i) {
      if ( strcmp(parsedArgs[0] , inBuiltCommands[i]) == 0 ) {
        j = i ;
        break ; 
      }
    }

    if (j == -1) {
      return 0 ; 
    }
    switch (j){
      case 0: // for changing current directory 
            if (parsedArgs[1] == NULL) {
              fprintf(stderr, "Directory Not mentioned for cd");
            }
            else {
              chdir(parsedArgs[1]) ; 
            }
            return 1 ; 
      case 1: // for printing the files in current directory
            getcwd(buf ,sizeof(buf));
            dp=opendir(buf); 
            while((sd=readdir(dp))!=NULL) {
                printf("%s\n",sd->d_name);
            }
            closedir(dp);
            return 1 ; 
      case 2: // for removing particular file 
              if (parsedArgs[1] == NULL) {
                fprintf(stderr, "File name not mentioned \n" );
              }
              else {
                if (strcmp(parsedArgs[1] , "-v") == 0 ){
                  if (remove(parsedArgs[2]) <0 ) {
                    fprintf(stderr, "Error : Unable to delete the file\n" );
                    fprintf(stdout, "Removing File : %s\n",parsedArgs[2]);
                  }
                }
                else if (strcmp(parsedArgs[1] , "-r") == 0 ){
                  rmtree(parsedArgs[2]) ; 
                }
                else if (strcmp(parsedArgs[1] , "-f") == 0 ){
                  if (remove(parsedArgs[2]) <0 ) {
                    fprintf(stderr, "Error : Unable to delete the file\n" );
                  }
                }
                else {
                  if (remove(parsedArgs[1]) <0 ) {
                    fprintf(stderr, "Error : Unable to delete the file\n" );
                  }
                }
              }
            return 1 ;
      case 3:
            temp = current ; 
            if (temp->command == NULL) {
              printf("No commands in history \n");
              return 1 ; 
            }

            if (parsedArgs[1]== NULL || (strlen(parsedArgs[1]) == 0 ) ){
              temp = head->next ; 
              while(temp != NULL) {
                printf("%s\n" , temp->command);
                temp = temp->next ; 
              }
            }
            else {
                historyTraverse = 0 ;
                x = strlen(parsedArgs[1]) ;
                for(k = 0  ; k < x ; k++) {
                  historyTraverse = (historyTraverse)*10 +  (int)(parsedArgs[1][k] - '0') ;  
                }
                while(temp->command != NULL & (historyTraverse > 1 ) ) {
                  //printf("%s\n" , temp->command);
                  temp = temp->previous ;
                  historyTraverse--;  
                }
                while(temp != NULL) {
                printf("%s\n" , temp->command);
                temp = temp->next ; 
              }
            }
            return 1 ;          
      case 4: 
            temp = current ; 
            if (parsedArgs[1] == NULL) {
              printf("Please mention which command to run \n");
              return 1 ; 
            }
            historyTraverse = 0 ;
            x = strlen(parsedArgs[1]) ;
            for(k = 0  ; k < x ; k++) {
                historyTraverse = (historyTraverse)*10 +  (int)(parsedArgs[1][k] - '0') ;  
            }
            while(temp->command != NULL & (historyTraverse > 1 ) ) {
                  temp = temp->previous ;
                  historyTraverse--;  
            }
            if (temp->command == NULL && historyTraverse >= 1 ){
              printf("Command used till Now are less than the argument passed \n");
            }
            else {
              buffer = (char *)malloc(strlen(temp->command)*sizeof(char));
              strcpy(buffer , temp->command);
              buffer2 = (char *)malloc(strlen(temp->command)*sizeof(char));
              strcpy(buffer2 , temp->command);
              if(checkParseAndBuffer(parsedArgs , buffer2)){
                fprintf(stderr, "Current issue cannot be ran ,Due to recursive nature \n" );
                return 1 ; 
              }
              runCommand(buffer  ,  head, current );
              current->next = NULL ; 
            }
            return 1 ;         
      case 5: 
            exit (1);
            return 1 ; 
      case 6: 
            fprintf(stdout, "Manual : \n inBuiltCommands : \n 1. cd : To change Current Directory \n 2. ls \n 3. rm \n 4. history \n 5. issue \n 6. exit \n7. rmexcept \n8. help \n  ");
            return 1 ; 
      case 7 :
            getcwd(buf ,sizeof(buf));
            dp=opendir(buf); 
            while((sd=readdir(dp))!=NULL) {
                i = 1 ; 
                pole = 1 ; 
                while(parsedArgs[i] != NULL){
                  if (strcmp(parsedArgs[i] , sd->d_name) == 0  ){
                   pole = 0  ; 
                  }
                  i++ ; 
                }
                if (pole){
                  remove(sd->d_name) ; 
                }
            }
            closedir(dp); 
            return 1 ;                  
      default :

            break ;  
     }            
}

int 
parse(char * cmdLine , char **parsedArgs) {
    int i  ; 
    for (int i = 0; i < 1000; ++i) {
      parsedArgs[i] = strsep(&cmdLine , " "); 

      if (parsedArgs[i] == NULL) {
          break ; 
      }

      if(strlen(parsedArgs[i]) == 0 ) {
        i--; 
      }
    }
}

int 
parseString(char * cmdLine , char **parsedArgs , char **parsedArgsPiped , struct history * current , struct history * head) {
    char * strpiped[2] ; 
    int piped  = 0 ;
    piped = checkParsePiped(cmdLine , parsedArgsPiped) ; 

    if (piped) {
        parse(strpiped[0] , parsedArgs) ; 
        parse(strpiped[1] , parsedArgsPiped);
    }else {
       parse(cmdLine , parsedArgs) ; 
    }

    if (inBuiltCmd(parsedArgs ,  current, head)) {
      return 0 ; 
    }
    else{
      return 1 + piped ; 
    }
}


void kill_child(int sig) {
    kill(getpid(),SIGKILL);
}

void 
execArgs (char ** parsedArgs) {
  signal(SIGALRM,(void (*)(int))kill_child);
  pid_t pid = fork() ; 
  int m = 0 ; // Variable for storing timeout 
  int pole = 0 ; 
  if (pid == -1) {
      fprintf(stderr, "Failed to execute the command \n");
  }
  if (pid == 0 ) {
      int i = 1 ; 
      int in, out;
      while(parsedArgs[i] != NULL) {
        if (strcmp(parsedArgs[i] , ">") ==0 ){
          i++ ; 
          in = open(parsedArgs[i], O_RDONLY);
          dup2(in , 0 ) ; 
        }
        else if (strcmp(parsedArgs[i] , "<") == 0 ) {
          i++ ; 
          out = open(parsedArgs[i], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
          dup2(out , 1) ; 
        }
        else {
          int x = strlen(parsedArgs[i]);
          int k ; 
            for(k = 0  ; k < x ; k++) {
                m = (m)*10 +  (int)(parsedArgs[i][k] - '0') ;  
            }
            pole =1 ; 
        }
        i++ ; 
      }
      close(in);
      close(out);
      if (pole) {
        alarm(m);
      }
    if (execvp(parsedArgs[0] , parsedArgs )< 0 ) {
          fprintf(stderr, "Failed to execute the command \n");
          return  ; 
    }
  }
  else {
      wait(NULL) ; 
      return ; 
  }
}

struct history *  
runCommand(char * cmdLine ,struct history * head , struct history * current  ) {
  int flag = 0 ;
  char *parsedArgs[1000] , *parsedArgsPiped[1000] ;
   
  flag = parseString(cmdLine , parsedArgs , parsedArgsPiped , current ,head) ; 
  
  if (flag == 1) { // No pipeing is present in the files 
    execArgs(parsedArgs) ; 
  }
  if (flag == 2) { // pipeing is present ... Currently not Involved  
    
  }
}

int 
main (int argc, char **argv)
{
  char * cmdLine  ;
#ifdef __linux__
    fprintf(stdout, "This is the LINUX version\n");
#endif
#ifdef WINDOWS
    fprintf(stdout, "This is the WINDOWS version\n");
#endif

    fprintf(stdout, "**************************************************************************************************\n*                                    CS 341 - Assignment Shell                                  *\n**************************************************************************************************\n");

  struct history * head  = (struct history * )malloc(sizeof(struct history)); 
  struct history * current ; 
  head->command = NULL ;  
  head->next = head->previous = NULL ; 
  current = head ;
 
 while(1){
    char * __buffer =NULL; 
#ifdef __linux__
    cmdLine = buildPrompt();
    if (cmdLine == NULL) {
      fprintf(stderr, "Unable to read command\n");
      continue;
    }
#endif  
    int length = strlen(cmdLine);
    int i ; 
    __buffer = (char *)malloc(length*sizeof(char));
    /*for (i = 0; i < strlen(cmdLine) ; ++i) {
           __buffer[i] = cmdLine[i] ;
    }*/
    strcpy(__buffer , cmdLine);
    //fprintf(stdout, "%s\n",__buffer );


    runCommand(cmdLine , head , current );

    struct history * historyNode  = (struct history *)malloc(sizeof(struct history)) ; 
    historyNode->command = (char *)malloc(length*sizeof(char));
    strcpy(historyNode->command , __buffer) ; 
    //fprintf(stdout, "%s\n",__buffer );
    //fprintf(stdout, "%s\n",historyNode->command );
    historyNode->next = NULL ; 
    current->next = historyNode ; 
    historyNode->previous = current ;
    current = historyNode  ;
    //fprintf(stdout, "%s\n",current->command );
    free(cmdLine);
  }
  return 0 ; 
}
  





