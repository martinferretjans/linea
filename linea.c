#include <stdio.h>
#include <termios.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static struct termios old_term, new_term;

void reset_terminal() {
    printf("\e[?25h"); //show cursor
    printf("\e[m"); //reset color changes
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

void configure_terminal() {
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;

    new_term.c_lflag &= ~(ICANON |ECHO);
    new_term.c_cc[VMIN] = 0;
    new_term.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW,&new_term);

    printf("\e[?25l"); //hide cursor
    atexit(reset_terminal);
}

// int read_key(char* buf) {
//     if(buf[0] == '\e' && buf[1] == '[') {
//         switch(buf[2]) {
//             case 'A': return 1;
//             case 'B': return 2;
//             case 'C': return 3;
//             case 'D': return 4;
//         }
//     }
//     return 0;
// }

void readInput() {
    char c;
    char seq[3];
    int nread = read(STDIN_FILENO, &c, 1);
    if(nread != 1) return;
    if(c == 27) {
        if(read(STDIN_FILENO, seq, 1) == 0) {
            printf("ESC");
            return;
        }
        if(seq[0] == 91) {
            if (read(STDIN_FILENO, seq + 1, 1) != 1)
                return;
            switch(seq[1]) {
                case 65: printf("UP\n"); break;
                case 66: printf("DOWN\n"); break;
                case 67: printf("RIGHT\n"); break;
                case 68: printf("LEFT\n"); break;
            }
            return;
        }
    }
    printf("%d\n",c);
}


int main(void) {
    configure_terminal();
    while(1) {
        readInput();
    }
    return 0;
}