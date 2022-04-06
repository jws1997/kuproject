/*
전체적인 흐름
1.가장 먼저 script_read 함수를 통해서 script에 대한 정보를 읽고 저정한다.
2.그 후엔 filesearch함수를 통해 trn디렉토리를 검색하여 training data에 대한 정보를 읽는다.
3.위 두 정보를 통해서 학습을 시작한다.
4.가장 먼저 a_mix 함수를 통해 utterance hmm의 trasition matrix를 계산한다.
5.그 다음엔 b_mix 함수를 통해 utterance hmm의 emmsion probability matrx를 계산한다.
6.이 후 accumulation함수 내에서 forward함수와 backward 함수를 통해서 forward와 backward probability, log liklihood를 계산하며
update에 필요한 값들을 누적시켜준다.
7.이제 누적이 됐으니 update함수를 통해서 실제 모델에 쓰일 parameter값을 계산해준다.
8.새 parameter을 기존 model에 update해준다.
9.file_update 함수를 통해서 새 parameter을 hmm.txt에 upload시켜준다.
10.4~9번 과정을 계속 반복하는데 5번째 반복마다 gaussian_split함수를 통해서 하나의 가우시안은 두개로 쪼개준다.
*/
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <Windows.h>
#include "hmm.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

#pragma warning(disable: 4996)


#define N_DIMENSION	39
#define M_INFINITY -1000000000		//log(0)대신 사용
#define N_STATE 3

typedef struct {		//가우시안 갯수를 늘려야 하므로 새로운 구조체 선언
	pdfType pdf[10];	//가우시안이 최대 10개까지 가능하다.
} stateType1;
typedef struct {		//위와 동일한 이유로 새로운 구조체 선언
	char *name;
	float tp[N_STATE + 2][N_STATE + 2];
	stateType1 state[N_STATE];
} hmmType1;

typedef struct {	//학습된 결과를 기존 hmm에 반영하기 위해서 어디서 왔는지를 기록한다.
	int phone_i;
	int state_i;
} goback;
typedef struct {	//training data에 대한 정답 script를 저장하기 위한 구조체
	char name[30];	//파일(training data) 이름
	int phone_num;	//phone 갯수
	char **phone;	//phone 이름 저장
	int state_num;
	goback *state_inf;
	stateType1 *state_seq;	//state의 sequnece
} script;

struct _finddata_t fd;	//파일 탐색을 위한 구조체

void filesearch(char file_path[], char file_name[][_MAX_PATH], int *index);	//trn파일 탐색 함수
void script_read(script *scr); //transcript 읽기
void a_mix(float **a, script *scr, int scr_i, hmmType1 *phones);	//utterance hmm transition 계산
void b_mix(long double ***b, float **input, script *scr, int scr_i, hmmType1 *phones, int input_length, int g_num);	//utterance hmm emmision 계산하는 함수
void forward(long double ***b, float **a, long double **f, script *scr, int scr_i, int input_length, long double *p, long double *ll); //forward 및 log likelihood 계산하는 함수
void backward(long double ***b, float **a, long double **back, script *scr, int scr_i, int input_length);	//backward 계산하는 함수
void accumulate(float **a, long double ***b, long double s_oc[][3][11], float ** input, int scr_i, int input_length, int g_num, long double *ll, script *scr, hmmType1 *new_phones);	//바움 웰치에서의 누적을 담당하는 함수
void update(long double s_oc[][3][11], hmmType1 *new_phones, int g_num);	//바움 웰치에서 업데이트 작업을 하는 함수
void file_update(hmmType1 *phones_up, int g_num);	//업데이트한 결과를 hmm.txt에 반영하는 함수
void gaussian_split(hmmType1 *phones_up, int g_num);	//매 5회 반복마다 하나의 가우시안을 두개로 나눠주는 함수
long double logsum(long double x, long double y);

