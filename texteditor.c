#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

struct termios orig_termios;

void die(const char *s){
    perror(s);
    exit(1);
}

void disableRawMode(){
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1){
        die("tcsetattr");
    }
}

void enableRawMode(){

    struct termios raw = orig_termios;

    if(tcgetattr(STDIN_FILENO, &raw) == -1){
        die("tcgetattr");
    }

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    atexit(disableRawMode);

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1){
        die("tcsetattr");
    }
}

int main(){

    enableRawMode();

    while(1){
        char c = '\0';
        read(STDIN_FILENO, &c, 1);
        while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
            if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN){
                die("tcseattr");
            }
            if(iscntrl(c)){
                printf("%d\r\n", c);
            }
            else{
                printf("%d ('%c')\r\n");
            }
            if(c == 'q'){
                break;
            }
        }
        disableRawMode();
        return 0;

    }
}