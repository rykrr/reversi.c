/* Ryan Kerr :: Reversi.c           */
/* Copyright (c) Ryan Kerr 2017     */

/* Controls                         */
/* Space:   Select                  */
/* h,j,k,l: Move cursor             */
/* arrows:  Move cursor             */
/* q:       Quit                    */

/* F1:      Help                    */
/* F2:      Toggle Hard Mode        */
/* 1:       Decrease delay          */
/* 2:       Increase delay          */

#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#ifndef MAX_BOARD_SIZE
#define MAX_BOARD_SIZE 50
#endif

#define ESP '-'
#define XSP 'X'
#define YSP 'O'

#ifndef DEFAULT_MODE
#define DEFAULT_MODE 0
#endif

#define STATUS(f)\
    attron(A_REVERSE|A_BOLD); f;\
    attroff(A_REVERSE|A_BOLD)

#define COLOR(f,a)\
    if(has_colors()) {\
        attron(COLOR_PAIR(a)); f;\
        attroff(COLOR_PAIR(a));\
    }

/* GLOBAL */
char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];

int X = 0, Y = 0;

int BOARD_SIZE = MAX_BOARD_SIZE;

int TOP_PAD = 0, LEFT_PAD = 0;

int mode = DEFAULT_MODE%4;

unsigned int delay = 1;

int CNUL[2];

struct chk {
    int inv;
    int end;
    int val;
    int ptn;
};

struct {
    int x;
    int y;
} dim;

struct {
    int cor[MAX_BOARD_SIZE*MAX_BOARD_SIZE][3];
    int len;
} ptlist;


/* PROTOTYPES */
int  shift(int, int, int);
void poten(int, int, int, int, int*);
void swchk(int, int, char, int*, int*, int*, int, int*);

void new_board();
void print_cursor(int, int);
void print_board();
int  check_board();
void clear_row();
int  cond_mode(int,int,int);

struct chk chk_d(int, int, char, int);
struct chk chk_x(int, int, char, int);
struct chk chk_y(int, int, char, int);

int  check(int, int, char, int*);
void qkchk(char);

void fchk(int, int, char, int*);
void flip(int, int, char);
void best(int[2]);
void skip(char p);

void init();
void play();
void quit();
void help();
void end();

int  count(int[2]);

//void predict(int*,int);

/* BEGIN PROGRAM */
int shift(int a, int b, int p) {
    int result = 0;
    for(int i=1; i<=p; i*=2)
        result |= (a&i && b&i?i:0)<<2;
    return result;
}

void poten(int i, int e, int m, int t, int *v) {
    if(i&m && e&m) *v+=t;
}

void swchk(int x, int y, char p, int *i, int *e, int *f, int m, int *t) {
    
    switch(board[x][y]) {
        case ESP:
        case '%':
            *f=-512;
            break;
        case XSP:
            if(p == XSP) { 
                *f=-512;
                *e|=m; 
            }
            else { 
                *i|=m;
                (*t)++;
            }
            break;
        case YSP:
            if(p == YSP) { 
                *f=-512;
                *e|=m; 
            }
            else { 
                *i|=m;
                (*t)++;
            }
            break;
    }
}

void new_board() {
    for(int x=0; x<BOARD_SIZE; x++) {
        for(int y=0; y<BOARD_SIZE; y++) {
            board[x][y] = ESP;
        }
    }
    
    board[BOARD_SIZE/2  ][BOARD_SIZE/2  ] = YSP;
    board[BOARD_SIZE/2  ][BOARD_SIZE/2-1] = XSP;
    board[BOARD_SIZE/2-1][BOARD_SIZE/2  ] = XSP;
    board[BOARD_SIZE/2-1][BOARD_SIZE/2-1] = YSP;
}