void main() {

	script *scr = NULL;	//script에 대한 정보 저장

	hmmType1 phones_up[21];	//학습된 결과를 반영하는 hmm 

	int scr_num = 0;	//training data 갯수



	FILE *fp;
	char file_path[_MAX_PATH] = ".\\trn";
	char file_name[2000][_MAX_PATH];
	int index = 0;

	int input_length = 0;	//하나의 training data의 시간축 길이
	int skip;

	int scr_i = 0; //현재 학습하는 training data가 scr의 몇번째 요소인지를 저장하는 index
	int n = 0;	//몇번 째 학습인지
	int g_num = 2;	//가우시안의 갯수를 저장한다.

	char str[30];


	fopen_s(&fp, "trn_mono.txt", "r");	//data 갯수 count
	if (fp != NULL) {
		while (fgets(str, 30, fp) != NULL) {
			if (str[0] == '"') {
				(scr_num)++;
			}
		}
		rewind(fp);

		scr = (script *)malloc(sizeof(script)*(scr_num));
	}


	script_read(scr);
	filesearch(file_path, file_name, &index);	//training data를 탐색하여 배열로 저장
	fclose(fp);

	for (int i = 0; i < 21; i++) {	// phones_up 초기화

		if (phones[i].name == "sp") {
			phones_up[i].name = phones[i].name;
			for (int j = 0; j < g_num; j++) {
				for (int k = 0; k < N_DIMENSION; k++) {

					phones_up[i].state[0].pdf[j].mean[k] = phones[i].state[0].pdf[j].mean[k];
					phones_up[i].state[0].pdf[j].var[k] = phones[i].state[0].pdf[j].var[k];

				}
				phones_up[i].state[0].pdf[j].weight = phones[i].state[0].pdf[j].weight;
			}
			for (int j = 0; j < 3; j++) {
				for (int k = 0; k < 3; k++) {

					phones_up[i].tp[j][k] = phones[i].tp[j][k];
				}
			}
		}
		else {
			phones_up[i].name = phones[i].name;
			for (int j = 0; j < N_STATE; j++) {
				for (int k = 0; k < g_num; k++) {
					for (int l = 0; l < N_DIMENSION; l++) {

						phones_up[i].state[j].pdf[k].mean[l] = phones[i].state[j].pdf[k].mean[l];
						phones_up[i].state[j].pdf[k].var[l] = phones[i].state[j].pdf[k].var[l];
					}
					phones_up[i].state[j].pdf[k].weight = phones[i].state[j].pdf[k].weight;
				}

			}
			for (int j = 0; j < 5; j++) {
				for (int k = 0; k < 5; k++) {

					phones_up[i].tp[j][k] = phones[i].tp[j][k];
				}
			}
		}

	}


	while (1) {
		hmmType1 new_phones[21];	//누적값(accumulation result)를 저장할 곳
		long double s_oc[21][3][11];	// [phone][state][pdf]의 인덱스를 통해서 gaussian occupance를 구한다. pdf가 0이면 state occupancy이다.
		char c;

		long double avg_ll = M_INFINITY; //average log likelihood를 저장

		printf("%d번째 학습을 시작하려면 o를 종료하려면 다른 키를 눌러주세요\n", n + 1);
		c = getch();
		if (c == 'o') {
			system("cls");
			for (int i = 0; i < 21; i++) {	//누적 결과를 저장할 new_phone 초기화

				if (phones[i].name == "sp") {
					phones_up[i].name = phones[i].name;
					for (int j = 0; j < g_num; j++) {
						for (int k = 0; k < N_DIMENSION; k++) {
							new_phones[i].state[0].pdf[j].mean[k] = 0;
							new_phones[i].state[0].pdf[j].var[k] = 0;

						}

					}
					for (int j = 0; j < 3; j++) {
						for (int k = 0; k < 3; k++) {
							new_phones[i].tp[j][k] = 0;

						}
					}
				}
				else {
					for (int j = 0; j < N_STATE; j++) {
						for (int k = 0; k < g_num; k++) {
							for (int l = 0; l < N_DIMENSION; l++) {
								new_phones[i].state[j].pdf[k].mean[l] = 0;
								new_phones[i].state[j].pdf[k].var[l] = 0;
							}
						}
					}
					for (int j = 0; j < 5; j++) {
						for (int k = 0; k < 5; k++) {
							new_phones[i].tp[j][k] = 0;
						}
					}
				}
			}

			for (int i = 0; i < 21; i++) {	//occupancy 초기화
				for (int j = 0; j < 3; j++) {
					for (int k = 0; k < 11; k++) {
						s_oc[i][j][k] = M_INFINITY;
					}
				}
			}

			for (int i = 0; i <scr_num; i++) {//학습 시작

				float **a;	//utteran hmm의 trasition matrix
				long double ***b;	// [state][time][pdf]의 인덱스를 통해서 utterance hmm의 emmision probability 저장. pdf가 0일 떄 모든 gaussian에 대한 합이다.
				float **input; //training data
				long double ll = M_INFINITY;	//iteration의 log likelihood를 저장한다.

				int j;
				int k = 0;
				int l = 0;
				int scr_i = 0;

				char temp[30];

				fopen_s(&fp, file_name[i], "r");
				printf("Proccessing %s...\n", file_name[i]);
				if (fp != NULL) {

					fscanf_s(fp, "%d", &input_length);
					fscanf_s(fp, "%d", &skip);


					input = (float **)malloc(sizeof(float *)*input_length);		// input file의 vector 갯수만큼 row 할당

					for (int i = 0; i < input_length; i++)		//input file의 dimension 만큼 column 할당	

						input[i] = (float *)malloc(sizeof(float)*N_DIMENSION);



					for (int i = 0; i < input_length; i++) 	//input 배열에 정보 저장
						for (int j = 0; j < N_DIMENSION; j++)
							fscanf_s(fp, "%f", &input[i][j]);

				}
				else
					printf("error");

				fclose(fp);

				for (j = 1; file_name[i][j] != '\0'; j++) {
					if (k == 4) {
						temp[l] = file_name[i][j];
						l++;
					}
					if (file_name[i][j] == '\\')
						k++;
				}
				temp[l - 4] = '\0';

				while (1) {
					if (strstr(scr[scr_i].name, temp) != NULL)
						break;
					scr_i++;
				}


				a = (float **)malloc(sizeof(float *)*(scr[scr_i].state_num + 2));

				for (int i = 0; i < scr[scr_i].state_num + 2; i++)
					a[i] = (float *)malloc(sizeof(float)*(scr[scr_i].state_num + 2));


				b = (long double ***)malloc(sizeof(long double **)*(scr[scr_i].state_num));


				for (int i = 0; i < scr[scr_i].state_num; i++) {

					b[i] = (long double **)malloc(sizeof(long double*)*input_length);
					for (int j = 0; j < input_length; j++)
						b[i][j] = (long double *)malloc(sizeof(long double)*(g_num + 1));
				}


				a_mix(a, scr, scr_i, phones_up);	//utterance hmm의 transition matrix를 구하며 

				b_mix(b, input, scr, scr_i, phones_up, input_length, g_num);	//utterance hmm의 emmision probability matrix를 구한다.

				accumulate(a, b, s_oc, input, scr_i, input_length, g_num, &ll, scr, new_phones);	//Baum-Welch에서 Accumulation과정을 실행한다.

				avg_ll = logsum(avg_ll, ll); //log likelihood 누적

				for (int u = 0; u < input_length; u++)
					free(input[u]);
				free(input);

				for (int u = 0; u < scr[scr_i].state_num + 2; u++)
					free(a[u]);
				free(a);

				for (int u = 0; u < scr[scr_i].state_num; u++) {
					for (int v = 0; v < input_length; v++)
						free(b[u][v]);
					free(b[u]);
				}
				free(b);

			}
			avg_ll = avg_ll - logl(scr_num);	//log likehood 계산


			update(s_oc, new_phones, g_num); //Baum Welch에서 update과정을 실행한다.

			new_phones[17].tp[0][1] = phones[17].tp[0][1];
			new_phones[17].tp[0][2] = phones[17].tp[0][2];

			for (int i = 0; i < 21; i++) {	//phone_up에 new_phone 복사, 즉 누적된 결과를 복사한다.

				if (phones[i].name == "sp") {
					for (int j = 0; j < g_num; j++) {
						for (int k = 0; k < N_DIMENSION; k++) {

							phones_up[i].state[0].pdf[j].mean[k] = new_phones[i].state[0].pdf[j].mean[k];
							phones_up[i].state[0].pdf[j].var[k] = new_phones[i].state[0].pdf[j].var[k];

						}
						phones_up[i].state[0].pdf[j].weight = new_phones[i].state[0].pdf[j].weight;
					}
					for (int j = 0; j < 3; j++) {
						for (int k = 0; k < 3; k++) {

							phones_up[i].tp[j][k] = new_phones[i].tp[j][k];
						}
					}
				}
				else {
					for (int j = 0; j < N_STATE; j++) {
						for (int k = 0; k < g_num; k++) {
							for (int l = 0; l < N_DIMENSION; l++) {

								phones_up[i].state[j].pdf[k].mean[l] = new_phones[i].state[j].pdf[k].mean[l];
								phones_up[i].state[j].pdf[k].var[l] = new_phones[i].state[j].pdf[k].var[l];
							}
							phones_up[i].state[j].pdf[k].weight = new_phones[i].state[j].pdf[k].weight;
						}

					}
					for (int j = 0; j < 5; j++) {
						for (int k = 0; k < 5; k++) {

							phones_up[i].tp[j][k] = new_phones[i].tp[j][k];
						}
					}
				}

			}

			file_update(phones_up, g_num);	//hmm.txt에 학습 결과를 update한다.
		}
		else
			break;

		system("HVite -T 1 -C etc/configuration -p -40 -s 6 -w etc/bigram -i recognized.txt -S etc/data_list -H hmm.txt etc/dictionary etc/hmm_list"); //학습이 끝난 후 HVite실행
		system("cls");
		system("HResults -p -I etc/transcript etc/vocabulary recognized.txt");	//confusion matrix를 생성하기 위해 HResuls 실행
		printf("%d번째 학습의 confusion matrix입니다.\n", n + 1);
		printf("학습전 average likelihood는 %f 입니다.\n", avg_ll);

		if ((n + 1) % 5 == 0) { //매 5번 iteration 마다 가우시안을 split할 것인지 물어본다.
			printf("%d 학습이 끝났습니다. 가우시안 스플릿을 하려면 o를 아니면 다른 키를 눌러주세요\n", n + 1);
			c = getch();
			if (c == 'o') {
				gaussian_split(phones_up, g_num);	//가우시안을 두개로 나눠준다.
				g_num++;	//가우시안 한개 추가
			}

		}

		n++;
	}

	return;

}
void filesearch(char file_path[], char file_name[][_MAX_PATH], int *index) {	//trn 디렉토리 내부에 있는 파일 검색한다.
	intptr_t handle;
	int check = 0;
	char file_path2[_MAX_PATH];

	strcat(file_path, "\\");
	strcpy(file_path2, file_path);
	strcat(file_path, "*");
	if ((handle = _findfirst(file_path, &fd)) == -1)
	{
		printf("No such file or directory\n");
		return;
	}

	while (_findnext(handle, &fd) == 0)
	{
		char file_pt[_MAX_PATH];
		strcpy(file_pt, file_path2);
		strcat(file_pt, fd.name);

		if (fd.attrib & _A_SUBDIR)
			check = 0; // 디렉토리면 0 반환
		else
			check = 1; // 그밖의 경우는 존재하는 파일이기에 1 반환


		if (check == 0 && fd.name[0] != '.')
		{
			filesearch(file_pt, file_name, &(*index));    //하위 디렉토리 검색
		}
		else if (check == 1 && fd.size != 0 && fd.name[0] != '.')
		{
			strcpy(file_name[*index], file_pt);
			(*index)++;
		}
	}
	_findclose(handle);


}
void script_read(script *scr) {	//script에 대한 정보를 scr배열에 저장한다.
	FILE *fp;
	char str[30];
	char *line_p;

	int state_num = 0;
	int i = 0;
	int j = 0;
	int m = 0;

	fopen_s(&fp, "trn_mono.txt", "r");

	if (fp != NULL) {

		fgets(str, 20, fp);

		while (fgets(str, 30, fp) != NULL) {
			if (str[0] != '"' && str[0] != '.') {
				i++;
			}
			else if (str[0] == '.') {

				scr[j].phone_num = i;
				scr[j].phone = (char**)malloc(sizeof(char*) * i);
				for (int m = 0; m < i; m++) {
					scr[j].phone[m] = (char*)malloc(sizeof(char) * 3);
				}
				i = 0;
				j++;
			}
		}
		rewind(fp);
		i = 0;
		j = 0;

		fgets(str, 20, fp);
		while (fgets(str, 30, fp) != NULL) {

			if (str[0] != '"' && str[0] != '.') {
				if ((line_p = strchr(str, '\n')) != NULL)*line_p = '\0';
				strcpy(scr[j].phone[i], str);
				if (strcmp("sp", scr[j].phone[i]) == 0)
					state_num++;
				else
					state_num += 3;
				i++;
			}
			else if (str[0] == '"') {
				if ((line_p = strchr(str, '\n')) != NULL)*line_p = '\0';
				for (m = 1; str[m] != '"'; m++)
					str[m - 1] = str[m];
				str[m - 1] = str[m + 1];
				strcpy(scr[j].name, str);
			}
			else if (str[0] == '.') {
				scr[j].phone_num = i;
				scr[j].state_num = state_num;
				scr[j].state_seq = (stateType1*)malloc(sizeof(stateType1) * state_num);
				scr[j].state_inf = (goback*)malloc(sizeof(goback)*state_num);
				state_num = 0;
				i = 0;
				j++;
			}
		}
	}


}

