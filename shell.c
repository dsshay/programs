#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


/*redirection of the standard I / O stream*/
int main(){
	int r;
	char *input_string = NULL;
	size_t input_length = 0;
	r = getline (&input_string, &input_length, stdin);
	if (r < 0){
		fprintf (stderr, "getline: %m\n");
		exit (1);
	}
	
	char *input_words [input_length];
	size_t input_words_count = 0;
	while (1){
		char *word = strtok(input_words_count == 0 ? input_string : NULL, " \n");
		if (word == NULL){
			break;
		}
		input_words [input_words_count] = word;
		input_words_count ++;			
	}
	if (input_words_count == 0){
		fprintf(stderr, "Empty input command\n");
		exit (1);
	}
	
	char **input_commands [input_words_count];
	size_t input_commands_count = 0;
	size_t first_word = 0;
	for (size_t i = 0; i < input_words_count; i ++){
		if (strcmp ("|", input_words[i]) == 0){
			input_commands[input_commands_count] = &input_words[first_word];
			input_words[i] = NULL;
			first_word = i+1;
			input_commands_count ++;
		}		
	}
	input_commands[input_commands_count] = &input_words[first_word];
	input_commands_count ++;
	input_words[input_words_count] = NULL;
	
	int prev_pipe_fds[2] = { -1, -1 }, curr_pipe_fds[2];
	int last_child_pid;

	for (size_t i = 0; i < input_commands_count; ++i){
		for (size_t j = 0;; ++j) {
			if (input_commands[i][j] == NULL) break;
		}
	
		r = pipe(curr_pipe_fds);
		if (r < 0){
			fprintf (stderr, "pipe: %m\n");
			exit (1);
		}

		last_child_pid = fork();
		if (last_child_pid < 0){
			fprintf (stderr, "fork: %m\n");
			exit (1);
		}
		if (last_child_pid == 0){
			if (prev_pipe_fds[0] > 0){
				close(STDIN_FILENO);
				r = dup2(prev_pipe_fds[0], STDIN_FILENO);
				if (r < 0){
					fprintf (stderr, "dup2: %m\n");
					exit (1);
				}
			}

			if (curr_pipe_fds[1] > 0){
				close(STDOUT_FILENO);
				r = dup2(curr_pipe_fds[1], STDOUT_FILENO);
				if (r < 0){
					fprintf (stderr, "dup2: %m\n");
					exit (1);
				}
			}

			close(prev_pipe_fds[0]);
			close(prev_pipe_fds[1]);
			close(curr_pipe_fds[0]);
			close(curr_pipe_fds[1]);

			r = execvp(input_commands[i][0], input_commands[i]);
			fprintf (stderr, "execvp: %m\n");
			exit (1);
		}

		close(prev_pipe_fds[0]);
		close(prev_pipe_fds[1]);
		memcpy(prev_pipe_fds, curr_pipe_fds, sizeof(prev_pipe_fds));
	}
	
	close (prev_pipe_fds[1]);
	
	char buff [1024];
	size_t count = 0;
	while (1){
		ssize_t size = read (prev_pipe_fds[0], buff, 1024);
		if (size == 0){
			break;
		}
		if (size < 0){
			fprintf (stderr, "read: %m\n");
			exit (1);
		}
		count += size;
		r = write (STDOUT_FILENO, buff, size);
		if (r < 0){
			fprintf (stderr, "write: %m\n");
			exit (1);
		}
	}
	fprintf (stderr, "Count of bytes in last program output: %zu\n", count);
	
	int status;
	r = waitpid (last_child_pid, &status, 0);
	if (r < 0){
		fprintf(stderr, "waitpid: %m\n");
		exit (1);
	}
	return status;
}
	 
