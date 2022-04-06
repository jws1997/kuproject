#include <stdio.h>
#include <windows.h>
#include <time.h>

#define BLACK 1	//흑돌
#define WHITE -1 //백돌
#define EMPTY 0 //빈곳
#define SIZE 19 //바둑판 가로 세로

typedef struct _point //좌표 저장을 위한 구조체
{
	int x;
	int y;
} point;

char choose_turn();	//흑돌 혹은 백돌 선택
void game_struct();
void state_show(int state[][SIZE]);	//현재 state 출력
void move_xy(int state[][SIZE], int x, int y, char turn); //돌을 두는 함수


point iterative_deepening(int state[][SIZE], char computer);
point alpha_beta_search(int state[][SIZE], char computer, int l, clock_t before);
int max_value(int state[][SIZE], int v[][SIZE], int a, int b, char s, char turn, int l, clock_t before);
int min_value(int state[][SIZE], int v[][SIZE], int a, int b, char turn, int l, clock_t before);

int utility(int state[][SIZE], char turn);
int eval(int state[][SIZE],char turn);
int distance(int state[][SIZE], int i, int j);
int score(int state[][SIZE], int i, int j, char turn);
int score_weak(int m, int l);
int score_strong(int m, int l);

int goal_test(int state[][SIZE], char turn);
int threethree(int state[][SIZE],int x, int y, char turn);

char player; 
char computer;

int main() {

	printf("오목 AI 프로그램\n시작하려면 아무키나 눌러주세요.·····");
	getch();
	game_struct();


	return 0;
}
void game_struct() {

	int state[SIZE][SIZE] = { EMPTY, };

	char turn = BLACK; //누구 차례인지 알려주는 변수
	point p;


	player = choose_turn();
	computer = player * -1;
	

	while (1) {
		int x;
		int y;
		double time;
		clock_t before;


		if (turn == player) {
			before = clock();
			while (1) {
				state_show(state);
				while (1) {
					printf("x좌표 입력\n");
					scanf_s("%d", &x);
					getchar();
					if (x >= 0 && x < SIZE)
						break;
					else
						printf("범위에 맞게 제대로 입력해주세요.\n");
				}
				while (1) {
					printf("y좌표 입력\n");
					scanf_s("%d", &y);
					getchar();
					if (y >= 0 && y < SIZE)
						break;
					else
						printf("범위에 맞게 제대로 입력해주세요.\n");
				}
				system("cls");
				if (state[x][y] == EMPTY && threethree(state, x, y, turn) == FALSE) {
					time = (double)(clock() - before) / CLOCKS_PER_SEC;
					printf("(%d, %d)에 착수\n", x, y);
					printf("%f초 경과.\n", time);
					break;
				}
				else if (state[x][y] != EMPTY)
					printf("이미 돌이 놓여져 있습니다.\n");
				else if (threethree(state,x,y,turn)==TRUE)
					printf("삼삼 금수입니다.\n");
			}
			if (time > 10) {
				printf("플레이어 10초 초과! 패배하셨습니다.\n");
				return;
			}
			move_xy(state, x, y, turn);
			
		}
		else if (turn == computer) {
			state_show(state);
			before = clock();
			printf("생각중....");
			p = iterative_deepening(state, computer);
			time = (double)(clock() - before) / CLOCKS_PER_SEC;
			move_xy(state, p.x, p.y, turn);
			system("cls");
			printf("(%d, %d)에 착수\n", p.x, p.y);
			printf("%f초 경과\n",time);
			if (time > 10) {
				printf("컴퓨터 10초 초과! 컴퓨터가 패배했습니다.\n");
				return;
			}
		}
		
		
		

		if (goal_test(state, turn)) {
			state_show(state);
			if (turn == player)
				printf("player wins");
			else if (turn == computer)
				printf("computer wins");
			return;
		}
		turn = turn * -1;
	}

}
char choose_turn() {

	char player;

	system("cls");
	printf("흑으로 진행하면 b를 백으로 진행하려면 w를 눌러주세요.\n");

	while (1) {
		player = getch();

		if (player == 'b' || player == 'B') {
			printf("흑 선택\n");
			return BLACK;

		}
		else if (player == 'w' || player == 'W') {
			printf("백 선택\n");
			return WHITE;

		}

		system("cls");
		printf("잘못 입력하셨습니다. 다시 입력해주세요.\n흑으로 진행하면 b를 백으로 진행하려면 w를 눌러주세요.\n");
	}
}

