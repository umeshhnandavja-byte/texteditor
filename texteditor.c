#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#define CTRL_KEY(k) ((k) & 0x1f)

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

char editorreadkey(){
    int nread;
    char c;
    while((nread = read(STDIN_FILENO, &c, 1) != 1)){
        if(nread == -1 && errno != EAGAIN){
            die("read");
        }
    }
    return c;
}

void editorprocesskeypress(){
    char c = editorreadkey();

    switch(c){
        case CTRL_KEY('q'):
        exit(0);
        break;
    }
}

void editorrefreshscreen(){
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
}

int main(){

    enableRawMode();

    while(1){
        editorrefreshscreen();
        editorprocesskeypress();    
    }

    return 0;
}