void a_mix(float **a, script *scr, int scr_i, hmmType1 *phones) { //utterance hmm의 transition matrix를 생성하고 scr배열에 state sequence에 대한 정보와 backtracking을 위한 정보 또한 저장한다
	int phones_index = 0;	//몇번째 phone인지에 대한 index
	int a_index = 0;	//transition matrix의 몇번째 요소를 다루고 있는지에 대한 index
	int state_index = 0;	//utterance hmm에서 몇번째 state인지를 저장하는 index
	int asd = 0;
	float temp_3_3 = 0;
	float temp_3_4 = 1;

	for (int i = 0; i < scr[scr_i].state_num + 2; i++)
		for (int j = 0; j < scr[scr_i].state_num + 2; j++)
			a[i][j] = 0;

	for (int i = 0; i < scr[scr_i].phone_num; i++) {

		for (phones_index = 0; strcmp(scr[scr_i].phone[i], phones[phones_index].name) != 0; phones_index++) {}


		if (strcmp(phones[phones_index].name, "sp") != 0) {	//sp인 경우와 sp가 아닌 경우를 나눈다. sp의 state 갯수가 나머지와 다르기 때문이다.
			for (int m = 0; m < 5; m++) {
				for (int n = 0; n < 5; n++) {
					if (i != scr[scr_i].phone_num - 1) {
						if (m == 0 && n == 0)
							a[a_index + m][a_index + n] = temp_3_3;
						else if (m == 0 && n == 1) {
							a[a_index + m][a_index + n] = temp_3_4*phones[phones_index].tp[m][n];
						}
						else if (m < 3 || n < 3)
							a[a_index + m][a_index + n] = phones[phones_index].tp[m][n];
					}
					else
					{

						if (m == 0 && n == 0)
							a[a_index + m][a_index + n] = temp_3_3;
						else if (m == 0 && n == 1) {
							a[a_index + m][a_index + n] = temp_3_4*phones[phones_index].tp[m][n];
						}
						else {
							a[a_index + m][a_index + n] = phones[phones_index].tp[m][n];

						}
					}



				}
			}
			for (int k = 0; k < 3; k++) {

				scr[scr_i].state_seq[state_index + k] = phones[phones_index].state[k];
				scr[scr_i].state_inf[state_index + k].phone_i = phones_index;
				scr[scr_i].state_inf[state_index + k].state_i = k;

			}
			temp_3_3 = phones[phones_index].tp[3][3];
			temp_3_4 = phones[phones_index].tp[3][4];
			state_index += 3;
			a_index += 3;
		}
		else {
			for (int m = 0; m < 3; m++) {
				for (int n = 0; n < 3; n++) {
					if (m == 0 && n == 0)
						a[a_index + m][a_index + n] = temp_3_3;
					else if (m == 0 && n == 1)
						a[a_index + m][a_index + n] = temp_3_4*phones[phones_index].tp[m][n];
					else if (m == 0 && n == 2)
						a[a_index + m][a_index + n] = temp_3_4*phones[phones_index].tp[m][n];
					else if (m < 1 || n < 1)
						a[a_index + m][a_index + n] = phones[phones_index].tp[m][n];

				}
			}
			scr[scr_i].state_seq[state_index] = phones[phones_index].state[0];
			scr[scr_i].state_inf[state_index].phone_i = phones_index;
			scr[scr_i].state_inf[state_index].state_i = 0;

			temp_3_3 = phones[phones_index].tp[1][1];
			temp_3_4 = phones[phones_index].tp[1][2];
			state_index += 1;
			a_index += 1;
		}
	}
	return;
}