void state_show(int state[][SIZE]) {
	printf("x＼y 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18\n");
	for (int i = 0; i < SIZE; i++) {
		printf("%2d ", i);
		for (int j = 0; j < SIZE; j++) {
			if (state[i][j] == WHITE)
				printf(" ●");
			else if (state[i][j] == BLACK)
				printf(" ○");
			else if (state[i][j] == 0)
				printf(" ·");
		}
		printf("\n");
	}
}

void move_xy(int state[][SIZE], int x, int y, char turn) {
	if (turn == WHITE) {
		state[x][y] = WHITE;
	}
	else if (turn == BLACK) {
		state[x][y] = BLACK;
	}
}

point iterative_deepening(int state[][SIZE], char computer) {
	clock_t before;
	point move;
	before = clock();


	for (int l = 0; (double)(clock() - before) / CLOCKS_PER_SEC < 9.99; l++) {
		move = alpha_beta_search(state, computer, l, before);
	}
	
	return move;
}

point alpha_beta_search(int state[][SIZE], char computer, int l, clock_t before) {
	int tempstate[SIZE][SIZE] = { EMPTY, };
	int v[SIZE][SIZE] = { EMPTY, };
	point move;
	int value;
	int x;
	int y;

	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {

			tempstate[i][j] = state[i][j];
		}

	}

	value = max_value(tempstate, v, -2000000, 2000000, 1, computer, l, before);



	for (x = 0; x < SIZE; x++) {
		for (y = 0; y < SIZE; y++) {
			if (v[x][y] == value && state[x][y] == EMPTY)
				break;
		}
		if (v[x][y] == value)
			break;
	}

	move.x = x;
	move.y = y;
	return move;
}

int max_value(int state[][SIZE], int v[][SIZE], int a, int b, char s, char turn, int l, clock_t before) {

	int val = -2000000;
	int tem;

	if (goal_test(state, turn) || l < 0 || (double)(clock() - before) / CLOCKS_PER_SEC >9.99) {
		return utility(state, -turn);
	}

	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (state[i][j] == EMPTY&&threethree(state,i,j,turn)==FALSE) {
				move_xy(state, i, j, turn);
				tem = min_value(state, v, a, b, -turn, l - 1, before);
				state[i][j] = EMPTY;
				if (val < tem) {
					val = tem;
				}

				if (s == 1)
					v[i][j] = val;

				

				if (val >= b)
					return val;

				if (a < val)
					a = val;


			}
		}

	}
	return val;

}
int min_value(int state[][SIZE], int v[][SIZE], int a, int b, char turn, int l, clock_t before) {

	int val = 2000000;
	int tem;
	if (goal_test(state,turn) || l < 0 || (double)(clock() - before) / CLOCKS_PER_SEC > 9.99)
		return utility(state, -turn);

	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (state[i][j] == EMPTY&&threethree(state, i, j, turn) == FALSE) {

				move_xy(state, i, j, turn);

				tem = max_value(state, v, a, b, 0, -turn, l - 1, before);
				state[i][j] = EMPTY;
				if (val > tem)
					val = tem;


				if (val <= a)
					return val;

				if (b > val)
					b = val;


			}
		}
	}
	return val;
}



