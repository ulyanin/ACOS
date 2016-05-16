#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1000
#define NUM_OF_THREADS 8

int num = 0;
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

int recalc(int x, int y)
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
	for (i = id; i < id	+ (SIZE / NUM_OF_THREADS); i++)
		for (j = 0; j < SIZE; j++)
		{
			recalc(i, j);
			//if (map[0][i][j] != map[1][i][j])
			//	end[(id * NUM_OF_THREADS) / SIZE] = 1;
			//printf("%d\n", 	end[(id * NUM_OF_THREADS) / SIZE]);
		}
	pthread_mutex_lock(&lock);
	mask -= 1;
	//mask = mask^(1 << ((id * NUM_OF_THREADS) / SIZE));
	pthread_mutex_unlock(&lock);
}

int easy_life(int num_it){
	FILE* game = fopen("game_of_life", "r");
	int i, j;
	for (i = 0; i < SIZE; i++)
		for (j = 0; j < SIZE; j++)
		{
			int c;
			c = fgetc(game);
			if (c == 10)
				c = fgetc(game);
			if (c != EOF)
				map[0][i][j] = c - '0';
		}
	fclose(game);
	time_t t1 = time(NULL);
	int st, q;
	for(st = 0; st < num_it; ++st){
		printf("%d\n", st);
		step = st & 1;
		for(i = 0; i < SIZE; ++i)
			for(q = 0; q < SIZE; ++q)
				recalc(i, q);
	}
	printf("easy life works: %f\n", difftime(time(NULL), t1));
	game = fopen("game_easy_life", "w");
	for(i = 0; i < SIZE; ++i){
		for(q = 0; q < SIZE; ++q)
			fprintf(game, "%c", map[step][i][q] + '0');
		fprintf(game, "\n");
	}
	fclose(game);
}

int generate_some_life(){
	srand(time(NULL));
	int i, k, q, j;
	for (i = 0; i < SIZE; i++)
		for (j = 0; j < SIZE; j++)
			map[0][i][j] = rand() % 2;

	FILE* game = fopen("game_of_life", "w");
	for(i = 0; i < SIZE; ++i){
		for(q = 0; q < SIZE; ++q)
			fprintf(game, "%c", map[step][i][q] + '0');
		fprintf(game, "\n");
	}
	fclose(game);
}

int life_by_threads(int num_it){
	int status;
	int tmp;
	int c;
	int game_over = 0;
 	int i, j;
	pthread_t thread[NUM_OF_THREADS];
	int generation = 0;
	FILE* game = fopen("game_of_life", "r");
	for (i = 0; i < SIZE; i++)
		for (j = 0; j < SIZE; j++)
		{
			c = fgetc(game);
			if (c == 10)
				c = fgetc(game);
			if (c != EOF)
				map[0][i][j] = c - '0';
		}
	fclose(game);

	time_t t1 = time(NULL);
	while(generation < num_it)
	{
		printf("%d\n", generation);
		/*
		system("clear");
		printf("GENERATION #%d\n", generation);
		for (i = 0; i < SIZE; i++){
			for (j = 0; j < SIZE; j++)
				printf("%s ", map[step][i][j] ? "■" : ".");
			printf("\n");
		}
		printf("\n");
		*/
		generation++;
		int k;
		mask = 8;//(1 << NUM_OF_THREADS) - 1;
		for (k = 0; k < NUM_OF_THREADS; k++){
			temp[k] = k * (SIZE / NUM_OF_THREADS);
			status = pthread_create(&thread[k], NULL, life_1, &temp[k]);
		}
		while(mask != 0)
			usleep(1);
		step = 1 - step;
		/*
		for (i = 0; i < NUM_OF_THREADS; i++)
			if (end[i] == 1)
			{
				game_over = 1;
				break;
			}
		*/
		/*
		if (game_over == 0)
		{
			printf("GAME OVER ;C\n");
			return 0;
		}
		game_over = 0;
		for (i = 0; i < NUM_OF_THREADS; i++)
			end[i] = 0;
		*/
		//usleep(500000);

	}

	printf("threads life works: %f\n", difftime(time(NULL), t1));
	int q;
	step ^= 1;
	game = fopen("game_threads_life", "w");
	for(i = 0; i < SIZE; ++i){
		for(q = 0; q < SIZE; ++q)
			fprintf(game, "%c", map[step][i][q] + '0');
		fprintf(game, "\n");
	}
	fclose(game);
}

int main(){
	int num;
	scanf("%d", &num);
	generate_some_life();
	easy_life(num);
	life_by_threads(num);
	
	return 0;
}

