#include <stdio.h>
#include <sys/timeb.h>
#define bool char


void initialize();
void update();
void teardown();

const int FRAME_RATE = 1000;
const int BOARD_WIDTH = 12;
const int BOARD_HEIGHT = 21;
char board[BOARD_HEIGHT][BOARD_WIDTH];

int score = 0;

bool game_over = false;

int main() {
	/* STUFF WE NEED:
		- 10 X 20 BOARD
		- TETROMINOS
		- SCORE
		- TETRICALES FALL AT SPEED
		- ROTATE WITH ARROW KEYS
		- FALL INSANTLY WITH SPACE
	*/

	struct timeb start, end;
	ftime(&start);
	int time_diff;
	ftime(&start);


	initialize();

	while (!game_over) {
		for (int i = 0; i < 100; i++) {

		}
		ftime(&end);
		time_diff = (int) 1000 * (end.time - start.time) + (end.millitm - start.millitm);
		if (time_diff > FRAME_RATE) {
			update();
			ftime(&start);
		}
	}

	teardown();

	
}

void initialize() {
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_WIDTH; j++) {
			if (j == 0 || j == BOARD_WIDTH - 1) {
				board[i][j] = '|';
			}
			else if (i == BOARD_HEIGHT - 1) {
				board[i][j] = '_';
			}
			else {
				board[i][j] = '.';
			}
		}
	}
}

void update() {
	printf("\033[0;0H\033[2J");
	printf("SCORE:%d\n\n", score);
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_WIDTH; j++) {
			printf("%c",board[i][j]);
		}
		printf("\n");
	}
	
}

void teardown() {

}