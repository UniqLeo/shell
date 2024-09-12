#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <fcntl.h>

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

void addPath(char **pathArray, char *text[], int pathSize){
	//size="len" of pathArray
	for(int i=1;i<pathSize;i++){
		//allocate memory for each arg
		pathArray[i] = malloc(40*sizeof(char));
		strcpy(pathArray[i],text[i]);
	}
}
void emptyPath(char **pathArray, int pathSize){
	printf("size: %d\n",pathSize);
	if(pathArray ==NULL){
		printf("path already null in empty path\n");
	}
	else{
		
		
		for(int i=0;i<pathSize;i++){
			free(pathArray[i]);
		}
		free(pathArray);
		pathArray = NULL;
		pathSize = 0;
		if(pathArray==NULL){
			printf("path null & size: %d\n",pathSize);
		}
	}
	
}

int execCommand(char *command, char*text[],char**pathArray,int words){
	for(int i =0;i<words;i++){
		if(access(pathArray[i], X_OK)==0){ //found command and is exe
		//make new process
			int pid = fork();
			if(pid<0){
				printf("pid <0\n");
				throwError();
				return 1;
			}
			else if(pid ==0){ //child
				printf("in child\n");
				//check if execv returned
				if(execv(pathArray[i],text)==-1){
					printf("execv returned\n");
					throwError();
					exit(1);
				}
				
			}
			else{ //parent
				int status;
				printf("in parent\n");
				wait(&status);
				if(WIFEXITED(status)){
					printf("Child process exited with status %d\n", WEXITSTATUS(status));
				}
				else{
					printf("Child process did not exit normally\n");
				}
				return 0;
			}

		}
	}
}


int main(int MainArgc, char *MainArgv[]){
	char prompt[] = "witsshell> ";
	char defPath[]="/bin/";
	//allocate memory for pathArray
	int pathSize = 1;
	int maxStrLength=1024;
	char **pathArray = malloc(pathSize *sizeof(char *));
	pathArray[0] = malloc(maxStrLength*sizeof(char));
	strcpy(pathArray[0],defPath);
	

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

				//check "built-ins"

				if(strcmp(command,"exit")==0) {
					if(words>1) {
						throwError();
					}
					else {
						exit(0);
					}
				}
				else if(strcmp(command,"cd")==0) {
					if(words!=2) {
						throwError();
					}
					else {
						//do the thing

					}
				}
				else if(strcmp(command, "path")==0) {
					//change the path
					//overwrite defPath 
					//check if text[1] is NULL, then the only argument was "path" and path should be empty
					if(text[1]==NULL){
						//strcpy(defPath,"");
						//printf("%s\n",defPath);
						//make pathArray empty
						printf("before empty path\n");
						printf("size: %d\n",pathSize);
						emptyPath(pathArray,pathSize);
						printf("after empty path, before print path\n");
						printPath(pathArray,pathSize);
						printf("after printpath\n");
					}
					else{
						//update path size
						pathSize=words+pathSize;
						//reallocate pathArray memory with size of words
						pathArray = realloc(pathArray, (pathSize)*sizeof(char *));
						//add args to search path
						addPath(pathArray,text,pathSize);
						printPath(pathArray,pathSize);
					}
					//which path mus defpath be overwritten with???
					//strcpy(defPath,text[1]);
					//printf("arg is: %s\n defPth: %s\n",text[1],defPath);
					//printf("%s",defPath);

				}
				else { //not "built-in"
					printPath(pathArray, pathSize);
					execCommand(command,text,pathArray,pathSize);
				}
			}
			else { //EOF
				exit(0);
			}
		}
	}
	//batch mode

	//get filename
	char *filename = MainArgv[1];
	FILE *file = fopen(filename, "r");

	if(file == NULL) {
		perror("Error opening file");
	}
	else{
		//read the lines
		char *line = NULL;
		char *inputLine[1024];
		size_t len = 0;
		ssize_t read = getline(&line, &len, file);
		while(read!= -1) {//for each line
			CheckAndRemoveChars(line);
			const int len_words = splitInput(line,inputLine);

			//get command
			char *command = inputLine[0];
			//exec command
			printf("command: %s\n", command);
			if(strcmp(command,"exit")==0) {
				exit(0);
			}
		}
		fclose(file);
		exit(0);
	}


	return(0);
}

