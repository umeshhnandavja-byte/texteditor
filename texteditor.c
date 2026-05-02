#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#define CTRL_KEY(k) ((k) & 0x1f)
struct editorconfig{
    int screenrows;
    int screencols;
    struct termios orig_termios;
};

struct editorconfig e;

void initEditor() {
    if (getWindowSize(&e.screenrows, &e.screencols) == -1) die("getWindowSize");
}


void die(const char *s){
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

void disableRawMode(){
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &e.orig_termios) == -1){
        die("tcsetattr");
    }
}

void enableRawMode(){

    struct termios raw = e.orig_termios;

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

int getCursorPosition(int *rows, int *cols) {
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
    printf("\r\n");
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (iscntrl(c)) {
        printf("%d\r\n", c);
        } 
        else {
        printf("%d ('%c')\r\n", c, c);
        }
    }
    editorReadKey();
    return -1;
}

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
        editorReadKey();
        return -1;
    } 
    else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

void editorprocesskeypress(){
    char c = editorreadkey();

    switch(c){
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
    }
}

void editordrawrows(){
    int y;
    for(y = 0; y < e.screenrows; y++){
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorrefreshscreen(){
    write(STDOUT_FILENO, "\x1b[2J" , 4);
    write(STDOUT_FILENO, "\x1b[H" , 3);

    editordrawrows();

    write(STDOUT_FILENO, "\x1b[H" , 3);
}

int main(){

    enableRawMode();
    initEditor();

    while(1){
        editorrefreshscreen();
        editorprocesskeypress();    
    }

    return 0;
}