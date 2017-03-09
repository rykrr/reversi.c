#include <stdio.h>
#include <ncurses.h>

#define BSIZE 26
#define ACHAR 'X'
#define BCHAR 'O'
#define ECHAR '-'

struct chk {
    int val;
    int pts;
};

struct {
    int points[BSIZE*BSIZE][3];
    int length;
} pts;

struct {
    int x;
    int y;
} dim;

void init_curses() {
    initscr();
    curs_set(0);
    nonl();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    
    getmaxyx(stdscr, dim.x, dim.y);
    
    if(dim.y < BSIZE+4 || dim.x < BSIZE+2) {
        endwin();
        printf("Screen too small to play game\n");
    }
}

void new_board(char board[BSIZE][BSIZE], int size) {
    
    for(int i=0; i<size; i++)
        for(int j=0; j<size; j++)
            board[i][j] = ECHAR;
    
    board[size/2  ][size/2  ] = ACHAR;
    board[size/2-1][size/2  ] = BCHAR;
    board[size/2  ][size/2-1] = BCHAR;
    board[size/2-1][size/2-1] = ACHAR;
}

void print_board(char board[BSIZE][BSIZE], int size) {
    
    for(int i=-1; i<size; i++) {
        
        if(0<=i) printf("%c ", i+97);
        else printf("  ");
        
        for(int j=0; j<size; j++) {
            
            if(i<0) printf("%c ", j+97);
            else printf("%c ", board[i][j]);
        }
        
        printf("\n");
    }
}

void check(char board[BSIZE][BSIZE], int size, int x, int y, int dx, int dy, char p, int *mask, int clevel, int flip, struct chk *result) {
    
    if(!clevel) {
        int m = 1;
        for(int i=-1; i<2; i++)
            for(int j=-1; j<2; j++)
                if(i||j)
                    check(board, size, x, y, i, j, p, &m, 1, flip, result);
        
        if(!flip && result->val) {
            pts.points[pts.length][0]   = x;
            pts.points[pts.length][1]   = y;
            pts.points[pts.length++][2] = result->val;
        }
        else if(flip && result->val) {
            board[x][y] = p;
        }
    }
    else {
        int cx=x+dx, cy=y+dy;
        int inv = 0, end = 0, fin = 0;
        
        while(!fin && 0<=cx && 0<=cy && cx<BSIZE && cy<BSIZE) {
            
            if(board[cx][cy] == p) {
                end++;
                fin++;
            }
            else if(board[cx][cy] != ECHAR) {
                inv++;
                
                if(flip==-1 && ((result->val>>*mask)&1))
                    board[cx][cy] = p;
            }
            else {
                fin++;
            }
            
            if(flip!=-1 && end && inv) {
                result->val |= 1<<*mask;
                result->pts += inv;
            }
            
            cx+=dx;
            cy+=dy;
        }
        
        if(flip==1)
            check(board, size, x, y, dx, dy, p, mask, 1, -1, result);
        (*mask) <<= 1;
    }
}

int main(void) {
    
    char board[BSIZE][BSIZE];
    int size = 8;
    
    do {
        printf("Enter a size (<%d): ", BSIZE);
        scanf("%d", &size);
    } while(size<8 || BSIZE<size);
    
    if(size%2!=0) size--;
    new_board(board, size);
    print_board(board, size);
    
    struct chk out = {0,0};
    check(board, size, 1, 3, 0, 0, BCHAR, 0, 0, 1, &out);
    print_board(board, size);
}