void b_mix(long double ***b, float **input, script *scr, int scr_i, hmmType1 *phones, int input_length, int g_num) {	//utterance hmm의 emmision probability matrix 계산

	long double *sum;	//각 pdf에 대한 결과 값
	long double fac = 0;	//모든 차원의 표준편차에 대한 곱 저장

	sum = (long double *)malloc(sizeof(long double)*g_num);

	for (int i = 0; i < scr[scr_i].state_num; i++) {
		for (int j = 0; j < input_length; j++) {
			long double t_sum = M_INFINITY;

			for (int l = 0; l < g_num; l++) {//underflow를 방지하기 위해 각 pdf에 대한 결과 값을 log domain에서 계산한다. 
				sum[l] = 0;
				fac = 0;
				for (int k = 0; k < N_DIMENSION; k++) {
					sum[l] += powl(input[j][k] - scr[scr_i].state_seq[i].pdf[l].mean[k], 2) / scr[scr_i].state_seq[i].pdf[l].var[k];
					fac += logl(scr[scr_i].state_seq[i].pdf[l].var[k]);
				}




				sum[l] = sum[l] / (-2);
				fac = fac / 2;
				fac += (N_DIMENSION / 2)*logl(2 * M_PI);
				sum[l] = logl(scr[scr_i].state_seq[i].pdf[l].weight) - fac + sum[l];


				b[i][j][l + 1] = sum[l];

			}

			for (int l = 0; l < g_num; l++) {	//모든 pdf에 대해서 더한다.
				t_sum = logsum(t_sum, sum[l]);
			}
			b[i][j][0] = t_sum;
		}
	}

	free(sum);


}