void print_cursor(int x, int y) {
    attroff(A_REVERSE|A_BLINK);
    attron(A_BOLD);
    COLOR (
        mvaddch(X+TOP_PAD,Y*2+LEFT_PAD+1, board[X][Y]),
        (board[X][Y]==ESP?0:board[X][Y]==XSP?1:2)
    );
    
    if(x == -1 && 0<=(X-1))
        x = X-1;
    
    if(x == -2 && (X+1)<BOARD_SIZE)
        x = X+1;
    
    if(y == -1 && 0<=(Y-1))
        y = Y-1;
    
    if(y == -2 && (Y+1)<BOARD_SIZE)
        y = Y+1;
    
    if(x<0) x = X;
    if(y<0) y = Y;
    
    attron(A_REVERSE|A_BLINK|A_BOLD);
    mvaddch(x+TOP_PAD,y*2+LEFT_PAD+1, board[x][y]);
    attroff(A_REVERSE|A_BLINK|A_BOLD);
    
    X = x;
    Y = y;
    
    refresh();
}

void print_board() {
    attron(A_BOLD);
    for(int x=0; x<BOARD_SIZE; x++) {
        for(int y=0; y<BOARD_SIZE; y++) {
            mvprintw(x+TOP_PAD, y*2+LEFT_PAD, " ");
            COLOR (
                mvprintw(x+TOP_PAD, y*2+LEFT_PAD+1, "%c", board[x][y]),
                (board[x][y]==ESP?0:board[x][y]==XSP?1:2)
            );
        }
        printw("\n");
    }
    attroff(A_BOLD);
    
    #ifndef AUTO_TEST
    print_cursor(X, Y);
    #endif
    refresh();
}

int check_board() {
    for(int x=0; x<BOARD_SIZE; x++)
        for(int y=0; y<BOARD_SIZE; y++)
            if(board[x][y] == ESP)
                return 0;
    return 1;
}

void fchk(int x, int y, char p, int *f) {
    if(board[x][y] == p)
        *f = -512;
    else if(board[x][y] != ESP)
        board[x][y] = p;
    else
        *f = -512;
}

#define DCHK(c,k) (-1<c && -1<k && c<BOARD_SIZE && k<BOARD_SIZE)

void dchk(struct chk *out, int x, int y, int dx, int dy, int *mask, char p, int g) {
    
    int c = x+dx, k = y+dy, fin=0;
    int ptn = 0;
    
    while(DCHK(c,k) && !fin) {
        
        if(g<=0) {
            swchk(c, k, p, &(out->inv), &(out->end), &fin, *mask, &ptn);
        }
        else {
            if(out->inv&*mask&&out->end&*mask)
                fchk(c, k, p, &fin);
        }
        
        c += dx;
        k += dy;
    }
    
    if(g<0)
        dchk(out, x, y, dx, dy, mask, p, 1);
    
    poten(out->inv, out->end, *mask, ptn, &(out->ptn));
    out->val = shift(out->inv, out->end, *mask);
    *mask <<= 1;
}

struct chk chk_x(int x, int y, char p, int g) {
    struct chk out = {0, 0, 0, 0};
    int mask = 1;
    
    dchk(&out, x, y, -1, 0, &mask, p, g);
    dchk(&out, x, y,  1, 0, &mask, p, g);
    
    return out;
}

struct chk chk_y(int x, int y, char p, int g) {
    struct chk out = {0, 0, 0, 0};
    int mask = 1;
    
    dchk(&out, x, y, 0, -1, &mask, p, g);
    dchk(&out, x, y, 0,  1, &mask, p, g);
    
    return out;
}

struct chk chk_d(int x, int y, char p, int g) {
    struct chk out = {0, 0, 0, 0};
    int mask = 1;
    
    dchk(&out, x, y, -1, -1, &mask, p, g);
    dchk(&out, x, y,  1,  1, &mask, p, g);
    dchk(&out, x, y, -1,  1, &mask, p, g);
    dchk(&out, x, y,  1, -1, &mask, p, g);
    
    return out;
}

int check(int x, int y, char p, int *s) {
    if(!(0 <= x && x < BOARD_SIZE) 
    || !(0 <= y && y < BOARD_SIZE))
        return -1;
    
    struct chk _x = chk_x(x, y, p, 0);
    struct chk _y = chk_y(x, y, p, 0);
    struct chk _d = chk_d(x, y, p, 0);
    
    *s = _x.ptn + _y.ptn + _d.ptn;
    
    if(_x.val || _y.val || _d.val)
        return 1;
    
    return 0;
}

