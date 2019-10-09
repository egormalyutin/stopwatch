#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>

// I haven't even read K&R so please don't kill me

struct termios saved;

void restore() {
	printf(" (exit)");

	// Show cursor
	printf("\e[?25h\n");
	
	// Restore stdin
	tcsetattr(STDIN_FILENO, TCSANOW, &saved);

	exit(0);
}

void main() {
	// Display help
	printf("Press Enter, Space or S to save current time\n");
	printf("Press Tab or R to save current time and reset timer\n");
	printf("Press Ctrl+C or Q to exit\n");

	// Restore when user hits ctrl+c
	signal(SIGINT, restore);

	// Disable cursor
	printf("\e[?25l");

	// Disable "^C" and disable buffered stdin in terminal
	struct termios attributes;
	tcgetattr(STDIN_FILENO, &saved);

	tcgetattr(STDIN_FILENO, &attributes);
	attributes.c_lflag &= ~ICANON & ~ECHO;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &attributes);

	// Checking stdin for input
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	fd_set savefds = readfds;

	setbuf(stdin, NULL);

	// Select timeout
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// Initial time
	struct timeval stv;
	gettimeofday(&stv, NULL);

	// Current time
	struct timeval tv;
	struct tm *ptm;

	// Sleep time
	struct timespec ts;
	ts.tv_sec = 0;

	// Last pressed character
	char chr = 1;

	while (1) {
		gettimeofday(&tv, NULL);
		
		// Get time passed from start
		timersub(&tv, &stv, &tv);

		ptm = gmtime((const time_t *) & tv.tv_sec);
		
		// Print time
		printf("\e[2K\r%02d:%02d:%02d.%06d",
			ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tv.tv_usec);
		fflush(stdout);

		// copied from https://stackoverflow.com/a/37271281
		if (chr != EOF) {
			int sel_rv = select(1, &readfds, NULL, NULL, &timeout);
			if (sel_rv > 0) {
				chr = getchar();

				if (chr == 'q') {
					// Exit
					restore();
				} else if (chr == '\n' || chr == 's' || chr == ' ') {
					// Save current time :D
					printf(" (save) \n");
				} else if (chr == '\t' || chr == 'r') {
					// Save current time
					printf(" (reset) \n");
					// Reset initial time
					gettimeofday(&stv, NULL);
				}

			} else if (sel_rv == -1) {
				perror("select failed");
			}

			readfds = savefds;
		}

		// Sleep some milliseconds
		ts.tv_nsec = 60 * 1000000;
		nanosleep(&ts, &ts);
	}
}