void forward(long double ***b, float **a, long double **f, script *scr, int scr_i, int input_length, long double *p, long double *ll) {//forward probability matrix를 계산한다
	long double sum = 0;
	int all_zero;	//다 0이면 1
	long double max = 0;	//underflow를 처리하기 위해 최댓값을 저장한다.

	for (int s = 0; s < scr[scr_i].state_num; s++) {	//foward probability 초기화
		if (a[0][s + 1] == 0)
			f[s][0] = M_INFINITY;
		else
			f[s][0] = b[s][0][0] + logl(a[0][s + 1]);


	}

	for (int t = 1; t < input_length; t++) {	//underflow를 방지하기 위해서 lod domain에서 forward probability 계산
		for (int s = 0; s < scr[scr_i].state_num; s++)
		{
			all_zero = 1;
			sum = 0;
			max = M_INFINITY;
			for (int s1 = 0; s1 < scr[scr_i].state_num; s1++) {
				if ((f[s1][t - 1] != M_INFINITY) && (a[s1 + 1][s + 1] != 0)) {
					if (max < f[s1][t - 1] + logl(a[s1 + 1][s + 1]))
						max = f[s1][t - 1] + logl(a[s1 + 1][s + 1]);
					all_zero = 0;
				}
			}
			for (int s1 = 0; s1 < scr[scr_i].state_num; s1++) {

				if ((f[s1][t - 1] != M_INFINITY) && (a[s1 + 1][s + 1] != 0)) {

					if (max != f[s1][t - 1] + logl(a[s1 + 1][s + 1]))
						sum += expl(f[s1][t - 1] + logl(a[s1 + 1][s + 1]) - max);
					else
						sum += 1;

				}

			}

			if (all_zero == 1)	//로그 안이 전부 0이면
				f[s][t] = M_INFINITY;
			else 	//로그 안이 0이 아니면
				f[s][t] = b[s][t][0] + max + logl(sum);
		}
	}
	sum = M_INFINITY;
	/*max = M_INFINITY;
	sum = 0;
	for (int s = 0; s < scr[scr_i].state_num; s++) {	//log likelihood(p)를 계산
		if (f[s][input_length - 1] != M_INFINITY && a[s + 1][scr[scr_i].state_num + 1] != 0) {
			if (max < f[s][input_length - 1] + logl(a[s + 1][scr[scr_i].state_num + 1]))
				max = f[s][input_length - 1] + logl(a[s + 1][scr[scr_i].state_num + 1]);
		}
	}
	for (int s = 0; s < scr[scr_i].state_num; s++) {
		if (f[s][input_length - 1] != M_INFINITY && a[s + 1][scr[scr_i].state_num + 1] != 0) {
			if (max != f[s][input_length - 1] + logl(a[s + 1][scr[scr_i].state_num + 1]))
				sum += expl(f[s][input_length - 1] + logl(a[s + 1][scr[scr_i].state_num + 1]) - max);
			else sum += 1;
		}
	}*/
	for (int s = 0; s < scr[scr_i].state_num; s++) {
		if (f[s][input_length - 1] != M_INFINITY && a[s + 1][scr[scr_i].state_num + 1] != 0)
		sum = logsum(sum, f[s][input_length - 1] + logl(a[s + 1][scr[scr_i].state_num + 1]));
	}
	//*p = max + logl(sum);
	*p = sum;
	
	*ll = *p;

}