void qkchk(char p) {
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
        for(int j=0; j<3; j++)
            ptlist.cor[i][j] = 0;
    ptlist.len = 0;
    
    for(int x=0; x<BOARD_SIZE; x++) {
        for(int y=0; y<BOARD_SIZE; y++) {
            int cont = 0;
            
            if(board[x][y] == ESP) {
                for(int i=-1; i<2; i++) {
                    if(-1<(x+i)&&(x+i)<BOARD_SIZE) {
                        
                        for(int j=-1; j<2; j++) {
                            
                            if(-1<(y+j)&&(y+j)<BOARD_SIZE) {
                                
                                if(!(i==0&&j==0) && board[x+i][y+j]==(p==XSP?YSP:XSP)) {
                                    cont++;
                                }
                            }
                        }
                    }
                }
            }
            
            int s = 0;
            if(cont && 0<check(x, y, p, &s)) {
                ptlist.cor[ptlist.len][0]   = x;
                ptlist.cor[ptlist.len][1]   = y;
                ptlist.cor[ptlist.len++][2] = s;
            }
        }
    }
}

void flip(int x, int y, char p) {
    chk_x(x, y, p, -1);
    chk_y(x, y, p, -1);
    chk_d(x, y, p, -1);
}

#define CORNER(c,i) (ptlist.cor[i][2]!=0 && c<ptlist.cor[i][2])

int corner() {
    int corner = -1;
    int cpoten = 0;
    
    for(int i=0; i<BOARD_SIZE; i++) {
        if(ptlist.cor[i][0]==0
        && ptlist.cor[i][1]==0) {
            if(CORNER(cpoten,i)) {
                cpoten=ptlist.cor[i][2];
                corner=0;
            }
        }
        if(ptlist.cor[i][0]==BOARD_SIZE-1
        && ptlist.cor[i][1]==0) {
            if(CORNER(cpoten,i)) {
                cpoten=ptlist.cor[i][2];
                corner=1;
            }
        }
        if(ptlist.cor[i][0]==0
        && ptlist.cor[i][1]==BOARD_SIZE-1) {
            if(CORNER(cpoten,i)) {
                cpoten=ptlist.cor[i][2];
                corner=2;
            }
        }
        if(ptlist.cor[i][0]==BOARD_SIZE-1
        && ptlist.cor[i][1]==BOARD_SIZE-1) {
            if(CORNER(cpoten,i)) {
                cpoten=ptlist.cor[i][2];
                corner=3;
            }
        }
    }
    return corner;
}

int cond_mode(int b, int s, int a) {
    switch(mode) {
        case 0:
            return b>s;
        case 1:
            return ((s-1)==a || s==a || (s+1)==a);
        default:
            return b<s;
    }
}

int bias(int s, int i) {
    int good = 0;
    int bad  = 0;
    
    for(int j=0; j<2; j++) {
        int c = ptlist.cor[i][j];
        if(c == 1 || c == BOARD_SIZE-2) {
            s = 0;
        }
        if(c < (BOARD_SIZE)/2-1) {
            if(c%2==0) good++;
            else bad++;
        }
        else if(c < (BOARD_SIZE)/2) {
            if(c%2==1) good++;
            else bad++;
        }
    }
    
    if(good==2) s *= 2;
    return s;
}

void best(int points[2]) {
    int p = ptlist.len;
    int a = 0;
    
    int best = (p?ptlist.cor[0][2]:0);
    points[0] = ptlist.cor[0][0];
    points[1] = ptlist.cor[0][1];
    
    if(mode == 1) {
        for(int i=0; i<p; i++)
            a += ptlist.cor[i][2];
        a = a / p;
    }
    
    int c = 0;
    if(mode == 2) {
        c = corner();
        switch(c) {
            case 0:
                points[0] = 0;
                points[1] = 0;
                break;
            case 1:
                points[0] = BOARD_SIZE-1;
                points[1] = 0;
                break;
            case 2:
                points[0] = 0;
                points[1] = BOARD_SIZE-1;
                break;
            case 3:
                points[0] = BOARD_SIZE-1;
                points[1] = BOARD_SIZE-1;
                break;
            default:
                break;
        }
    }
    
    if(mode!=2 || c == -1) {
        for(int i=(mode<3?0:(p==1?0:p-1)); i<p; i++) {
            int s = ptlist.cor[i][2];
            if(mode == 2) s = bias(s,i);
            if(cond_mode(best,s,a)) {
                best = s;
                points[0] = ptlist.cor[i][0];
                points[1] = ptlist.cor[i][1];
            }
        }
    }
}

