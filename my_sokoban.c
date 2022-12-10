#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <unistd.h>

FILE * flog;

const int ERROR = 84;

const void usage() {
    fprintf(stderr,
        "USAGE\n"
        "     Usage ./my_sokoban map\n"
        "DESCRIPTION\n"
        "     map file representing the warehouse map, containing ‘#’ for walls,\n"
        "         ‘P’ for the player, ‘X’ for boxes and ‘O’ for storage locations.\n");
}

/* returns a newly allocated char * containing the content of the file */
char * read_map(const char * filename) {
    long length;
    char * res;
    FILE * f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        exit(ERROR);
    }
    fseek (f, 0, SEEK_END);
    length = ftell(f);
    res = malloc(length + 1);
    if (!res) {
        perror("read_map");
        exit(ERROR);
    }
    fseek (f, 0, SEEK_SET);
    fread(res, 1, length, f);
    res[length] = '\0';
    return res;
}

/* this data representation assumes all the lines have the same length */
typedef struct sokoban_map {
    const char * init; /* initial state shall be preserved to allow walking over "O" */
    char * m; /* current state of the map */
    int w, h; /* width and height */
} * SokobanMap;

typedef struct position {
    int x, y, pos;
} * Position;

SokobanMap make_map_from_string (const char * m) {
    SokobanMap res = (SokobanMap)malloc(sizeof(struct sokoban_map));
    char * ptr;
    res->init = strdup(m);
    res->m = strdup(m);
    /* count width */
    ptr = strchr(m, '\n');
    do {
        ptr++;
    } while (*ptr != '#');
    res->w = ptr-m;
    res->h = strlen(m) / res->w;
    fprintf(flog, "make_map_from_string %ld -> w=%d h=%d\n", strlen(m), res->w, res->h);
    return res;
}

/* allocated a new Position */
Position get_player_position(SokobanMap m) {
    Position res = (Position)malloc(sizeof(struct position));
    char * ptr = strchr(m->m, 'P');
    res->pos = ptr-m->m;
    res->x = res->pos % m->w;
    res->y = (res->pos - res->x) / m->w;
    return res;
}

bool isGameFinished(SokobanMap m) {
    for (int i = 0; m->init[i]; i++) {
        if (m->init[i] == 'O' && m->m[i] != 'X') return false;
    }
    return true;
}

bool isInsideMap(SokobanMap m, int x, int y) {
    return x >= 0 && y >= 0 && x < m->w && y < m->h;
}

void update_map(SokobanMap m, int ch) {
    int dx = 0;
    int dy = 0;
    int new_x;
    int new_y;
    int new_pos;
    Position p = get_player_position(m);
    if (ch == KEY_LEFT) {
        dx = -1;
	} else if (ch == KEY_RIGHT) {
        dx = 1;
	} else if (ch == KEY_UP) {
        dy = -1;
	} else if (ch == KEY_DOWN) {
        dy = +1;
    } else {
        return;
    }
    fprintf(flog, "ch=%d, x=%d, y=%d, dx=%d, dy=%d, pos=%d\n", ch, p->x, p->y, dx, dy, p->pos);
    fflush(flog);
    new_x = p->x + dx;
    new_y = p->y + dy;
    if (!isInsideMap(m, new_x, new_y)) return;
    new_pos = new_x + new_y * m->w;
    if (m->m[new_pos] == ' ' || m->m[new_pos] == 'O') {
        m->m[new_pos] = 'P';
        m->m[p->pos] = m->init[p->pos] == 'O' ? 'O' : ' ';
    } else if (m->m[new_pos] == 'X') {
        int new_x2 = new_x + dx;
        int new_y2 = new_y + dy;
        int new_pos2;
        if (isInsideMap(m, new_x, new_y) && m->m[new_pos2 = new_x2 + new_y2 * m->w] != '#'
                && m->m[new_pos2 = new_x2 + new_y2 * m->w] != 'X') {
            m->m[new_pos2] = 'X';
            m->m[new_pos] = 'P';
            m->m[p->pos] = m->init[p->pos] == 'O' ? 'O' : ' ';
        }
    } else {
        fprintf(flog, "new_pos=%d m=%c\n", new_pos, m->m[new_pos]);
        fflush(flog);
    }
}

int main(int argc, char * argv[]) {
    const char * map_string;
    SokobanMap m;
    bool finished = false;

    if (argc != 2) {
        usage();
        return ERROR;
    } else if  (strcmp(argv[1], "-h") == 0) {
        usage();
        return 0;
    }
    flog = fopen("log.txt", "wt");
 
    map_string = read_map(argv[1]);
    m = make_map_from_string (map_string);
    free((void *)map_string);

    initscr();
    raw();
    keypad(stdscr, true);
    do {
        mvprintw(0, 0, "%s", m->m);
        refresh();
        int ch = getch();
        if (ch == 'r' || ch == 'R') { /* resign */
            finished = true;
        } else {
            update_map(m, ch);
            if (isGameFinished(m)) {
                mvprintw(0, 0, "%s", m->m);
                mvprintw(5, 5, " You have won! ");
                refresh();
                usleep(5000000);
                finished = true;
            }
        }
	} while (!finished);
    endwin();
    return 0;
}