void backward(long double ***b, float **a, long double **back, script *scr, int scr_i, int input_length) {	//backward probability matrix를 계산한다.
	int all_zero;

	long double sum = 0;
	long double max = 0;
	long double p = 0;

	for (int s = 0; s < scr[scr_i].state_num; s++) {

		if (a[s + 1][scr[scr_i].state_num + 1] == 0)
			back[s][input_length - 1] = M_INFINITY;
		else
			back[s][input_length - 1] = logl(a[s + 1][scr[scr_i].state_num + 1]);

	}

	for (int t = input_length - 2; t >= 0; t--) {
		for (int s = 0; s < scr[scr_i].state_num; s++)
		{
			all_zero = 1;
			max = M_INFINITY;
			sum = 0;

			for (int s1 = 0; s1 < scr[scr_i].state_num; s1++) {

				if ((back[s1][t + 1] != M_INFINITY) && (a[s + 1][s1 + 1] != 0)) {

					if (max < b[s1][t + 1][0] + back[s1][t + 1] + logl(a[s + 1][s1 + 1]))
						max = b[s1][t + 1][0] + back[s1][t + 1] + logl(a[s + 1][s1 + 1]);

					all_zero = 0;
				}
			}
			for (int s1 = 0; s1 < scr[scr_i].state_num; s1++) {
				if ((back[s1][t + 1] != M_INFINITY) && (a[s + 1][s1 + 1] != 0)) {
					if (max != b[s1][t + 1][0] + logl(a[s + 1][s1 + 1]) + back[s1][t + 1])
						sum += expl(b[s1][t + 1][0] + logl(a[s + 1][s1 + 1]) + back[s1][t + 1] - max);
					else
						sum += 1;

				}

			}

			if (all_zero == 1)	//로그 안이 전부 0이면
				back[s][t] = M_INFINITY;
			else	//로그 안이 0이 아니면
				back[s][t] = max + logl(sum);

		}
	}
	max = M_INFINITY;
	sum = 0;
	for (int s = 0; s < scr[scr_i].state_num; s++) {
		if (back[s][0] != M_INFINITY && a[0][s + 1] != 0) {
			if (max < back[s][0] + logl(a[0][s + 1]) + b[s][0][0])
				max = back[s][0] + logl(a[0][s + 1]) + b[s][0][0];
		}
	}
	for (int s = 0; s < scr[scr_i].state_num; s++) {
		if (back[s][0] != M_INFINITY && a[0][s + 1] != 0) {
			if (max != back[s][0] + logl(a[0][s + 1]) + b[s][0][0])
				sum += expl(back[s][0] + logl(a[0][s + 1]) + b[s][0][0] - max);
			else
				sum += 1;
		}
	}
	p = max + logl(sum);

}
void accumulate(float **a, long double ***b, long double s_oc[][3][11], float **input, int scr_i, int input_length, int g_num, long double *ll, script *scr, hmmType1 *new_phones) {

	long double **f;	//forward 확률을 저장한다
	long double **back;	//backward 확률을 저장한다.
	long double p = 0;	//loglikelihood 저장한다.

	long double **a1;	//
	long double ***mean;//[state][pdf][dim]의 인덱스를 통해 평균을 누적시킨다.
	long double ***var;//[state][pdf][dim]의 인덱스를 통해 분산을 누적시킨다.
					   //밑엔 초기화하는 작업
	f = (long double **)malloc(sizeof(long double *)*(scr[scr_i].state_num));
	for (int i = 0; i < scr[scr_i].state_num; i++)
		f[i] = (long double *)malloc(sizeof(long double)*input_length);

	back = (long double **)malloc(sizeof(long double *)*(scr[scr_i].state_num));
	for (int i = 0; i < scr[scr_i].state_num; i++)
		back[i] = (long double *)malloc(sizeof(long double)*input_length);
	a1 = (long double **)malloc(sizeof(long double *)*(scr[scr_i].state_num + 2));
	for (int i = 0; i < scr[scr_i].state_num + 2; i++)
		a1[i] = (long double *)malloc(sizeof(long double)*(scr[scr_i].state_num + 2));


	mean = (long double ***)malloc(sizeof(long double **)*(scr[scr_i].state_num));
	for (int i = 0; i < scr[scr_i].state_num; i++) {
		mean[i] = (long double**)malloc(sizeof(long double*) * g_num);
		for (int j = 0; j < g_num; j++)
			mean[i][j] = (long double *)malloc(sizeof(long double) * N_DIMENSION);
	}

	var = (long double***)malloc(sizeof(long double**)*(scr[scr_i].state_num));
	for (int i = 0; i < scr[scr_i].state_num; i++) {
		var[i] = (long double**)malloc(sizeof(long double*) * g_num);
		for (int j = 0; j < g_num; j++)
			var[i][j] = (long double *)malloc(sizeof(long double) * N_DIMENSION);
	}

	for (int i = 0; i < scr[scr_i].state_num + 2; i++) {
		for (int j = 0; j < scr[scr_i].state_num + 2; j++)
			a1[i][j] = M_INFINITY;
	}

	for (int i = 0; i < scr[scr_i].state_num; i++) {
		for (int j = 0; j < g_num; j++) {
			for (int k = 0; k < N_DIMENSION; k++) {
				mean[i][j][k] = 0;
				var[i][j][k] = 0;
			}
		}
	}

	forward(b, a, f, scr, scr_i, input_length, &p, &(*ll)); //forward 확률및 log likelihood를 계산한다.
	backward(b, a, back, scr, scr_i, input_length); //backward 확률을 계산한다.

	for (int t = 0; t < input_length; t++) {
		for (int s = 0; s < scr[scr_i].state_num; s++) {
			long double s_oc_tio = M_INFINITY;	//
			long double s_oc_tik;	//time t에서 state i와 k번째 gaussian에 대한 state occupancy 계산.
			long double a_oc_tij;	//time t에서 state i,j에 대한 occupancy 확률
			int pi = scr[scr_i].state_inf[s].phone_i;
			int si = scr[scr_i].state_inf[s].state_i;

			if (f[s][t] != M_INFINITY && back[s][t] != M_INFINITY)	//로그 도메인에서 계산한다.
				s_oc_tio = f[s][t] + back[s][t] - p;
			else
				s_oc_tio = M_INFINITY;

			s_oc[pi][si][0] = logsum(s_oc_tio, s_oc[pi][si][0]);




			if (t == 1) {	//transition 누적
				a1[0][s + 1] = logsum(s_oc[pi][si][0], a1[0][s + 1]);
			}

			if (t != input_length - 1) {	//transition 누적
				for (int s1 = 0; s1 < scr[scr_i].state_num; s1++) {

					if ((a[s + 1][s1 + 1] != 0) && (f[s][t] != M_INFINITY) && (back[s1][t + 1] != M_INFINITY))
						a_oc_tij = f[s][t] + logl(a[s + 1][s1 + 1]) + b[s1][t + 1][0] + back[s1][t + 1] - p;
					else
						a_oc_tij = M_INFINITY;

					a1[s + 1][s1 + 1] = logsum(a_oc_tij, a1[s + 1][s1 + 1]);

				}

			}

			for (int g = 0; g < g_num; g++) {	//평균,분산,state occupancy 누적
				if (s_oc_tio != M_INFINITY)
					s_oc_tik = s_oc_tio + b[s][t][g + 1] - b[s][t][0];
				else
					s_oc_tik = M_INFINITY;

				s_oc[pi][si][g + 1] = logsum(s_oc[pi][si][g + 1], s_oc_tik);

				if (s_oc_tik != M_INFINITY) {
					for (int d = 0; d < N_DIMENSION; d++) {
						mean[s][g][d] += expl(s_oc_tik)*input[t][d];
						var[s][g][d] += expl(s_oc_tik)*powl(input[t][d], 2);

					}
				}
			}



		}
	}



	for (int i = 0; i < scr[scr_i].state_num; ) {	//transition을 누적한 결과를 new_phones에 복사해준다.
		int p = scr[scr_i].state_inf[i].phone_i;
		int s = scr[scr_i].state_inf[i].state_i;


		if (p != 17) {
			for (int m = 0; m < 3; m++) {
				for (int n = 0; n < 3; n++) {
					new_phones[p].tp[m + 1][n + 1] += expl(a1[i + m + 1][i + n + 1]);
				}
			}
			i += 3;
		}
		else {
			new_phones[p].tp[1][1] += expl(a1[i + 1][i + 1]);
			i += 1;
		}
	}


	for (int i = 0; i < scr[scr_i].state_num; i++) {	//평균과 분산을 누적한 결가를 new_phones에 복사해준다.
		int x = scr[scr_i].state_inf[i].phone_i;
		int y = scr[scr_i].state_inf[i].state_i;
		for (int j = 0; j < g_num; j++) {

			for (int k = 0; k < N_DIMENSION; k++) {
				new_phones[x].state[y].pdf[j].var[k] += var[i][j][k];
				new_phones[x].state[y].pdf[j].mean[k] += mean[i][j][k];
			}

		}
	}

	for (int i = 0; i < scr[scr_i].state_num; i++) {
		free(f[i]);
		free(back[i]);
	}
	for (int i = 0; i < scr[scr_i].state_num + 2; i++) {
		free(a1[i]);
	}
	for (int i = 0; i < scr[scr_i].state_num; i++) {
		for (int j = 0; j <g_num; j++) {
			free(mean[i][j]);
			free(var[i][j]);
		}
		free(mean[i]);
		free(var[i]);

	}

	free(mean);
	free(var);
	free(a1);
	free(f);
	free(back);
	return;
}