void init() {
    initscr();
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
    nonl();
    cbreak();
    
    getmaxyx(stdscr, dim.x, dim.y);
    
    if(dim.y < BOARD_SIZE+4 || dim.x < BOARD_SIZE+4) {
        endwin();
        printf("Screen too small to play game\n");
        exit(1);
    }
    else {
        TOP_PAD  = (dim.x-BOARD_SIZE)/2;
        LEFT_PAD = (dim.y)/2 - BOARD_SIZE;
    }
    
    if(has_colors()) {
        start_color();
        init_pair(1, COLOR_RED,  COLOR_BLACK);
        init_pair(2, COLOR_BLUE, COLOR_BLACK);
    }
    
    for(int x=0; x<dim.x; x++)
        for(int y=0; y<dim.y; y++)
            mvprintw(x,y, " ");
    
    attron(A_REVERSE|A_BOLD);
    for(int y=0; y<dim.y; y++) {
        mvprintw(0, y, " ");
        mvprintw(dim.x-1, y, " ");
    }
    mvprintw(0,0, " Reversi.c");
    mvprintw(0, dim.y-15, "Ryan Kerr 2017");
    attroff(A_REVERSE|A_BOLD);
    
    print_board();
    
    count(CNUL);
}

void clear_row(int x) {
    for(int y=0; y<dim.y; y++)
        mvprintw(x, y, " ");
}

void skip(char p) {
    STATUS (clear_row(dim.x-1));
    STATUS (
        mvprintw(dim.x-1, 1, 
                "No moves, turn goes to %c. "
                "(Press any key)", p)
    );
    getch();
    STATUS(clear_row(dim.x-1));
    count(CNUL);
}

void help() {
    for(int x=1; x<dim.x-1; x++)
        clear_row(x);
    
    mvprintw(2,  1, "F1:        Help");
    mvprintw(3,  1, "F2:        Toggle Hard Mode");
    mvprintw(5,  1, "h,j,k,l:   Move Cursor");
    mvprintw(6,  1, "Arrows:    Move Cursor");
    mvprintw(7,  1, "Space:     Select");
    mvprintw(9,  1, "1:         Decrease Delay");
    mvprintw(10, 1, "2:         Increase Delay");
    mvprintw(12, 1, "q:         Quit");
    
    STATUS (clear_row(dim.x-1));
    STATUS (
        mvprintw(dim.x-1, 1, "Press Any Key To Continue.")
    );
    
    getch();
    for(int x=1; x<dim.x-1; x++)
        clear_row(x);
    STATUS (clear_row(dim.x-1));
    count(CNUL);
    print_board();
}

void end() {
    count(CNUL);
    STATUS (clear_row(dim.x-1));
    if(CNUL[0]==CNUL[1]) {
        STATUS (
            mvprintw(dim.x-1, 1, "Draw! [ X: %d | O: %d ] (Press any key)",
                    CNUL[0], CNUL[1])
        );
    }
    else {
        STATUS (
            mvprintw(dim.x-1, 1, "%c wins! [ X: %d | O: %d ] (Press any key)",
                    CNUL[0]>CNUL[1]?XSP:YSP, CNUL[0], CNUL[1])
        );
    }
    getch();
    STATUS (clear_row(dim.x-1));
    STATUS (
        mvprintw(dim.x-1, 1, "Play Again? (y/n)");
    );
    
    char c = 0;
    do { c = getch(); } while(!(c!='y' || c!='n'));
    
    endwin();
    
    if(c == 'n')
        exit(0);
    new_board();
    init();
}

void quit() {
    STATUS (clear_row(dim.x-1));
    STATUS (
        mvprintw(dim.x-1, 1, "Are you sure? (y/n)");
    );
    
    char c = 0;
    do { c = getch(); } while(!(c != 'y' || c != 'n'));
    
    if(c == 'y') {
        endwin();
        exit(0);
    }
    
    count(CNUL);
}

