// How to run

// $ make
// $ ./mmcc example/lifegame.c > tmp.s
// $ cc -static -o tmp tmp.s
// $ ./tmp

int print_board(int (*life)[20]) {
    for (int i=0; i<20; i=i+1) {
        for (int j=0; j<20; j=j+1) {
            if (life[i][j])
                printf("**");
            else
                printf("..");
        }
        printf("\n");
    }
    printf("\n");
}

int init_board(int (*life)[20]) {
    for (int i=0; i<20; i=i+1)
        for (int j=0; j<20; j=j+1)
            life[i][j] = 0;

    life[8][10] = 1;
    life[9][10] = 1;
    life[10][10] = 1;
    life[11][10] = 1;
    life[12][10] = 1;
    life[8][12] = 1;
    life[9][12] = 1;
    life[10][12] = 1;
    life[11][12] = 1;
    life[12][12] = 1;
}

int nextgen(int (*life)[20], int h, int w) {
    int count = 0;
    for (int y=-1; y<=1; y=y+1) {
        for (int x=-1; x<=1; x=x+1) {
            if (life[h+y][w+x] == 1)
                count = count + 1;
        }
    }

    int nextgen = 0;
    if (life[h][w] == 1) {
        if (count == 2)
            nextgen = 1;
        else if (count == 3)
            nextgen = 1;
        else
            nextgen = 0;
    } else {
        if (count == 3)
            nextgen = 1;
        else
            nextgen = 0;
    }

    life[h][w] = nextgen;
}

int main() {
    int life[400];
    init_board(life);
    print_board(life);
    system("clear");
    int gen = 0;
    while (1) {
        printf("=========== Generation [%03d] ===========\n", gen++);
        for (int i=0; i<20; i=i+1)
            for (int j=0; j<20; j=j+1)
                nextgen(life, i, j);
        print_board(life);
        usleep(1000*100*5);
        system("clear");
    }
    return 0;
}
