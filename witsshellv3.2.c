#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <fcntl.h>

bool containsChar(char *str, char target) {
    while (*str) {
        if (*str == target) {
            return true;
        }
        str++;
    }
    return false;
}

bool findChar(char *text[], int size, char target, int *targetIndex) {
    for (int i = 0; i < size; i++) {
        if (containsChar(text[i], target)) {
			*targetIndex = i;
            return true;
        }
    }
    return false;
}

void printComm(char *text[], int words) {
	for(int i=0; i<words; i++) {
		printf("%d: %s\n", i,text[i]);
	}
}
void printPath(char **pathArray, int pathSize) {
	if(pathArray==NULL){
		printf("In print path and pathArray is NULL\n");
	}
	else{
		for(int i=0; i<pathSize; i++) {
			printf("%d: %s\n", i,pathArray[i]);
		}
	}

}


int splitInput(char input[], char *text[]) {
	char *token;
	char *rest = input; // Pointer to track the remaining string
	int i = 0;
	int words = 0;

	while ((token = strsep(&rest, " \t")) != NULL) {
		if (*token != '\0') {  // Ignore empty tokens (in case of consecutive spaces)
			text[i++] = token;
			words++;
		}
	}
	text[i] = NULL;
	return words;
}

void CheckAndRemoveChars(char *input) {
	if(input[strcspn(input, "\n")] == '\n') {
		input[strcspn(input, "\n")] = '\0';
	}
}
void throwError() {
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
}

void addPath(char **pathArray, char *text[], int pathSize, int startPoint){
	//size="len" of pathArray
	for(int i=startPoint;i<pathSize;i++){
		//allocate memory for each arg
		pathArray[i] = malloc(40*sizeof(char));
		strcpy(pathArray[i],text[i+1-startPoint]);
	}
}

void emptyPath(char **pathArray, int* pathSize){
	if(pathArray ==NULL){
		throwError();
		// printf("In emptyPath and pathArray is null\n");
	}
	else{		
		for(int i=0;i<*pathSize;i++){
			free(pathArray[i]);
			pathArray[i]=NULL;
		}
		free(pathArray);
		*pathSize = 0;
	}
}

int execCommand(char *command, char*text[],char**pathArray,int pathSize){
	if(pathArray==NULL){
		//printf("exec comm error path null\n");
		throwError();
		//return 1;
		//printf("In execCommand and pathArray is null\n");
	}
	else{
		for(int i =0;i<pathSize;i++){
			if(access(pathArray[i], X_OK)==0){ //found command and is exe
			//make new process
				int pid = fork();
				// if(pid<0){
				// 	throwError();
				// 	//printf("exec comm error pid<0\n");
				// 	//return 1;
				// }
				// else 
				if(pid ==0){ //child
					//printf("dis da arr: %s\n",pathArray[i]);
					//check if execv returned
					//if(execv(pathArray[i],text)==-1){
					if(execv(strcat(pathArray[i],text[0]),text)==-1){
						
						//printf("execv returned\n");
						//continue;
						//throwError();
						
					}
					//printf("child, otside if\n");
					exit(1);
				}
				else if(pid>0){ //parent
					int status;
					wait(&status);
					// if(WIFEXITED(status)){
					// 	//printf("Child process exited with status %d\n", WEXITSTATUS(status));
					// 	continue;
					// }
					// else{
					// 	//printf("Child process did not exit normally\n");
					// 	continue;
					// }
					// return 0;
					//wait(NULL);
					//printf("parent after wait\n");
					//exit(0);
				}

			}
			else{
				continue;
			}
		}
	}

}
void outputRedirection(char *filename){
	//if fd =-1 open failed
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC,0644);
	if(fd<0){
		throwError();
	}
	if(dup2(fd, STDOUT_FILENO) < 0){
		throwError();
	}
	close(fd);
}