int count(int score[2]) {
    score[0] = 0;
    score[1] = 0;
    
    for(int x=0; x<BOARD_SIZE; x++) {
        for(int y=0; y<BOARD_SIZE; y++) {
            if(board[x][y] == XSP)
                score[0]++;
            else if(board[x][y] == YSP)
                score[1]++;
        }
    }
    
    STATUS (
        mvprintw(dim.x-1, 1, "[%s] [ %c: %2d | %c: %2d ] ",
                (mode==0?"EASY":mode==1?"AVG"
                :mode==2?"HARD":"MISC"),
                XSP, score[0], 
                YSP, score[1]);
    );
    
    return (score[0] == 0 || score[1] == 0);
}

void play() {
    
    int no_op = 0;
    int e = 0;
    new_board();
    print_board();
    
    count(CNUL);
    
    while(!check_board() && no_op != 4 && !e) {
        qkchk(XSP);
        
        #ifdef AUTO_TEST
        
        usleep(AUTO_TEST);
        if(0<ptlist.len && !e) {
            int bst[2];
            best(bst);
            board[bst[0]][bst[1]] = XSP;
            flip(bst[0], bst[1], XSP);
            no_op = 0;
        }
        else {
            skip(YSP);
            no_op++;
        }
        
        #else
        
        if(0<ptlist.len) {
            int c;
            int fi = 0;
            int go = 0;
            
            no_op = 0;
            
            do {
                c = getch();
                
                switch(c) {
                    case KEY_RESIZE:
                        endwin();
                        init();
                        break;
                        
                    case KEY_F(1):
                        help();
                        break;
                        
                    case KEY_F(2):
                        mode = (mode+1)%4;
                        count(CNUL);
                        break;
                        
                    case KEY_LEFT:
                    case 'h':
                        print_cursor(X, -1);
                        break;
                    
                    case KEY_RIGHT:
                    case 'l':
                        print_cursor(X, -2);
                        break;
                    
                    case KEY_UP:
                    case 'k':
                        print_cursor(-1, Y);
                        break;
                    
                    case KEY_DOWN:
                    case 'j':
                        print_cursor(-2, Y);
                        break;
                    
                    case 10:
                    case ' ':
                        for(int i=0; i<ptlist.len; i++)
                            if(ptlist.cor[i][0] == X
                            && ptlist.cor[i][1] == Y)
                                go = 1;
                        
                        if(go)
                            mvprintw(dim.x-1, dim.y-10, "  Valid  ");
                        else
                            mvprintw(dim.x-1, dim.y-10, " Invalid ");
                        
                        break;
                    
                    case '1':
                        if(1<delay)
                            delay--;
                        break;
                    
                    case '2':
                        delay++;
                        break;
                    
                    case 'q':
                        quit();
                        break;
                }
                
                refresh();
                
            } while(!go);
            
            board[X][Y] = XSP;
            flip(X, Y, XSP);
            
            e = count(CNUL);
        }
        else if(!e) {
            no_op++;
            skip(YSP);
        }
        #endif
        
        print_board();
        
        #ifdef AUTO_TEST
        usleep(AUTO_TEST);
        #else
        sleep(delay);
        #endif
        
        qkchk(YSP);
        if(0<ptlist.len && !e) {
            no_op = 0;
            
            int bst[2];
            best(bst);
            board[bst[0]][bst[1]] = YSP;
            flip(bst[0], bst[1], YSP);
            
            count(CNUL);
        }
        else if(!e) {
            no_op++;
            skip(XSP);
        }
        
        print_board();
        refresh();
    }
    
    end();
}

int main(int argc, char* argv[]) {
    printf("Reversi.c :: Ryan Kerr 2017\n");
    printf("Press F1 for in-game help\n");
    
    int size = 0;
    do { 
        printf("Please Enter An Even Board Size (<=%d): ", MAX_BOARD_SIZE);
        scanf("%d", &size);
    } while(!(4 < size && size <= MAX_BOARD_SIZE));
    BOARD_SIZE = size;
    new_board();
    init();
    for(;;) play();
}
