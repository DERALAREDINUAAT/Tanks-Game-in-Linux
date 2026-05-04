#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <string.h>

#define H 20
#define W 40
#define LIFE 3

typedef struct {
    char board[H][W];
    int lifeA;
    int lifeB;
} shared_t;

int shmid;
int semid;
shared_t *sh;

void sem_lock() {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_unlock() {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

void cleanup(int sig) {
    endwin();
    shmdt(sh);
    exit(0);
}

void init_board() {
    for(int i=0;i<H;i++)
        for(int j=0;j<W;j++)
            sh->board[i][j]=' ';

    for(int i=0;i<H;i++){
        sh->board[i][0] = '#';
        sh->board[i][W-1] = '#';
    }
    for(int j=0;j<W;j++){
        sh->board[0][j] = '#';
        sh->board[H-1][j] = '#';
    }

    sh->board[2][2]='A';
    sh->board[H-3][W-3]='B';

    sh->lifeA = LIFE;
    sh->lifeB = LIFE;
}

void find_player(char p, int *y, int *x){
    for(int i=0;i<H;i++)
        for(int j=0;j<W;j++)
            if(sh->board[i][j]==p){
                *y=i;*x=j;
                return;
            }
}

void draw(char me){
    clear();
    for(int i=0;i<H;i++){
        for(int j=0;j<W;j++)
            mvaddch(i,j,sh->board[i][j]);
    }
    mvprintw(H,0,"A life: %d | B life: %d | You: %c",sh->lifeA,sh->lifeB,me);
    refresh();
}

void move_player(char p,int dy,int dx){
    int y,x;
    find_player(p,&y,&x);
    int ny=y+dy, nx=x+dx;
    if(sh->board[ny][nx]==' '){
        sh->board[y][x]=' ';
        sh->board[ny][nx]=p;
    }
}

void shoot(char me,int dy,int dx){
    int y,x;
    find_player(me,&y,&x);
    int by=y+dy, bx=x+dx;
    while(1){
        if(by<=0 || by>=H-1 || bx<=0 || bx>=W-1) break;
        if(sh->board[by][bx]=='#') break;
        if(sh->board[by][bx]=='A' || sh->board[by][bx]=='B'){
            if(sh->board[by][bx]=='A') sh->lifeA--;
            else sh->lifeB--;
            break;
        }
        sh->board[by][bx]='.';
        draw(me);
        usleep(80000);
        sh->board[by][bx]=' ';
        by+=dy;
        bx+=dx;
    }
}

int main(int argc,char **argv){
    if(argc!=8){
        printf("Usage: %s tabla A|B up down left right fire\n",argv[0]);
        return 1;
    }

    char me = argv[2][0];
    int key_up = argv[3][0];
    int key_down = argv[4][0];
    int key_left = argv[5][0];
    int key_right = argv[6][0];
    int key_fire = argv[7][0];

    key_t key = ftok(argv[1],'T');
    shmid = shmget(key,sizeof(shared_t),IPC_CREAT|0666);
    sh = shmat(shmid,NULL,0);

    semid = semget(key,1,IPC_CREAT|0666);
    semctl(semid,0,SETVAL,1);

    sem_lock();
    if(sh->lifeA==0 && sh->lifeB==0) init_board();
    sem_unlock();

    signal(SIGINT,cleanup);
    initscr();
    noecho();
    cbreak();
    keypad(stdscr,TRUE);
    nodelay(stdscr,TRUE);

    int ch, dy=0, dx=1;
    while(1){
        sem_lock();
        draw(me);
        if(sh->lifeA<=0 || sh->lifeB<=0){
            mvprintw(H+1,0,"GAME OVER");
            refresh();
            sem_unlock();
            break;
        }
        sem_unlock();

        ch = getch();
        sem_lock();
        switch(ch){
            case KEY_UP:
            case 0:
            case 'w': if(ch==key_up){dy=-1;dx=0; move_player(me,dy,dx);} break;
            case KEY_DOWN:
            case 's': if(ch==key_down){dy=1;dx=0; move_player(me,dy,dx);} break;
            case KEY_LEFT:
            case 'a': if(ch==key_left){dy=0;dx=-1; move_player(me,dy,dx);} break;
            case KEY_RIGHT:
            case 'd': if(ch==key_right){dy=0;dx=1; move_player(me,dy,dx);} break;
            default: if(ch==key_fire) shoot(me,dy,dx); break;
        }
        sem_unlock();
        usleep(60000);
    }
    cleanup(0);
    return 0;
}
