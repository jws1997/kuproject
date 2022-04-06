/*
��ü���� �帧
1.���� ���� script_read �Լ��� ���ؼ� script�� ���� ������ �а� �����Ѵ�.
2.�� �Ŀ� filesearch�Լ��� ���� trn���丮�� �˻��Ͽ� training data�� ���� ������ �д´�.
3.�� �� ������ ���ؼ� �н��� �����Ѵ�.
4.���� ���� a_mix �Լ��� ���� utterance hmm�� trasition matrix�� ����Ѵ�.
5.�� ������ b_mix �Լ��� ���� utterance hmm�� emmsion probability matrx�� ����Ѵ�.
6.�� �� accumulation�Լ� ������ forward�Լ��� backward �Լ��� ���ؼ� forward�� backward probability, log liklihood�� ����ϸ�
update�� �ʿ��� ������ ���������ش�.
7.���� ������ ������ update�Լ��� ���ؼ� ���� �𵨿� ���� parameter���� ������ش�.
8.�� parameter�� ���� model�� update���ش�.
9.file_update �Լ��� ���ؼ� �� parameter�� hmm.txt�� upload�����ش�.
10.4~9�� ������ ��� �ݺ��ϴµ� 5��° �ݺ����� gaussian_split�Լ��� ���ؼ� �ϳ��� ����þ��� �ΰ��� �ɰ��ش�.
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
#define M_INFINITY -1000000000		//log(0)��� ���
#define N_STATE 3

typedef struct {		//����þ� ������ �÷��� �ϹǷ� ���ο� ����ü ����
	pdfType pdf[10];	//����þ��� �ִ� 10������ �����ϴ�.
} stateType1;
typedef struct {		//���� ������ ������ ���ο� ����ü ����
	char *name;
	float tp[N_STATE + 2][N_STATE + 2];
	stateType1 state[N_STATE];
} hmmType1;

typedef struct {	//�н��� ����� ���� hmm�� �ݿ��ϱ� ���ؼ� ��� �Դ����� ����Ѵ�.
	int phone_i;
	int state_i;
} goback;
typedef struct {	//training data�� ���� ���� script�� �����ϱ� ���� ����ü
	char name[30];	//����(training data) �̸�
	int phone_num;	//phone ����
	char **phone;	//phone �̸� ����
	int state_num;
	goback *state_inf;
	stateType1 *state_seq;	//state�� sequnece
} script;

struct _finddata_t fd;	//���� Ž���� ���� ����ü

void filesearch(char file_path[], char file_name[][_MAX_PATH], int *index);	//trn���� Ž�� �Լ�
void script_read(script *scr); //transcript �б�
void a_mix(float **a, script *scr, int scr_i, hmmType1 *phones);	//utterance hmm transition ���
void b_mix(long double ***b, float **input, script *scr, int scr_i, hmmType1 *phones, int input_length, int g_num);	//utterance hmm emmision ����ϴ� �Լ�
void forward(long double ***b, float **a, long double **f, script *scr, int scr_i, int input_length, long double *p, long double *ll); //forward �� log likelihood ����ϴ� �Լ�
void backward(long double ***b, float **a, long double **back, script *scr, int scr_i, int input_length);	//backward ����ϴ� �Լ�
void accumulate(float **a, long double ***b, long double s_oc[][3][11], float ** input, int scr_i, int input_length, int g_num, long double *ll, script *scr, hmmType1 *new_phones);	//�ٿ� ��ġ������ ������ ����ϴ� �Լ�
void update(long double s_oc[][3][11], hmmType1 *new_phones, int g_num);	//�ٿ� ��ġ���� ������Ʈ �۾��� �ϴ� �Լ�
void file_update(hmmType1 *phones_up, int g_num);	//������Ʈ�� ����� hmm.txt�� �ݿ��ϴ� �Լ�
void gaussian_split(hmmType1 *phones_up, int g_num);	//�� 5ȸ �ݺ����� �ϳ��� ����þ��� �ΰ��� �����ִ� �Լ�
long double logsum(long double x, long double y);

void main() {

	script *scr = NULL;	//script�� ���� ���� ����

	hmmType1 phones_up[21];	//�н��� ����� �ݿ��ϴ� hmm 

	int scr_num = 0;	//training data ����



	FILE *fp;
	char file_path[_MAX_PATH] = ".\\trn";
	char file_name[2000][_MAX_PATH];
	int index = 0;

	int input_length = 0;	//�ϳ��� training data�� �ð��� ����
	int skip;

	int scr_i = 0; //���� �н��ϴ� training data�� scr�� ���° ��������� �����ϴ� index
	int n = 0;	//��� ° �н�����
	int g_num = 2;	//����þ��� ������ �����Ѵ�.

	char str[30];


	fopen_s(&fp, "trn_mono.txt", "r");	//data ���� count
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
	filesearch(file_path, file_name, &index);	//training data�� Ž���Ͽ� �迭�� ����
	fclose(fp);

	for (int i = 0; i < 21; i++) {	// phones_up �ʱ�ȭ

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
		hmmType1 new_phones[21];	//������(accumulation result)�� ������ ��
		long double s_oc[21][3][11];	// [phone][state][pdf]�� �ε����� ���ؼ� gaussian occupance�� ���Ѵ�. pdf�� 0�̸� state occupancy�̴�.
		char c;

		long double avg_ll = M_INFINITY; //average log likelihood�� ����

		printf("%d��° �н��� �����Ϸ��� o�� �����Ϸ��� �ٸ� Ű�� �����ּ���\n", n + 1);
		c = getch();
		if (c == 'o') {
			system("cls");
			for (int i = 0; i < 21; i++) {	//���� ����� ������ new_phone �ʱ�ȭ

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

			for (int i = 0; i < 21; i++) {	//occupancy �ʱ�ȭ
				for (int j = 0; j < 3; j++) {
					for (int k = 0; k < 11; k++) {
						s_oc[i][j][k] = M_INFINITY;
					}
				}
			}

			for (int i = 0; i <scr_num; i++) {//�н� ����

				float **a;	//utteran hmm�� trasition matrix
				long double ***b;	// [state][time][pdf]�� �ε����� ���ؼ� utterance hmm�� emmision probability ����. pdf�� 0�� �� ��� gaussian�� ���� ���̴�.
				float **input; //training data
				long double ll = M_INFINITY;	//iteration�� log likelihood�� �����Ѵ�.

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


					input = (float **)malloc(sizeof(float *)*input_length);		// input file�� vector ������ŭ row �Ҵ�

					for (int i = 0; i < input_length; i++)		//input file�� dimension ��ŭ column �Ҵ�	

						input[i] = (float *)malloc(sizeof(float)*N_DIMENSION);



					for (int i = 0; i < input_length; i++) 	//input �迭�� ���� ����
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


				a_mix(a, scr, scr_i, phones_up);	//utterance hmm�� transition matrix�� ���ϸ� 

				b_mix(b, input, scr, scr_i, phones_up, input_length, g_num);	//utterance hmm�� emmision probability matrix�� ���Ѵ�.

				accumulate(a, b, s_oc, input, scr_i, input_length, g_num, &ll, scr, new_phones);	//Baum-Welch���� Accumulation������ �����Ѵ�.

				avg_ll = logsum(avg_ll, ll); //log likelihood ����

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
			avg_ll = avg_ll - logl(scr_num);	//log likehood ���


			update(s_oc, new_phones, g_num); //Baum Welch���� update������ �����Ѵ�.

			new_phones[17].tp[0][1] = phones[17].tp[0][1];
			new_phones[17].tp[0][2] = phones[17].tp[0][2];

			for (int i = 0; i < 21; i++) {	//phone_up�� new_phone ����, �� ������ ����� �����Ѵ�.

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

			file_update(phones_up, g_num);	//hmm.txt�� �н� ����� update�Ѵ�.
		}
		else
			break;

		system("HVite -T 1 -C etc/configuration -p -40 -s 6 -w etc/bigram -i recognized.txt -S etc/data_list -H hmm.txt etc/dictionary etc/hmm_list"); //�н��� ���� �� HVite����
		system("cls");
		system("HResults -p -I etc/transcript etc/vocabulary recognized.txt");	//confusion matrix�� �����ϱ� ���� HResuls ����
		printf("%d��° �н��� confusion matrix�Դϴ�.\n", n + 1);
		printf("�н��� average likelihood�� %f �Դϴ�.\n", avg_ll);

		if ((n + 1) % 5 == 0) { //�� 5�� iteration ���� ����þ��� split�� ������ �����.
			printf("%d �н��� �������ϴ�. ����þ� ���ø��� �Ϸ��� o�� �ƴϸ� �ٸ� Ű�� �����ּ���\n", n + 1);
			c = getch();
			if (c == 'o') {
				gaussian_split(phones_up, g_num);	//����þ��� �ΰ��� �����ش�.
				g_num++;	//����þ� �Ѱ� �߰�
			}

		}

		n++;
	}

	return;

}
void filesearch(char file_path[], char file_name[][_MAX_PATH], int *index) {	//trn ���丮 ���ο� �ִ� ���� �˻��Ѵ�.
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
			check = 0; // ���丮�� 0 ��ȯ
		else
			check = 1; // �׹��� ���� �����ϴ� �����̱⿡ 1 ��ȯ


		if (check == 0 && fd.name[0] != '.')
		{
			filesearch(file_pt, file_name, &(*index));    //���� ���丮 �˻�
		}
		else if (check == 1 && fd.size != 0 && fd.name[0] != '.')
		{
			strcpy(file_name[*index], file_pt);
			(*index)++;
		}
	}
	_findclose(handle);


}
void script_read(script *scr) {	//script�� ���� ������ scr�迭�� �����Ѵ�.
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

void a_mix(float **a, script *scr, int scr_i, hmmType1 *phones) { //utterance hmm�� transition matrix�� �����ϰ� scr�迭�� state sequence�� ���� ������ backtracking�� ���� ���� ���� �����Ѵ�
	int phones_index = 0;	//���° phone������ ���� index
	int a_index = 0;	//transition matrix�� ���° ��Ҹ� �ٷ�� �ִ����� ���� index
	int state_index = 0;	//utterance hmm���� ���° state������ �����ϴ� index
	int asd = 0;
	float temp_3_3 = 0;
	float temp_3_4 = 1;

	for (int i = 0; i < scr[scr_i].state_num + 2; i++)
		for (int j = 0; j < scr[scr_i].state_num + 2; j++)
			a[i][j] = 0;

	for (int i = 0; i < scr[scr_i].phone_num; i++) {

		for (phones_index = 0; strcmp(scr[scr_i].phone[i], phones[phones_index].name) != 0; phones_index++) {}


		if (strcmp(phones[phones_index].name, "sp") != 0) {	//sp�� ���� sp�� �ƴ� ��츦 ������. sp�� state ������ �������� �ٸ��� �����̴�.
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

void b_mix(long double ***b, float **input, script *scr, int scr_i, hmmType1 *phones, int input_length, int g_num) {	//utterance hmm�� emmision probability matrix ���

	long double *sum;	//�� pdf�� ���� ��� ��
	long double fac = 0;	//��� ������ ǥ�������� ���� �� ����

	sum = (long double *)malloc(sizeof(long double)*g_num);

	for (int i = 0; i < scr[scr_i].state_num; i++) {
		for (int j = 0; j < input_length; j++) {
			long double t_sum = M_INFINITY;

			for (int l = 0; l < g_num; l++) {//underflow�� �����ϱ� ���� �� pdf�� ���� ��� ���� log domain���� ����Ѵ�. 
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

			for (int l = 0; l < g_num; l++) {	//��� pdf�� ���ؼ� ���Ѵ�.
				t_sum = logsum(t_sum, sum[l]);
			}
			b[i][j][0] = t_sum;
		}
	}

	free(sum);


}

void forward(long double ***b, float **a, long double **f, script *scr, int scr_i, int input_length, long double *p, long double *ll) {//forward probability matrix�� ����Ѵ�
	long double sum = 0;
	int all_zero;	//�� 0�̸� 1
	long double max = 0;	//underflow�� ó���ϱ� ���� �ִ��� �����Ѵ�.

	for (int s = 0; s < scr[scr_i].state_num; s++) {	//foward probability �ʱ�ȭ
		if (a[0][s + 1] == 0)
			f[s][0] = M_INFINITY;
		else
			f[s][0] = b[s][0][0] + logl(a[0][s + 1]);


	}

	for (int t = 1; t < input_length; t++) {	//underflow�� �����ϱ� ���ؼ� lod domain���� forward probability ���
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

			if (all_zero == 1)	//�α� ���� ���� 0�̸�
				f[s][t] = M_INFINITY;
			else 	//�α� ���� 0�� �ƴϸ�
				f[s][t] = b[s][t][0] + max + logl(sum);
		}
	}
	sum = M_INFINITY;
	/*max = M_INFINITY;
	sum = 0;
	for (int s = 0; s < scr[scr_i].state_num; s++) {	//log likelihood(p)�� ���
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

void backward(long double ***b, float **a, long double **back, script *scr, int scr_i, int input_length) {	//backward probability matrix�� ����Ѵ�.
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

			if (all_zero == 1)	//�α� ���� ���� 0�̸�
				back[s][t] = M_INFINITY;
			else	//�α� ���� 0�� �ƴϸ�
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

	long double **f;	//forward Ȯ���� �����Ѵ�
	long double **back;	//backward Ȯ���� �����Ѵ�.
	long double p = 0;	//loglikelihood �����Ѵ�.

	long double **a1;	//
	long double ***mean;//[state][pdf][dim]�� �ε����� ���� ����� ������Ų��.
	long double ***var;//[state][pdf][dim]�� �ε����� ���� �л��� ������Ų��.
					   //�ؿ� �ʱ�ȭ�ϴ� �۾�
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

	forward(b, a, f, scr, scr_i, input_length, &p, &(*ll)); //forward Ȯ���� log likelihood�� ����Ѵ�.
	backward(b, a, back, scr, scr_i, input_length); //backward Ȯ���� ����Ѵ�.

	for (int t = 0; t < input_length; t++) {
		for (int s = 0; s < scr[scr_i].state_num; s++) {
			long double s_oc_tio = M_INFINITY;	//
			long double s_oc_tik;	//time t���� state i�� k��° gaussian�� ���� state occupancy ���.
			long double a_oc_tij;	//time t���� state i,j�� ���� occupancy Ȯ��
			int pi = scr[scr_i].state_inf[s].phone_i;
			int si = scr[scr_i].state_inf[s].state_i;

			if (f[s][t] != M_INFINITY && back[s][t] != M_INFINITY)	//�α� �����ο��� ����Ѵ�.
				s_oc_tio = f[s][t] + back[s][t] - p;
			else
				s_oc_tio = M_INFINITY;

			s_oc[pi][si][0] = logsum(s_oc_tio, s_oc[pi][si][0]);




			if (t == 1) {	//transition ����
				a1[0][s + 1] = logsum(s_oc[pi][si][0], a1[0][s + 1]);
			}

			if (t != input_length - 1) {	//transition ����
				for (int s1 = 0; s1 < scr[scr_i].state_num; s1++) {

					if ((a[s + 1][s1 + 1] != 0) && (f[s][t] != M_INFINITY) && (back[s1][t + 1] != M_INFINITY))
						a_oc_tij = f[s][t] + logl(a[s + 1][s1 + 1]) + b[s1][t + 1][0] + back[s1][t + 1] - p;
					else
						a_oc_tij = M_INFINITY;

					a1[s + 1][s1 + 1] = logsum(a_oc_tij, a1[s + 1][s1 + 1]);

				}

			}

			for (int g = 0; g < g_num; g++) {	//���,�л�,state occupancy ����
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



	for (int i = 0; i < scr[scr_i].state_num; ) {	//transition�� ������ ����� new_phones�� �������ش�.
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


	for (int i = 0; i < scr[scr_i].state_num; i++) {	//��հ� �л��� ������ �ᰡ�� new_phones�� �������ش�.
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

		if (i == 17) {	//sp�϶��� update.
			new_phones[i].tp[1][1] = new_phones[i].tp[1][1] / expl(s_oc[i][0][0]);
			new_phones[i].tp[1][2] = 1 - new_phones[i].tp[1][1];
			for (int k = 0; k < g_num;k++) {
				new_phones[i].state[0].pdf[k].weight = expl(s_oc[i][0][k + 1]) / expl(s_oc[i][0][0]);	//weight update

				for (int d = 0; d < N_DIMENSION; d++) {

					new_phones[i].state[0].pdf[k].mean[d] = new_phones[i].state[0].pdf[k].mean[d] / expl(s_oc[i][0][k + 1]);	//��� update

					new_phones[i].state[0].pdf[k].var[d] = (new_phones[i].state[0].pdf[k].var[d] / expl(s_oc[i][0][k + 1])) - powl(new_phones[i].state[0].pdf[k].mean[d], 2);//�л� update


				}
			}
		}

		else {	//�������� ���� update

			for (int j = 0; j < 3; j++) {	//transition update

				for (int k = 0; k < 3; k++) {
					new_phones[i].tp[j + 1][k + 1] = new_phones[i].tp[j + 1][k + 1] / expl(s_oc[i][j][0]);
				}
				new_phones[i].tp[0][1] = 1;
				new_phones[i].tp[3][4] = 1 - (new_phones[i].tp[3][1] + new_phones[i].tp[3][2] + new_phones[i].tp[3][3]);

				for (int k = 0; k < g_num; k++) {
					new_phones[i].state[j].pdf[k].weight = expl(s_oc[i][j][k + 1]) / expl(s_oc[i][j][0]);//weight update

					for (int d = 0; d < N_DIMENSION; d++) {	//��հ� �л� update

						new_phones[i].state[j].pdf[k].mean[d] = new_phones[i].state[j].pdf[k].mean[d] / expl(s_oc[i][j][k + 1]);
						x = new_phones[i].state[j].pdf[k].var[d];

						new_phones[i].state[j].pdf[k].var[d] = (new_phones[i].state[j].pdf[k].var[d] / expl(s_oc[i][j][k + 1])) - powl(new_phones[i].state[j].pdf[k].mean[d], 2);

					}

				}


			}
		}
	}
	
}
void file_update(hmmType1 *phones_up, int g_num) {	//hmm.txt�� model�� update�����ش�.
	FILE *fp = NULL;
	
	int result = remove("hmm.txt");

	if (result == 0)
		printf("����\n");
	else
		printf("����\n");


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
void gaussian_split(hmmType1 *phones_up, int g_num) {	//����þ��� split�Ѵ�.

	srand(time(NULL));

	for (int i = 0; i < 21; i++) {
		if (i == 17) {	//sp�� ��
			int max_index;	//weight�� �ִ��� pdf�� index�� �����Ѵ�.
			float max_weight = 0; //�ִ� weight�� �����Ѵ�.
			for (int k = 0; k < g_num; k++) {
				if (max_weight < phones_up[i].state[0].pdf[k].weight) {	//�ִ� weight ã��
					max_weight = phones_up[i].state[0].pdf[k].weight;
					max_index = k;
				}
			}
			phones_up[i].state[0].pdf[max_index].weight = max_weight / 2;	//weight �й�
			phones_up[i].state[0].pdf[g_num].weight = max_weight / 2;

			for (int l = 0; l < N_DIMENSION; l++) {//��տ� small random vector ���ϱ�
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
		else {	//sp�� �ƴ� ��
			for (int j = 0; j < 3; j++) {
				int max_index;
				float max_weight = 0;
				for (int k = 0; k < g_num; k++) {
					if (max_weight < phones_up[i].state[j].pdf[k].weight) {	//�ִ� weight ã��
						max_weight = phones_up[i].state[j].pdf[k].weight;
						max_index = k;
					}
				}
				phones_up[i].state[j].pdf[max_index].weight = max_weight / 2;	//weight �й�
				phones_up[i].state[j].pdf[g_num].weight = max_weight / 2;

				for (int l = 0; l < N_DIMENSION; l++) {//��տ� small random vector ���ϱ�
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
long double logsum(long double x, long double y) {	//x=logA,y=logB�̸� logsum(x,y)�� log(A+B)�� ��ȯ�Ѵ�
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