int utility(int state[][SIZE],char turn) {
	int i = goal_test(state, turn);
	if (i) {
		if (turn == player)
			return -1500000;
		else if (turn == computer)
			return 1500000;
	}
	else
		return eval(state,turn);
	
} 
int eval(int state[][SIZE],char turn) {

	int evaluation = 0;

	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (state[i][j] != EMPTY) {
				evaluation = evaluation+ score(state,i,j,turn)+distance(state,i,j);
			}
		}
	}
	return evaluation;
}
int distance(int state[][SIZE], int i, int j) {
	int d = 0;
	int x = j - 9;
	int y = i - 9;

	if (x < 0)
		x = x*-1;
	if (y < 0)
		y = y*-1;
	if (x >= y) {
		if (state[i][j] == computer)
			return x*-1;
		else
			return x;
	}
	else {
		if (state[i][j] == computer)
			return y*-1;
		else
			return y;
	}

}
int score(int state[][SIZE], int i, int j, char turn) {
	int m = 0;
	int l = 0;
	int eval = 0;
	int c = state[i][j];

	for (m = 0, l = 0; state[i + m][j] == c && i + m < SIZE; m++) {}
	if (i - 1 < 0 || state[i - 1][j] != c) {
		if (i + m < SIZE&&state[i + m][j] == EMPTY) { l += 1; }
		if (i - 1 >= 0 && state[i - 1][j] == EMPTY) { l += 1; }
		if (c == turn&& c == computer)
			eval += score_weak(m, l);
		else if (c == -turn && c == computer)
			eval += score_strong(m, l);
		else if (c == turn && c == player)
			eval -= score_weak(m, l);
		else if (c == -turn && c == player)
			eval -= score_strong(m, l);
	}

	



	for (m = 0, l = 0; state[i][j + m] == c && j + m < SIZE; m++) {}
	if (j - 1 < 0 || state[i][j - 1] != c) {
		if (j + m < SIZE&&state[i][j + m] == EMPTY) { l += 1; }
		if (j - 1 >= 0 && state[i][j - 1] == EMPTY) { l += 1; }
		if (c == turn&& c == computer)
			eval += score_weak(m, l);
		else if (c == -turn && c == computer)
			eval += score_strong(m, l);
		else if (c == turn && c == player)
			eval -= score_weak(m, l);
		else if (c == -turn && c == player)
			eval -= score_strong(m, l);
		
	}


	for (m = 0, l = 0; state[i + m][j + m] == c && i + m < SIZE && j + m < SIZE; m++) {}
	if (j - 1 < 0 || i - 1 < 0 || state[i - 1][j - 1] != c) {
		if (i + m < SIZE&&j + m < SIZE&&state[i + m][j + m] == EMPTY) { l += 1; }
		if (j - 1 >= 0 && i - 1 >= 0 && state[i - 1][j - 1] == EMPTY) { l += 1; }
		if (c == turn&& c == computer)
			eval += score_weak(m, l);
		else if (c == -turn && c == computer)
			eval += score_strong(m, l);
		else if (c == turn && c == player)
			eval -= score_weak(m, l);
		else if (c == -turn && c == player)
			eval -= score_strong(m, l);
	}


	for (m = 0, l = 0; state[i - m][j + m] == c && i - m >= 0 && j + m < SIZE; m++) {}
	if (j - 1 < 0 || i + 1 > SIZE || state[i + 1][j - 1] != c) {
		if (i - m >= 0 && j + m < SIZE&&state[i - m][j + m] == EMPTY) { l += 1; }
		if (j - 1 >= 0 && i + 1 < SIZE && state[i + 1][j - 1] == EMPTY) { l += 1; }
		if (c == turn&& c == computer)
			eval += score_weak(m, l);
		else if (c == -turn && c == computer)
			eval += score_strong(m, l);
		else if (c == turn && c == player)
			eval -= score_weak(m, l);
		else if (c == -turn && c == player)
			eval -= score_strong(m, l);
	}

	
	if (i - 1 >= 0 && i + 4 < SIZE&&state[i - 1][j] == EMPTY&&state[i + 3][j] == c && state[i + 4][j] == EMPTY) {
		if (state[i + 1][j] == EMPTY&&state[i + 2][j] == c) {
			if (c == turn&&c == computer)
				eval += score_weak(3, 2);
			else if (c == -turn && c == computer)
				eval += score_strong(3, 2);
			else if (c == turn && c == player)
				eval -= score_weak(3, 2);
			else if (c == -turn && c == player)
				eval -= score_strong(3, 2);
		}

		if (state[i + 1][j] == c&&state[i + 2][j] == EMPTY)
		{
			if (c == turn&&c == computer)
				eval += score_weak(3, 2);
			else if (c == -turn && c == computer)
				eval += score_strong(3, 2);
			else if (c == turn && c == player)
				eval -= score_weak(3, 2);
			else if (c == -turn && c == player)
				eval -= score_strong(3, 2);
		}
	}

	if (j - 1 >= 0 && j + 4 < SIZE&&state[i][j - 1] == EMPTY&&state[i][j + 3] == c && state[i][j + 4] == EMPTY) {
		if (state[i][j + 1] == EMPTY&&state[i][j + 2] == c)
		{
			if (c == turn&&c == computer)
				eval += score_weak(3, 2);
			else if (c == -turn && c == computer)
				eval += score_strong(3, 2);
			else if (c == turn && c == player)
				eval -= score_weak(3, 2);
			else if (c == -turn && c == player)
				eval -= score_strong(3, 2);
		}
		if (state[i][j + 1] == c&&state[i][j + 2] == EMPTY)
		{
			if (c == turn&&c == computer)
				eval += score_weak(3, 2);
			else if (c == -turn && c == computer)
				eval += score_strong(3, 2);
			else if (c == turn && c == player)
				eval -= score_weak(3, 2);
			else if (c == -turn && c == player)
				eval -= score_strong(3, 2);
		}
	}

	if (j - 1 >= 0 && i - 1 >= 0 && i + 4 < SIZE && j + 4 < SIZE &&state[i - 1][j - 1] == EMPTY && state[i + 3][j + 3] == c&&state[i + 4][j + 4] == EMPTY) {
		if (state[i + 1][j + 1] == EMPTY&&state[i + 2][j + 2] == c)
		{
			if (c == turn&&c == computer)
				eval += score_weak(3, 2);
			else if (c == -turn && c == computer)
				eval += score_strong(3, 2);
			else if (c == turn && c == player)
				eval -= score_weak(3, 2);
			else if (c == -turn && c == player)
				eval -= score_strong(3, 2);
		}
		if (state[i + 1][j + 1] == c&&state[i + 2][j + 2] == EMPTY)
		{
			if (c == turn&&c == computer)
				eval += score_weak(3, 2);
			else if (c == -turn && c == computer)
				eval += score_strong(3, 2);
			else if (c == turn && c == player)
				eval -= score_weak(3, 2);
			else if (c == -turn && c == player)
				eval -= score_strong(3, 2);
		}
	}

	if (j - 1 >= 0 && i + 1 < SIZE && i - 4 >= 0 && j + 4 < SIZE &&state[i + 1][j - 1] == EMPTY && state[i - 3][j + 3] == c && state[i - 4][i + 4] == EMPTY) {
		if (state[i - 1][j + 1] == EMPTY&&state[i - 2][j + 2] == c)
		{
			if (c == turn&&c == computer)
				eval += score_weak(3, 2);
			else if (c == -turn && c == computer)
				eval += score_strong(3, 2);
			else if (c == turn && c == player)
				eval -= score_weak(3, 2);
			else if (c == -turn && c == player)
				eval -= score_strong(3, 2);
		}
		if (state[i - 1][j + 1] == c&&state[i - 2][j + 2] == EMPTY)
		{
			if (c == turn&&c == computer)
				eval += score_weak(3, 2);
			else if (c == -turn && c == computer)
				eval += score_strong(3, 2);
			else if (c == turn && c == player)
				eval -= score_weak(3, 2);
			else if (c == -turn && c == player)
				eval -= score_strong(3, 2);
		}

	}
	return eval;
}
int score_weak(int m, int l) {		//약하게 평가
	int eval = 0;


		if (l == 1) {	//한쪽이 막힌 경우
			if (m == 2) { eval = 10; }
			else if (m == 3) { eval = 100; }
			else if (m == 4) { eval = 10000; }
		}
		else if (l == 2) {	//양쪽이 뚫린 경우
			if (m == 2) { eval = 20; }
			else if (m == 3) { eval = 600; }
			else if (m == 4) { eval = 20000; }
		}

	
	return eval;
}
int score_strong(int m, int l) {		//강하게 평가
	int eval = 0;
	
	
		if (l == 1) {	//한쪽이 막힌 경우
			if (m == 2) { eval = 15; }
			else if (m == 3) { eval = 500; }
			else if (m == 4) { eval = 100000; }
		}
		else if (l == 2) {	//양쪽이 뚫린 경우
			if (m == 2) { eval = 25; }
			else if (m == 3) { eval = 2000; }
			else if (m == 4) { eval = 200000; }
		}
	
	return eval;
}

