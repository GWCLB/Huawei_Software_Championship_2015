#ifndef _texas_h_
#define _texas_h_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define fold 0
#define call 1
#define raise 2
#define raise2 3
#define check 4

int pork_max(int *deal,int n,int m);//Ϊ�˼��㹫����ֵ����
int suit_num(int *deal,int n,int m );//�ж�m-n+1���ƵĻ�ɫ
int pork_same_num(int *deal,int n,int m);//�ж�m-n+1���Ƶ�����
int judgemypork(int pork_num);

void deal_dealtmp(int *deal,int *dealtmp,int n);
void set_all_pork(int *deal,int pork_num,int *all_pork);
int judge_pork(int *deal,int pork_num);
double gailv(int *deal,int *dealtmp,int n);

#endif