void update(long double s_oc[][3][11], hmmType1 *new_phones, int g_num) {
	long double x;
	for (int i = 0; i < 21; i++) {

		if (i == 17) {	//sp일때의 update.
			new_phones[i].tp[1][1] = new_phones[i].tp[1][1] / expl(s_oc[i][0][0]);
			new_phones[i].tp[1][2] = 1 - new_phones[i].tp[1][1];
			for (int k = 0; k < g_num;k++) {
				new_phones[i].state[0].pdf[k].weight = expl(s_oc[i][0][k + 1]) / expl(s_oc[i][0][0]);	//weight update

				for (int d = 0; d < N_DIMENSION; d++) {

					new_phones[i].state[0].pdf[k].mean[d] = new_phones[i].state[0].pdf[k].mean[d] / expl(s_oc[i][0][k + 1]);	//평균 update

					new_phones[i].state[0].pdf[k].var[d] = (new_phones[i].state[0].pdf[k].var[d] / expl(s_oc[i][0][k + 1])) - powl(new_phones[i].state[0].pdf[k].mean[d], 2);//분산 update


				}
			}
		}

		else {	//나머지에 대한 update

			for (int j = 0; j < 3; j++) {	//transition update

				for (int k = 0; k < 3; k++) {
					new_phones[i].tp[j + 1][k + 1] = new_phones[i].tp[j + 1][k + 1] / expl(s_oc[i][j][0]);
				}
				new_phones[i].tp[0][1] = 1;
				new_phones[i].tp[3][4] = 1 - (new_phones[i].tp[3][1] + new_phones[i].tp[3][2] + new_phones[i].tp[3][3]);

				for (int k = 0; k < g_num; k++) {
					new_phones[i].state[j].pdf[k].weight = expl(s_oc[i][j][k + 1]) / expl(s_oc[i][j][0]);//weight update

					for (int d = 0; d < N_DIMENSION; d++) {	//평균과 분산 update

						new_phones[i].state[j].pdf[k].mean[d] = new_phones[i].state[j].pdf[k].mean[d] / expl(s_oc[i][j][k + 1]);
						x = new_phones[i].state[j].pdf[k].var[d];

						new_phones[i].state[j].pdf[k].var[d] = (new_phones[i].state[j].pdf[k].var[d] / expl(s_oc[i][j][k + 1])) - powl(new_phones[i].state[j].pdf[k].mean[d], 2);

					}

				}


			}
		}
	}
	
}
void file_update(hmmType1 *phones_up, int g_num) {	//hmm.txt에 model을 update시켜준다.
	FILE *fp = NULL;
	
	int result = remove("hmm.txt");

	if (result == 0)
		printf("성공\n");
	else
		printf("실패\n");


	fp= fopen("hmm.txt", "w");

	fprintf(fp, "~o\n");
	fprintf(fp, "<VECSIZE> 39 <NULLD> <MFCC_D_A_0> <DIAGC>\n");
	fprintf(fp, "~v \"varFloor1\"\n");
	fprintf(fp, "<VARIANCE> 39\n");
	fprintf(fp, " 1.155000e+000 7.543587e-001 8.050661e-001 9.630864e-001 8.041837e-001 8.106853e-001 7.842855e-001 7.111993e-001 7.518033e-001 5.864958e-001 7.210582e-001 4.245332e-001 1.658078e+000 4.303107e-002 3.307829e-002 3.192059e-002 3.902683e-002 3.759940e-002 3.475500e-002 3.803094e-002 3.948206e-002 3.604316e-002 3.104777e-002 3.077290e-002 2.320139e-002 4.205974e-002 5.578214e-003 5.010017e-003 4.579471e-003 5.877771e-003 5.709495e-003 5.665889e-003 6.231423e-003 6.701753e-003 5.970786e-003 5.396303e-003 5.125446e-003 4.012546e-003 5.954688e-003\n");

	for (int i = 0; i < 21; i++) {

		if (i == 17) {
			fprintf(fp, "~h \"%s\"\n", phones[i].name);
			fprintf(fp, "<BEGINHMM>\n");
			fprintf(fp, "<NUMSTATES> 3\n");
			for (int j = 0; j < 1; j++) {
				fprintf(fp, "<STATE> %d\n", j + 2);
				fprintf(fp, "<NUMMIXES> %d\n", g_num);
				for (int k = 0; k < g_num; k++) {
					fprintf(fp, "<MIXTURE> %d %e\n", k + 1, phones_up[i].state[j].pdf[k].weight);
					fprintf(fp, "<MEAN> 39\n");
					for (int l = 0; l < N_DIMENSION; l++) {
						fprintf(fp, " %e", phones_up[i].state[j].pdf[k].mean[l]);
					}
					fprintf(fp, "\n<VARIANCE> 39\n");
					for (int l = 0; l < N_DIMENSION; l++) {
						fprintf(fp, " %e", phones_up[i].state[j].pdf[k].var[l]);
					}
					fprintf(fp, "\n");
				}
			}
			fprintf(fp, "<TRANSP> 3\n");
			for (int a = 0; a < 3; a++) {
				for (int b = 0; b < 3; b++) {
					fprintf(fp, " %e", phones_up[i].tp[a][b]);
				}
				fprintf(fp, "\n");
			}
			fprintf(fp, "<ENDHMM>\n");

		}
		else {
			fprintf(fp, "~h \"%s\"\n", phones[i].name);
			fprintf(fp, "<BEGINHMM>\n");
			fprintf(fp, "<NUMSTATES> 5\n");

			for (int j = 0; j < 3; j++) {
				fprintf(fp, "<STATE> %d\n", j + 2);
				fprintf(fp, "<NUMMIXES> %d\n", g_num);
				for (int k = 0; k < g_num; k++) {
					fprintf(fp, "<MIXTURE> %d %e\n", k + 1, phones_up[i].state[j].pdf[k].weight);
					fprintf(fp, "<MEAN> 39\n");
					for (int l = 0; l < N_DIMENSION; l++) {
						fprintf(fp, " %e", phones_up[i].state[j].pdf[k].mean[l]);
					}
					fprintf(fp, "\n<VARIANCE> 39\n");
					for (int l = 0; l < N_DIMENSION; l++) {
						fprintf(fp, " %e", phones_up[i].state[j].pdf[k].var[l]);
					}
					fprintf(fp, "\n");
				}
			}
			fprintf(fp, "<TRANSP> 5\n");
			for (int a = 0; a < 5; a++) {
				for (int b = 0; b < 5; b++) {
					fprintf(fp, " %e", phones_up[i].tp[a][b]);
				}
				fprintf(fp, "\n");
			}
			fprintf(fp, "<ENDHMM>\n");

		}
	}
	fclose(fp);
}
void gaussian_split(hmmType1 *phones_up, int g_num) {	//가우시안을 split한다.

	srand(time(NULL));

	for (int i = 0; i < 21; i++) {
		if (i == 17) {	//sp일 때
			int max_index;	//weight가 최대인 pdf의 index를 저장한다.
			float max_weight = 0; //최대 weight을 저장한다.
			for (int k = 0; k < g_num; k++) {
				if (max_weight < phones_up[i].state[0].pdf[k].weight) {	//최대 weight 찾기
					max_weight = phones_up[i].state[0].pdf[k].weight;
					max_index = k;
				}
			}
			phones_up[i].state[0].pdf[max_index].weight = max_weight / 2;	//weight 분배
			phones_up[i].state[0].pdf[g_num].weight = max_weight / 2;

			for (int l = 0; l < N_DIMENSION; l++) {//평균에 small random vector 더하기
				phones_up[i].state[0].pdf[g_num].mean[l] = phones_up[i].state[0].pdf[max_index].mean[l];
				phones_up[i].state[0].pdf[g_num].var[l] = phones_up[i].state[0].pdf[max_index].var[l];
				if (rand() % 2 == 0) {
					phones_up[i].state[0].pdf[max_index].mean[l] += (float)(rand() % 9973) / 99730;
					phones_up[i].state[0].pdf[g_num].mean[l] += (float)(rand() % 9973) / 99730;
				}
				else {
					phones_up[i].state[0].pdf[max_index].mean[l] -= (float)(rand() % 9973) / 99730;
					phones_up[i].state[0].pdf[g_num].mean[l] -= (float)(rand() % 9973) / 99730;
				}

			}

		}
		else {	//sp가 아닐 때
			for (int j = 0; j < 3; j++) {
				int max_index;
				float max_weight = 0;
				for (int k = 0; k < g_num; k++) {
					if (max_weight < phones_up[i].state[j].pdf[k].weight) {	//최대 weight 찾기
						max_weight = phones_up[i].state[j].pdf[k].weight;
						max_index = k;
					}
				}
				phones_up[i].state[j].pdf[max_index].weight = max_weight / 2;	//weight 분배
				phones_up[i].state[j].pdf[g_num].weight = max_weight / 2;

				for (int l = 0; l < N_DIMENSION; l++) {//평균에 small random vector 더하기
					phones_up[i].state[j].pdf[g_num].mean[l] = phones_up[i].state[j].pdf[max_index].mean[l];
					phones_up[i].state[j].pdf[g_num].var[l] = phones_up[i].state[j].pdf[max_index].var[l];
					if (rand() % 2 == 0) {
						phones_up[i].state[j].pdf[max_index].mean[l] += (float)(rand() % 9973) / 99730;
						phones_up[i].state[j].pdf[g_num].mean[l] += (float)(rand() % 9973) / 99730;
					}
					else {

						phones_up[i].state[j].pdf[max_index].mean[l] -= (float)(rand() % 9973) / 99730;
						phones_up[i].state[j].pdf[g_num].mean[l] -= (float)(rand() % 9973) / 99730;

					}
				}

			}
		}
	}
}
long double logsum(long double x, long double y) {	//x=logA,y=logB이면 logsum(x,y)는 log(A+B)를 반환한다
	if (x <= M_INFINITY&&y <= M_INFINITY)
		return M_INFINITY;
	else if (x <= M_INFINITY&& y > M_INFINITY)
		return y;
	else if (x > M_INFINITY && y <= M_INFINITY)
		return x;
	else {
		if (x > y)
			return x + logl(1 + expl(y - x));
		else
			return y + logl(1 + expl(x - y));
	}
}
