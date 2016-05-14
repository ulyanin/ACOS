#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 48
#define NUM_OF_THREADS 8

static long long generation = 0;
int num = 0;
pthread_t thread[NUM_OF_THREADS];
int end[NUM_OF_THREADS];
int step = 0;
char map[2][SIZE][SIZE];
unsigned long long mask = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int temp[NUM_OF_THREADS];

int add(int x)
{
	if (x == SIZE - 1)
		return 0;
	else
		return x + 1;
}

int sub(int x)
{
	if (x == 0)
		return SIZE - 1;
	else
		return x - 1;
}

int alive(int x, int y)
{
	int al = 0;
	if (map[step][sub(x)][sub(y)] == 1) 
		++al;
	if (map[step][sub(x)][y] == 1)
		++al;
	if (map[step][sub(x)][add(y)] == 1)
		++al;
	if (map[step][x][sub(y)] == 1)
		++al;
	if (map[step][x][add(y)] == 1)
		++al;
	if (map[step][add(x)][sub(y)] == 1)
		++al;
	if (map[step][add(x)][y] == 1)
		++al;
	if (map[step][add(x)][add(y)] == 1)
		++al;
	return al;
}

void recalc(int x, int y)
{
	if (map[step][x][y] == 0)
	{
		if (alive(x, y) == 3)
			map[1 - step][x][y] = 1;
		else
			map[1 - step][x][y] = 0;
	}
	if (map[step][x][y] == 1)
	{
		if (alive(x, y) == 3 || alive(x, y) == 2)
			map[1 - step][x][y] = 1;
		else
			map[1 - step][x][y] = 0;
	}
}

void* life_1(void* arg)
{
	int i, j;
	int id = *(int*)arg;
        for (i = id; i < id + (SIZE / NUM_OF_THREADS); i++)
		for (j = 0; j < SIZE; j++)
		{
			recalc(i, j);
			if (map[0][i][j] != map[1][i][j])
				end[(id * NUM_OF_THREADS) / SIZE] = 1;
			//printf("%d\n", 	end[(id * NUM_OF_THREADS) / SIZE]);
		}
	pthread_mutex_lock(&lock);
	mask = mask^(1 << ((id * NUM_OF_THREADS) / SIZE));
	pthread_mutex_unlock(&lock);
        return NULL;
}

int main(){
        int status;
	int game_over = 0;
	int i, j;/*
	FILE* game = fopen("game_of_life", "r");
	for (i = 0; i < SIZE; i++)
		for (j = 0; j < SIZE; j++)
		{
			c = fgetc(game);
			if (c == 10)
				c = fgetc(game);
			if (c != EOF)
				map[0][i][j] = c - '0';
		}*/
	/*map[0][10][10] = 1;
    map[0][10][11] = 1;
    map[0][11][10] = 1;
    map[0][11][11] = 1;

    map[0][10][21] = 1;
    map[0][11][21] = 1;
    map[0][9][21] = 1;

    map[0][8][22] = 1;
    map[0][12][22] = 1;
    map[0][7][23] = 1;
    map[0][13][23] = 1;
    map[0][8][24] = 1;
    map[0][12][24] = 1;

    map[0][10][25] = 1;
    map[0][11][25] = 1;
    map[0][9][25] = 1;

    map[0][10][26] = 1;
    map[0][11][26] = 1;
    map[0][9][26] = 1;

    map[0][9][31] = 1;
    map[0][8][31] = 1;
    map[0][7][31] = 1;

    map[0][9][32] = 1;
    map[0][10][32] = 1;
    map[0][7][32] = 1;
    map[0][6][32] = 1;

    map[0][9][33] = 1;
    map[0][10][33] = 1;
    map[0][7][33] = 1;
    map[0][6][33] = 1;

    map[0][9][34] = 1;
    map[0][10][34] = 1;
    map[0][8][34] = 1;
    map[0][7][34] = 1;
    map[0][6][34] = 1;

    map[0][10][35] = 1;
    map[0][11][35] = 1;
    map[0][6][35] = 1;
    map[0][5][35] = 1;

    map[0][7][40] = 1;
    map[0][6][40] = 1;

    map[0][8][44] = 1;
    map[0][9][44] = 1;
    map[0][8][45] = 1;
    map[0][9][45] = 1;*/
	srand(time(NULL));
	for (i = 0; i < SIZE; i++)
		for (j = 0; j < SIZE; j++)
			map[0][i][j] = rand() % 2;
	while(1)
	{
		system("clear");
		printf("GENERETION #%lld\n", generation);
		for (i = 0; i < SIZE; i++){
			for (j = 0; j < SIZE; j++)
				printf("%s ", map[step][i][j] ? "■" : ".");
			printf("\n");
		}
		//printf("\n");
		generation++;
		int k;
		mask = (1 << NUM_OF_THREADS) - 1;
		for (k = 0; k < NUM_OF_THREADS; k++){
			temp[k] = k * (SIZE / NUM_OF_THREADS);
			status = pthread_create(&thread[k], NULL, life_1, &temp[k]);
                        if (status != 0)
                            perror("error pthread creating\n");
		}
		while(mask != 0)
			usleep(1);
		step = 1 - step;
		for (i = 0; i < NUM_OF_THREADS; i++)
			if (end[i] == 1)
			{
				game_over = 1;
				break;
			}
		if (game_over == 0)
		{
			printf("GAME OVER ;C\n");
			return 0;
		}
		game_over = 0;
		for (i = 0; i < NUM_OF_THREADS; i++)
			end[i] = 0;

		usleep(100000);

	}
	return 0;
}

