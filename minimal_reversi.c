/* Ryan Kerr :: Reversi.c           */
/* Copyright (c) Ryan Kerr 2017     */

/* Controls                         */
/* Space:   Select                  */
/* h,j,k,l: Move cursor             */
/* arrows:  Move cursor             */
/* q:       Quit                    */

/* F1:      Help                    */

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#include "minralg.h"

#define BSIZE 26
#define ACHAR 'X'
#define BCHAR 'O'
#define ECHAR '-'

#define STATUS(f)\
    attron(A_REVERSE|A_BOLD); f;\
    attroff(A_REVERSE|A_BOLD)

#define COLOR(f,a)\
    if(has_colors()) {\
        attron(COLOR_PAIR(a)); f;\
        attroff(COLOR_PAIR(a));\
    }

struct chk {
    int val;
    int pts;
};

struct point {
    int points[BSIZE*BSIZE][3];
    int length;
};

struct board {
    char b[BSIZE][BSIZE];
    int s;
    int x;
    int y;
};

struct scrdim {
    int x;
    int y;
    int t;
    int l;
};

void clear_row(struct scrdim *dim, int row) {
    
    for(int y=0; y<dim->y; y++)
        mvprintw(row, y, " ");
}

void new_board(struct board *brd) {
    
    for(int i=0; i<brd->s; i++)
        for(int j=0; j<brd->s; j++)
            brd->b[i][j] = ECHAR;
    
    brd->b[brd->s/2  ][brd->s/2  ] = ACHAR;
    brd->b[brd->s/2-1][brd->s/2  ] = BCHAR;
    brd->b[brd->s/2  ][brd->s/2-1] = BCHAR;
    brd->b[brd->s/2-1][brd->s/2-1] = ACHAR;
}

void print_cursor(struct board *brd, struct scrdim *dim, int x, int y) {
    
    attroff(A_REVERSE|A_BLINK);
    attron(A_BOLD);
    COLOR (
        mvaddch(dim->t+brd->x, brd->y*2+dim->l+1, brd->b[brd->x][brd->y]),
        (brd->b[brd->x][brd->y]==ECHAR?0:brd->b[brd->x][brd->y]==ACHAR?1:2)
    );
    
    if(!x) x = -3;
    if(!y) y = -3;
    
    if(x == -1 && -1<(brd->x-1))     x = brd->x-1;
    if(y == -1 && -1<(brd->y-1))     y = brd->y-1;
    
    if(x == -2 && (brd->x+1)<brd->s) x = brd->x+1;
    if(y == -2 && (brd->y+1)<brd->s) y = brd->y+1;
    
    if(x<0) x = brd->x;
    if(y<0) y = brd->y;
    
    attron(A_REVERSE|A_BLINK|A_BOLD);
    mvaddch(x+dim->t, y*2+dim->l+1, brd->b[x][y]);
    attroff(A_REVERSE|A_BLINK|A_BOLD);
    
    brd->x=x;
    brd->y=y;
    
    refresh();
}

void print_board(struct board *brd, struct scrdim *dim) {
    
    attron(A_BOLD);
    for(int x=0; x<brd->s; x++) {
        for(int y=0; y<brd->s; y++) {
            COLOR (
                mvprintw(dim->t+x, y*2+dim->l+1, "%c ", brd->b[x][y]),
                (brd->b[x][y]==ECHAR?0:brd->b[x][y]==ACHAR?1:2)
            );
        }
    }
    attroff(A_BOLD);
    refresh();
}

void hint(struct board *brd, struct point *pts, struct scrdim *dim) {
    
    for(int i=0; i<pts->length; i++)
        brd->b[pts->points[i][0]][pts->points[i][1]] = '#';
    print_board(brd, dim);
    
    getch();
    
    for(int i=0; i<pts->length; i++)
        brd->b[pts->points[i][0]][pts->points[i][1]] = ECHAR;
    print_board(brd, dim);
}

void print_score(struct scrdim *dim, const char *status, int a, int b, int c, int d, int e) {
    
    STATUS(clear_row(dim, dim->x-1));
    STATUS(mvprintw(dim->x-1,1, status, a, b, c, d, e));
}

