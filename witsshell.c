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

int splitInput(char input[], char *text[]) {
	char *token;
	char *rest = input; // Pointer to track the remaining string
	int i = 0;
	int words = 0;

	while ((token = strsep(&rest, " ")) != NULL) {
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


int main(int MainArgc, char *MainArgv[]){
	char prompt[] = "witsshell> ";

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

				}
				else { //not "built-in"

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
		ssize_t read;
		while((read = getline(&line, &len, file)) != -1) {//for each line
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

