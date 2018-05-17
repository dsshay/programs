#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

int out_char = 0, counter = 128;
pid_t pid, ppid;

void usage(char* name);
int openfile(char* inputFileName, char* outputFileName);
int copyfile(char* inputFileName, char* outputFileName, int inputFD, int outputFD);
void childexit(int sig);
void parentexit(int sig);
void empty(int sig);
void one(int sig);
void zero(int sig);

int main(int argc, char** argv) {

	if (argc != 3)
	{
		fprintf(stderr, "Wrong count of parameters:\n");
		usage (argv[0]);
		return -1;
	}

	return openfile (argv[1], argv[2]);
}

void usage(char* name) {
	fprintf(stderr, "Usage: %s inputFileName, outputFileName\n", name);
}

int openfile(char* inputFileName, char* outputFileName) {

	int inputFD = -1, outputFD = -1;
	int status = 1;

	inputFD = open(inputFileName, O_RDONLY);
	if (inputFD == -1) {
		fprintf(stderr, "inputfileName '%s' open error: %s\n", inputFileName, strerror(errno));
		status = -1;
		goto hell;
	}

	outputFD = open(outputFileName, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (outputFD == -1) {
		fprintf(stderr, "outputfileName '%s' open error: %s\n", outputFileName, strerror(errno));
		status = -1;
		goto hell;
	}

	status = copyfile(inputFileName, outputFileName, inputFD, outputFD);

hell:
	close(inputFD);
	close(outputFD);
	return status;
}

int copyfile(char* inputFileName, char* outputFileName, int inputFD, int outputFD) {

	int r;

	ppid = getpid();

	sigset_t set;

	struct sigaction opexit, opone, opzero;

	memset(&opexit, 0, sizeof(opexit));
	memset(&opone, 0, sizeof(opone));
	memset(&opzero, 0, sizeof(opzero));

	opexit.sa_handler = childexit;

	sigfillset(&opexit.sa_mask); 		
	r = sigaction(SIGCHLD, &opexit, NULL);
	if (r < 0) {
		fprintf (stderr, "Unable to determine how to process the signal SIGCHLD");
		exit (1);
	}

	opone.sa_handler = one;
	sigfillset(&opone.sa_mask);
	r = sigaction(SIGUSR1, &opone, NULL);
	if (r < 0) {
		fprintf (stderr, "Unable to determine how to process the signal SIGUSR1");
		exit (1);
	}

	opzero.sa_handler = zero;
	sigfillset(&opzero.sa_mask);
	r = sigaction(SIGUSR2, &opzero, NULL);
	if (r < 0) {
		fprintf (stderr, "Unable to determine how to process the signal SIGUSR2");
		exit (1);
	}

	sigemptyset(&set);
	if (r < 0) {
		fprintf (stderr, "SIGEMPTYSET: %m\n");
		exit (1);
	}

	r = sigaddset(&set, SIGUSR1);
	if (r < 0) {
		fprintf (stderr, "SIGADDSET SIGUSR1: %m\n");
		exit (1);
	}
	r = sigaddset(&set, SIGUSR2);
	if (r < 0) {
		fprintf (stderr, "SIGADDSET SIGUSR2: %m\n");
		exit (1);
	}
	r = sigaddset(&set, SIGCHLD);
	if (r < 0) {
		fprintf (stderr, "SIGADDSET SIGСHLD: %m\n");
		exit (1);
	}

	r = sigprocmask(SIG_BLOCK, &set, NULL );	
	if (r < 0) {
		fprintf (stderr, "sigprocmask: %m\n");
		exit (1);
	}
	sigemptyset(&set);

	pid = fork();
	if (pid < 0){
		fprintf (stderr, "fork: %m\n");
		exit (1);
	}
	if (pid == 0) {

		struct sigaction opempty, opalarm;
		memset(&opalarm, 0, sizeof(opalarm));
		memset(&opempty, 0, sizeof(opempty));

		opempty.sa_handler = empty;

		sigfillset(&opempty.sa_mask);
		r = sigaction(SIGUSR1, &opempty, NULL);
		if (r < 0) {
			fprintf (stderr, "Unable to determine how to process the signal SIGUSR1 on child process");
			exit (1);
		}

		opalarm.sa_handler = parentexit;
		sigfillset(&opalarm.sa_mask);
		r = sigaction(SIGALRM, &opalarm, NULL);
		if (r < 0) {
			fprintf (stderr, "Unable to determine how to process the signal SIGALARM");
			exit (1);
		}

		int bytesIn = 1;
		r = 1;

		while (1) {
			r = read(inputFD, &bytesIn, 1);
			if (r < -1) {
			    fprintf(stderr, "Can't read file %s error: %s\n", inputFileName, strerror(errno));
			    exit (1);
			}

			if (r == 0) {
				break;
			}

			alarm(1);

			for ( int i = 128; i >= 1; i /= 2) {
				if (i & bytesIn)
					kill(ppid, SIGUSR1);
				else
					kill(ppid, SIGUSR2);
				sigsuspend(&set);
			}
		}
	exit (EXIT_SUCCESS);
	}

	while (1) {
			sigsuspend(&set);
			if(counter == 0) {
				r = write(outputFD, &out_char, 1);
				if (r < 0){
					fprintf(stderr, "Can't write in file %s error: %s\n", outputFileName, strerror(errno));
				}
				counter = 128;
				out_char = 0;
			}
	}

return 0;

}

	void childexit(int sig) {
		exit (0);
	}

	void parentexit(int sig) {
		exit (0);
	}

	void empty(int sig) {
	}

	void one(int sig) {
		out_char += counter;
		counter /= 2;
		kill(pid, SIGUSR1);
	}

	void zero(int sig) {
		counter/=2;
		kill(pid, SIGUSR1);
	}
