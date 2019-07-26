#include<sys/wait.h> 
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

int lsh_cd(char **args);	// forward declarations - declare, but not define functions 
int lsh_help(char **args);	// because lsh_help use array of builtins;
int lsh_exit(char **args);

char * builtin_str[] = {		// array of builtin command names 
	"cd",
	"help",
	"exit"
};

int (*builtint_func[]) (char **) = { // array of corresponding functions - can be simply added new functions  
	&lsh_cd,
	&lsh_help,		     // array of function pointers that take array of strings and return an int 
 	&lsh_exit
};

int lsh_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *); 
}

int lsh_cd(char **args) {
	if (args[1] == NULL)
		fprintf(stderr, "lsh: expected argument to \"cd\"\n");
	else { 
		if (chdir(args[0]) != 0 /* i.e. can call system call to this directory */)
			perror("lsh");
	}
	return 1;
}


int lsh_help (char **args) 
{
	int i;
	printf("Konstantin's LSH ");
	printf("The following are builtin ");
	
	for(i = 0; i < lsh_num_builtins(); ++i) {
		printf(" %s\n", builtin_str[i]);	
	}	

	return 1;
}
 
int lsh_exit (char **args) {	// that signal for command loop to terminate 
	return 0; 		
}

char *lsh_read_line(void) {
	int bufsize =  LSH_RL_BUFSIZE;
	int position = 0;
	
	char *buffer = (char *)malloc(sizeof(char) * bufsize);
	int c; // store as int (not char), because EOF in an int
	
	if(!buffer) {
		fprintf(stderr, "lsh: ALLOCATION ERROR\n");
		exit(EXIT_FAILURE);
	}

	while(1) {
		// Read a character from stdin
		c = getchar();
		
		// if hit to EOF/'\n' = replace it witch a null char and return 
		if (c == EOF || c == '\n') {
			buffer[position] = '\0';
			return buffer;
		} else {
			buffer[position] = c;
		}
		position++;

		// if exceed buffer => reallocate 
		if (position >= bufsize) {
			bufsize += LSH_RL_BUFSIZE;
			buffer = (char *) realloc(buffer /* pointer to m that reallocate */, bufsize /* size */);
			if (!buffer) {
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

char **lsh_split_line(char *line) 
{
	int bufsize = LSH_TOK_BUFSIZE, position = 0;
	char **tokens = (char **)malloc(bufsize * sizeof(char *)); // array of pointers of tokens from input == amount equal splitting lines
	char *token;
	
	if(!tokens) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}
	
	token = strtok(line, LSH_TOK_DELIM);
	while (token != NULL /* while have lexemes */) {
		tokens[position] = token;
		position++;
		
		if (position >= bufsize) {
			bufsize += LSH_TOK_BUFSIZE;
			tokens = (char *) realloc(tokens, bufsize * sizeof(char *));
		
			if (!tokens) {
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		
		}
		token = strtok(NULL /* means from place where last success call of function */ , LSH_TOK_DELIM);
	}
	token[position] = NULL;
	return tokens;
}

int lsh_launch(char **args) {
	pid_t pid, wpid;
	int status;
	
	pid = fork();
	if (pid == 0) {
		// Child process
		if (execvp(args[0], args) == -1) {
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		// also erro forking
		perror("lsh");
	} else {
		// parrent process
		do {
			wpid = waitpid(pid, &status, WUNTRACED /* return manage to stopped, stutus of that not notice */ )
		} while (!WIFEXITED(status)/* WIFEXITED - successfully finished */ && !WIFSIGNALED(status) /* WIFSIGNALED - killed by not handled signal */);
	}
	
}

int lsh_execute(char **args)
{
	int i;
	
	// An empty command was entered 
	if (args[0] == NULL) 
		return 1;
	
	for (i = 0; i < lsh_num_builtins(); i++){
		if (strcmp(args[0], builtin_str[i]) == 0)
			return (*builtin_func[i])(args);
	}

	return lsh_launch(args);
}

void lsh_loop(void) {
	char *line;
	char **args;
	int status;

	do {	
		printf(" >");		
		line = lsh_read_line();
		args = lsh_split_line();
		status = lsh_execute(args); // to determine when to exit
		
		free(line);
		free(args);

	} while(status);

}

int main(int argc, char * argv[])
{
	lsh_loop();
}