void score(struct board *brd, struct chk *scr, struct scrdim *dim) {
    
    scr->val=scr->pts=0; 
    
    for(int x=0; x<brd->s; x++) {
        for(int y=0; y<brd->s; y++) {
            if(brd->b[x][y]==ACHAR)
                scr->val++;
            else if(brd->b[x][y]==BCHAR)
                scr->pts++;
        }
    }
    
    print_score(dim, "[%c: %d | %c: %d]", ACHAR, scr->val, BCHAR, scr->pts, 0);
}

void game_over(struct board *brd, struct chk *scr, struct scrdim *dim) {
    
    score(brd, scr, dim);
    
    if(scr->pts==scr->val)
        print_score(dim, "Draw! [%c: %d | %c: %d]", ACHAR, scr->val, BCHAR, scr->pts, 0);
    else if(scr->pts<scr->val)
        print_score(dim, "Player %c Wins! [%c: %d | %c: %d]", ACHAR, ACHAR, scr->val, BCHAR, scr->pts);
    else if(scr->val<scr->pts)
        print_score(dim, "Player %c Wins! [%c: %d | %c: %d]", BCHAR, ACHAR, scr->val, BCHAR, scr->pts);
    
    getch();
    getch();
    
    print_score(dim, "Do you want to play again? (y/n)", 0, 0, 0, 0, 0);
    char r = getch();
    
    if(r=='n' || r=='N') {
        endwin();
        exit(0);
    }
}

void quit(struct scrdim *dim) {
    
    print_score(dim, "Are you sure you want to quit? (y/n)", 0, 0, 0, 0, 0);
    char r = getch();
    
    if(r=='y' || r=='Y') {
        endwin();
        exit(0);
    }
}

void init_curses(struct board *brd, struct scrdim *dim) {
    
    dim->x=dim->y=dim->t=dim->l = 0;
    
    initscr();
    curs_set(0);
    nonl();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    
    getmaxyx(stdscr, dim->x, dim->y);
    
    if(dim->y < brd->s+4 || dim->x < brd->s+2) {
        endwin();
        printf("Screen too small to play game\n");
    }
    else {
        dim->t = (dim->x-brd->s)/2;
        dim->l = (dim->y)/2-brd->s;
    }
    
    if(has_colors()) {
        start_color();
        init_pair(1, COLOR_RED,  COLOR_BLACK);
        init_pair(2, COLOR_BLUE, COLOR_BLACK);
    }
    
    for(int x=0; x<dim->x; x++)
        for(int y=0; y<dim->y; y++)
            mvprintw(x,y, " ");
    
    STATUS(clear_row(dim, 0));
    STATUS(clear_row(dim, dim->x-1));
    STATUS(mvprintw(0,0, " Reversi.c"));
    STATUS(mvprintw(0, dim->y-15, "Ryan Kerr 2017"));
    
    print_board(brd, dim);
}

void check(struct board *brd, struct scrdim *dim, struct point *pts,
           int x, int y, int dx, int dy, char p, int *mask, int clevel,
           int flip, struct chk *result) {
    
    if(!clevel) {
        int m = 1;
        result->val=result->pts=0;
        
        for(int i=-1; i<2; i++)
            for(int j=-1; j<2; j++)
                if(i||j)
                    check(brd, dim, pts, x, y, i, j, p, &m, 1, flip, result);
        
        if(!flip && result->val) {
            pts->points[pts->length][0]   = x;
            pts->points[pts->length][1]   = y;
            pts->points[pts->length++][2] = result->val;
        }
        else if(flip && result->val) {
            brd->b[x][y] = p;
        }
    }
    else {
        int cx=x+dx, cy=y+dy;
        int inv = 0;
        
        while(0<=cx && 0<=cy && cx<brd->s && cy<brd->s
              && brd->b[cx][cy] != ECHAR && brd->b[cx][cy] != p) {
            
            inv++;
            
            if(flip==-1 && *mask&result->val)
                brd->b[cx][cy] = p;
            
            cx+=dx;
            cy+=dy;
        }
        
        if(-1<flip && 0<=cx && 0<=cy && cx<brd->s && cy<brd->s
        && brd->b[cx][cy] == p && inv) {
            
            result->val |= *mask;
            result->pts += inv;
        }
        
        if(flip==1)
            check(brd, dim, pts, x, y, dx, dy, p, mask, 1, -1, result);
        
        if(-1<flip)
            (*mask) <<= 1;
    }
}

