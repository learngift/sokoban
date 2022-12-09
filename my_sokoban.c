#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

const int ERROR = 84;

const void usage() {
    fprintf(stderr,
    "USAGE\n"
    "     Usage ./my_sokoban map\n"
    "DESCRIPTION\n"
    "     map file representing the warehouse map, containing ‘#’ for walls,\n"
    "         ‘P’ for the player, ‘X’ for boxes and ‘O’ for storage locations.\n");
}

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

int main(int argc, char * argv[]) {
    const char * map;

    if (argc != 2) {
        usage();
        return ERROR;
    } else if  (strcmp(argv[1], "-h") == 0) {
        usage();
        return 0;
    }

    map = read_map(argv[1]);
    printf("%s", map);
    return 0;
}