int goal_test(int state[][SIZE], char turn) {
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (state[i][j] == turn) {
				int c = state[i][j];
				int m;

				for (m = 0; state[i + m][j] == c && i + m < SIZE; m++) {}

				if (m == 5) {
					return TRUE;
				}


				for (m = 0; state[i][j + m] == c && j + m < SIZE; m++) {}

				if (m == 5)
				{
					return TRUE;
				}


				for (m = 0; state[i + m][j + m] == c && i + m < SIZE && j + m < SIZE; m++) {}

				if (m == 5)
				{
					return TRUE;
				}


				for (m = 0; state[i - m][j + m] == c && i - m >= 0 && j + m < SIZE; m++) {}

				if (m == 5)
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}
int threethree(int state[][SIZE], int x, int y, char turn) {
	int sam1 = 0;
	int sam2 = 0;
	int temp[SIZE][SIZE];
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			temp[i][j] = state[i][j];
		}
	}
	temp[x][y] = turn;



	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (state[i][j] == turn) {
				int m;
				int l;
				int c = state[i][j];

				for (m = 0, l = 0; state[i + m][j] == c && i + m < SIZE; m++) {}
				if (i - 1 < 0 || state[i - 1][j] != c) {
					if (i + m < SIZE&&state[i + m][j] == EMPTY) { l += 1; }
					if (i - 1 >= 0 && state[i - 1][j] == EMPTY) { l += 1; }
					if (m == 3 && l == 2)
						sam1 += 1;


				}



				for (m = 0, l = 0; state[i][j + m] == c && j + m < SIZE; m++) {}
				if (j - 1 < 0 || state[i][j - 1] != c) {
					if (j + m < SIZE&&state[i][j + m] == EMPTY) { l += 1; }
					if (j - 1 >= 0 && state[i][j - 1] == EMPTY) { l += 1; }
					if (m == 3 && l == 2)
						sam1 += 1;

				}


				for (m = 0, l = 0; state[i + m][j + m] == c && i + m < SIZE && j + m < SIZE; m++) {}
				if (j - 1 < 0 || i - 1 < 0 || state[i - 1][j - 1] != c) {
					if (i + m < SIZE&&j + m < SIZE&&state[i + m][j + m] == EMPTY) { l += 1; }
					if (j - 1 >= 0 && i - 1 >= 0 && state[i - 1][j - 1] == EMPTY) { l += 1; }
					if (m == 3 && l == 2)
						sam1 += 1;
				}


				for (m = 0, l = 0; state[i - m][j + m] == c && i - m >= 0 && j + m < SIZE; m++) {}
				if (j - 1 < 0 || i + 1 > SIZE || state[i - 1][j - 1] != c) {
					if (i - m >= 0 && j + m < SIZE&&state[i - m][j + m] == EMPTY) { l += 1; }
					if (j - 1 >= 0 && i + 1 < SIZE && state[i + 1][j - 1] == EMPTY) { l += 1; }
					if (m == 3 && l == 2)
						sam1 += 1;
				}



				if (i - 1 >= 0 && i + 4 < SIZE&&state[i - 1][j] == EMPTY&&state[i + 3][j] == c && state[i + 4][j] == EMPTY) {
					if (state[i + 1][j] == EMPTY&&state[i + 2][j] == c)
						sam1 += 1;
					if (state[i + 1][j] == c&&state[i + 2][j] == EMPTY)
						sam1 += 1;
				}

				if (j - 1 >= 0 && j + 4 < SIZE&&state[i][j - 1] == EMPTY&&state[i][j + 3] == c && state[i][j + 4] == EMPTY) {
					if (state[i][j + 1] == EMPTY&&state[i][j + 2] == c)
						sam1 += 1;
					if (state[i][j + 1] == c&&state[i][j + 2] == EMPTY)
						sam1 += 1;
				}
				if (j - 1 >= 0 && i - 1 >= 0 && i + 4 < SIZE && j + 4 < SIZE &&state[i - 1][j - 1] == EMPTY && state[i + 3][j + 3] == c&&state[i + 4][j + 4] == EMPTY) {
					if (state[i + 1][j + 1] == EMPTY&&state[i + 2][j + 2] == c)
						sam1 += 1;
					if (state[i + 1][j + 1] == c&&state[i + 2][j + 2] == EMPTY)
						sam1 += 1;
				}
				if (j - 1 >= 0 && i + 1 < SIZE && i - 4 >= 0 && j + 4 < SIZE &&state[i + 1][j - 1] == EMPTY && state[i - 3][j + 3] == c && state[i - 4][i + 4] == EMPTY) {
					if (state[i - 1][j + 1] == EMPTY&&state[i - 2][j + 2] == c)
						sam1 += 1;
					if (state[i - 1][j + 1] == c&&state[i - 2][j + 2] == EMPTY)
						sam1 += 1;
				}
			}
		}
	}
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (temp[i][j] == turn) {
				int m;
				int l;
				int c = temp[i][j];


				for (m = 0, l = 0; temp[i + m][j] == c && i + m < SIZE; m++) {}
				if (i - 1 < 0 || temp[i - 1][j] != c) {
					if (i + m < SIZE&&temp[i + m][j] == EMPTY) { l += 1; }
					if (i - 1 >= 0 && temp[i - 1][j] == EMPTY) { l += 1; }
					if (m == 3 && l == 2)
						sam2 += 1;
				}



				for (m = 0, l = 0; temp[i][j + m] == c && j + m < SIZE; m++) {}
				if (j - 1 < 0 || temp[i][j - 1] != c) {
					if (j + m < SIZE&&temp[i][j + m] == EMPTY) { l += 1; }
					if (j - 1 >= 0 && temp[i][j - 1] == EMPTY) { l += 1; }
					if (m == 3 && l == 2)
						sam2 += 1;

				}


				for (m = 0, l = 0; temp[i + m][j + m] == c && i + m < SIZE && j + m < SIZE; m++) {}
				if (j - 1 < 0 || i - 1 < 0 || temp[i - 1][j - 1] != c) {
					if (i + m < SIZE&&j + m < SIZE&&temp[i + m][j + m] == EMPTY) { l += 1; }
					if (j - 1 >= 0 && i - 1 >= 0 && temp[i - 1][j - 1] == EMPTY) { l += 1; }
					if (m == 3 && l == 2)
						sam2 += 1;
				}


				for (m = 0, l = 0; temp[i - m][j + m] == c && i - m >= 0 && j + m < SIZE; m++) {}
				if (j - 1 < 0 || i + 1 > SIZE || temp[i - 1][j - 1] != c) {
					if (i - m >= 0 && j + m < SIZE&&temp[i - m][j + m] == EMPTY) { l += 1; }
					if (j - 1 >= 0 && i + 1 < SIZE && temp[i + 1][j - 1] == EMPTY) { l += 1; }
					if (m == 3 && l == 2)
						sam2 += 1;
				}



				if (i - 1 >= 0 && i + 4 < SIZE&&temp[i - 1][j] == EMPTY&&temp[i + 3][j] == c && temp[i + 4][j] == EMPTY) {
					if (temp[i + 1][j] == EMPTY&&temp[i + 2][j] == c)
						sam2 += 1;
					if (temp[i + 1][j] == c&&temp[i + 2][j] == EMPTY)
						sam2 += 1;
				}

				if (j - 1 >= 0 && j + 4 < SIZE&&temp[i][j - 1] == EMPTY&&temp[i][j + 3] == c && temp[i][j + 4] == EMPTY) {
					if (temp[i][j + 1] == EMPTY&&temp[i][j + 2] == c)
						sam2 += 1;
					if (temp[i][j + 1] == c&&temp[i][j + 2] == EMPTY)
						sam2 += 1;
				}
				if (j - 1 >= 0 && i - 1 >= 0 && i + 4 < SIZE && j + 4 < SIZE &&temp[i - 1][j - 1] == EMPTY && temp[i + 3][j + 3] == c&&temp[i + 4][j + 4] == EMPTY) {
					if (temp[i + 1][j + 1] == EMPTY&&temp[i + 2][j + 2] == c)
						sam2 += 1;
					if (temp[i + 1][j + 1] == c&&temp[i + 2][j + 2] == EMPTY)
						sam2 += 1;
				}
				if (j - 1 >= 0 && i + 1 < SIZE && i - 4 >= 0 && j + 4 < SIZE &&temp[i + 1][j - 1] == EMPTY && temp[i - 3][j + 3] == c && temp[i - 4][i + 4] == EMPTY) {
					if (temp[i - 1][j + 1] == EMPTY&&temp[i - 2][j + 2] == c)
						sam2 += 1;
					if (temp[i - 1][j + 1] == c&&temp[i - 2][j + 2] == EMPTY)
						sam2 += 1;
				}
			}
		}

	}
	if (sam2 - sam1 >= 2)
		return TRUE;
	else
		return FALSE;
}