int main(int MainArgc, char *MainArgv[]){
	char prompt[] = "witsshell> ";
	char defPath[]="/bin/";
	//allocate memory for pathArray
	int pathSize = 1;
	int startPoint;
	int maxStrLength=1024;
	char **pathArray = malloc(pathSize *sizeof(char *));
	pathArray[0] = malloc(maxStrLength*sizeof(char));
	strcpy(pathArray[0],defPath);
	char *sh = ".sh";
	char target = '>';
	int targetIndex;

	//interactive mode --> start loop
	if(MainArgc ==1) {
		while(1) {
			printf("%s",prompt);
	//get input
			char *input;
			size_t size;
			char *text[1024];//array of pointers
			const ssize_t result = getline(&input, &size, stdin);

			if(result != -1) { //successful read
				// check and remove newline character
				CheckAndRemoveChars(input);

				//if empty then just disregard that input
				if(strcmp(input,"")==0) {
					continue;
				}

				//split input
				const int words = splitInput(input, text);
				

				//get command
				char *command = text[0];


				if(strcmp(command,"exit")==0) {
					if(words>1) {
						throwError();
					}
					else {
						exit(0);
						break;
					}
				}
				else if(strcmp(command,"cd")==0) {
					if(words!=2) {
						throwError();
					}
					else {
						//change directory
						int cdResult = chdir(text[1]);
						if(cdResult==-1){//chdir failed
							throwError();
						}

					}
				}
				else if(strcmp(command, "path")==0) {
					//change the path
					//overwrite defPath 
					//check if text[1] is NULL, then the only argument was "path" and path should be empty
					if(text[1]==NULL){

						emptyPath(pathArray,&pathSize);
						pathArray =NULL;

					}
					else{
						//store old pathSize for starting point in addPath
						startPoint = pathSize;
						//update path size
						pathSize=words-1+pathSize; //minus 1 for the command
						//reallocate pathArray memory with new pathsize
						char** newarr = realloc(pathArray, pathSize*sizeof(char *));
						if(newarr ==NULL){//realloc didnt work
							throwError();
							//printf("Realloc didnt work\n");
						}
						else{
							pathArray = newarr;
						}
						//add args to search path
						addPath(pathArray,text,pathSize,startPoint);
					}
				}
				else if((strcmp(command, "printpath")==0)){
					printPath(pathArray,pathSize);
				}
				else { //not "built-in"
					//check if path has been set
					if((pathSize<=1 ) && strstr(command,sh)!=NULL){//path hasnt been set and arg is shell script
						//printf("shell script error\n");
						throwError();
					}
					else{
						if(findChar(text,words,target,&targetIndex)){
							//redirection command
							if(targetIndex!=words-2){
								//> is not the second last thing so throw error
								throwError();
							}
						}
						else{
							execCommand(command,text,pathArray,pathSize);
						}
						
					}
				}
			}
			else { //EOF
				printf("\n");
				exit(0);
			}
		}
	}
	//batch mode

	//get filename
	char *filename = MainArgv[1];
	FILE *file = fopen(filename, "r");

	if(file == NULL) {
		//perror("Error opening file");
		throwError();
	}
	else{
		//read the lines
		char *line = NULL;
		char *inputLine[1024];
		size_t len = 0;
		ssize_t read;
		while((read= getline(&line, &len, file))!= -1) {//for each line
			CheckAndRemoveChars(line);
			const int len_words = splitInput(line,inputLine);

			//get command
			char *command = inputLine[0];
			//exec command
			if(strcmp(command,"exit")==0) {
				if(len_words>1) {
					throwError();
				}
				else {
					exit(0);
				}
			}
			else if(strcmp(command,"cd")==0) {
				if(len_words!=2) {
					throwError();
				}
				else {
					//change directory
					int cdResult = chdir(inputLine[1]);
					if(cdResult==-1){//chdir failed
						throwError();
					}

				}
			}
			else if(strcmp(command, "path")==0) {
				//change the path
				//overwrite defPath 
				//check if text[1] is NULL, then the only argument was "path" and path should be empty
				if(inputLine[1]==NULL){

					emptyPath(pathArray,&pathSize);
					pathArray =NULL;

				}
				else{
					//store old pathSize for starting point in addPath
					startPoint = pathSize;
					//update path size
					pathSize=len_words-1+pathSize; //minus 1 for the command
					//reallocate pathArray memory with new pathsize
					char** newarr = realloc(pathArray, pathSize*sizeof(char *));
					if(newarr ==NULL){//realloc didnt work
						throwError();
						printf("Realloc didnt work\n");
					}
					else{
						pathArray = newarr;
					}
					//add args to search path
					addPath(pathArray,inputLine,pathSize,startPoint);
				}
			}
			else if((strcmp(command, "printpath")==0)){
				printPath(pathArray,pathSize);
			}
			else { //not "built-in"
				//check if path has been set
				if((pathSize<=1 ) && strstr(command,sh)!=NULL){//path hasnt been set and arg is shell script
					//printf("shell script error\n");
					throwError();
					
				}
				else{
					if(findChar(inputLine,len_words,target,&targetIndex)){
						//redirection command
						if(targetIndex!=len_words-2){
							//> is not the second last thing so throw error
							throwError();
						}
					}
					else{
						execCommand(command,inputLine,pathArray,pathSize);
					}
					
				}
			}		
		}
		fclose(file);
		exit(0);
	}


	return(0);
}