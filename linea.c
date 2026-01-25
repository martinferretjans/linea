#include <stdio.h>
#include <ctype.h>
#include <termios.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#define CTRL_KEY(k) ((k) & 0x1f)

struct editorConfig{
	struct termios orig_term;
	int screen_rows;
	int screen_cols;
};

struct editorConfig E;

//int exit_loop = 0;

void editorDrawRows(){
	int y;
	for (y = 0; y < E.screen_rows-3; y++){
		write(STDOUT_FILENO, "~\r\n", 3);
	}
}


int getWindowSize(int *rows, int *cols){
	struct winsize ws;

	if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
		return -1;
	} else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

void editorClearScreen(){
	write(STDOUT_FILENO, "\x1b[2J", 4);
	//Cursor pos at 1;1 (\1xB[1;1H)
	write(STDOUT_FILENO, "\x1b[H", 3);
	editorDrawRows();
	write(STDOUT_FILENO, "\x1b[H", 3);
}


void die(const char* e){
	editorClearScreen();
	perror(e);
	exit(1);
}

void disableRawMode() {
    	//printf("\x1B[?25h"); //show cursor
    	//printf("\x1B[m"); //reset color changes
    	//printf("dingdong\n");
    	//fflush(stdout);
    	if(tcsetattr(STDIN_FILENO, TCSANOW, &E.orig_term) == -1) die("tcsetattr");
}

void enableRawMode() {
    
	if(tcgetattr(STDIN_FILENO, &E.orig_term) == -1) die("tcgetattr");
	
	struct termios new_term = E.orig_term;

	//IXON disables CTRL-S and CTRL-Q, ICRNL disables CTRL-M
	new_term.c_iflag &= ~(IXON | ICRNL);
	//OPOST Turns off output processing, so we have to do "\r\n" instead of "\n"
    	new_term.c_oflag &= ~(OPOST); 
	new_term.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
    	new_term.c_cc[VMIN] = 0;
    	new_term.c_cc[VTIME] = 1;
    	if(tcsetattr(STDIN_FILENO, TCSANOW,&new_term) == -1) die("tcsetattr_enable_raw");
    	printf("\x1B[?25l"); //hide cursor
    	atexit(disableRawMode);
}

char editorReadInput() {
	char c;
	int nread;
	while((nread = read(STDIN_FILENO, &c, 1)) <= 0){
		if(nread == -1 && errno != EAGAIN) die("read");
	}
	return c;
}

void editorProcessInput(){
	char c = editorReadInput();
	switch(c){
		case CTRL_KEY('q'):
			editorClearScreen();
			exit(0);
			break;
		default: 
			editorClearScreen(); printf("%c\r\n", c); break;
	}

}


void initEditor(){
	if(getWindowSize(&E.screen_rows, &E.screen_cols) == -1) 
		die("getWindowSize");
}

/*
void signalHandler(){
	exit_loop = 1;
}
*/

int main(void) {
	enableRawMode();
	initEditor();

	//signal(SIGINT, signalHandler);

	while(1){
		//editorClearScreen();
		editorProcessInput();
    	}
	return 0;
}
