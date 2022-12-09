#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> 

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

typedef struct sokoban_map {
    const char * init; /* initial state shall be preserved to allow walking over "O" */
    char * m;
    size_t w, h; /* width and height */
} * SokobanMap;

typedef struct position {
    size_t x, y, pos;
} * Position;

SokobanMap make_map_from_string (const char * m) {
    SokobanMap res = (SokobanMap)malloc(sizeof(struct sokoban_map));
    char * ptr;
    res->init = strdup(m);
    res->m = strdup(m);
    /* remove the player from the reference map */
    ptr = strchr(res->init, 'P');
    *ptr = ' ';
    /* count width */
    ptr = strchr(m, '\n');
    do {
        ptr++;
    } while (*ptr != '#');
    res->w = ptr-m;
    res->h = 0;
}

Position get_player_position(SokobanMap m) {
    Position res = (Position)malloc(sizeof(struct position));
    char * ptr = strchr(m->m, 'P');
    res->pos = ptr-m->m;
    res->x = res->pos % m->w;
    res->y = (res->pos - res->x) / m->w;
    return res;
}

bool isGameFinished(SokobanMap m) {
    const char * po = m->init; /* next storage location */
    do {
        po = strchr(po, 'O');
        if (m->m[po-m->init] != 'X') {
            return false;
        }
    } while (po++);
    return true;
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
    new_x = p->x + dx;
    new_y = p->y + dy;
    if (new_x < 0 || new_y <0 || new_x >= m->w || new_y >= m->h) {
        return;
    }
    new_pos = new_x + new_y * m->w;
    if (m->m[new_pos] == ' ') {
        m->m[new_pos] = 'P';
        m->m[p->pos] = m->init[p->pos];
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
            /* todo check if won */
        }
	} while (!finished);
    endwin();
    return 0;
}