int check_board(struct board *brd) {
    
    for(int x=0; x<brd->s; x++)
        for(int y=0; y<brd->s; y++)
            if(brd->b[x][y] == ECHAR)
                return 1;
    return 0;
}

void check_all(struct board *brd, struct point *pts, struct scrdim *dim, char c) {
    
    struct chk out = {0,0};
    pts->length = 0;
    
    for(int x=0; x<brd->s; x++)
        for(int y=0; y<brd->s; y++)
            if(brd->b[x][y]==ECHAR)
                check(brd, dim, pts, x, y, 0, 0, c, 0, 0, 0, &out);
}

void loop(struct board *brd, struct point *pts, struct scrdim *dim) {
    char c = 0;
    int no_op = 0;
    struct chk scr = {0,0};
    
    while(check_board(brd) && no_op<2) {
        
        struct chk out = {0,0};
        
        pts->length = 0;
        check_all(brd, pts, dim, ACHAR);
        
        if(0<pts->length) {
            int go = 0;
            no_op = 0;
            
            do {
                switch(c = getch()) {
                    case KEY_RESIZE:
                        endwin();
                        init_curses(brd, dim);
                        break;
                        
                    case 'h':
                        print_cursor(brd, dim, 0, -1);
                        break;
                    
                    case 'l':
                        print_cursor(brd, dim, 0, -2);
                        break;
                    
                    case 'k':
                        print_cursor(brd, dim, -1, 0);
                        break;
                    
                    case 'j':
                        print_cursor(brd, dim, -2, 0);
                        break;
                        
                    case 'p':
                        hint(brd, pts, dim);
                        break;
                        
                    case 'q':
                        quit(dim);
                        break;
                    
                    case 10:
                    case 7:
                    case ' ':
                        for(int i=0; i<pts->length && !go; i++)
                            if(pts->points[i][0] == brd->x
                            && pts->points[i][1] == brd->y)
                                go = 1;
                        
                        if(go)
                            mvprintw(dim->x-1, dim->y-10, "  Valid  ");
                        else
                            mvprintw(dim->x-1, dim->y-10, " Invalid ");
                        
                        break;
                }
            } while(!go);
            
            check(brd, dim, pts, brd->x, brd->y, 0, 0, ACHAR, 0, 0, 1, &out);
            print_board(brd, dim);
            print_cursor(brd, dim, 0, 0);
        }
        else {
            no_op++;
            print_score(dim, "X has no moves, skipping!", 0, 0, 0, 0, 0);
            getch();
        }
        
        score(brd, &scr, dim);
        
        pts->length = 0;
        check_all(brd, pts, dim, BCHAR);
        
        if(!(scr.val&&scr.pts) && check_board(brd))
            break;
        
        if(0<pts->length) {
            
            no_op=0;
            struct chk out = {0,0};
            
            check(brd, dim, pts, pts->points[0][0], pts->points[0][1],
                    0, 0, BCHAR, 0, 0, 1, &out);
            
            print_board(brd, dim);
            refresh();
        }
        else {
            no_op++;
            print_score(dim, "O has no moves, skipping!", 0, 0, 0, 0, 0);
            getch();
        }
        
        print_cursor(brd, dim, 0, 0);
    }
    
    game_over(brd, &scr, dim);
}

int main(void) {
    
    struct board brd;
    struct point pts;
    struct scrdim dim;
    
    brd.x = brd.y = brd.s = 0;
    dim.x = dim.y = dim.t = dim.l = 0;
    pts.length = 0;
    
    do {
        printf("Enter a size (<%d): ", BSIZE);
        scanf("%d", &brd.s);
    } while(brd.s<4 || BSIZE<brd.s);
    
    if(brd.s%2!=0) brd.s--;
    init_curses(&brd, &dim);
    
    for(;;) {
        new_board(&brd);
        print_board(&brd, &dim);
        print_cursor(&brd, &dim, 0, 0);
        
        loop(&brd, &pts, &dim);
    }
}
