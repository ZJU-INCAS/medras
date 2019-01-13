//Dataprocess 
//#pragma once
#ifndef MEDR_DP_H_INCLUDED
#define MEDR_DP_H_INCLUDED

#include "../inc/sys.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>


int inputData(st_share_data *sd);
void initBListByRData(st_share_data *sd);
void initBListForAKI(st_share_data *sd,int k);
void print1rd2file(FILE *fp,st_Resdata *rd);
void printTitleInOutfile(FILE *fp, int num);
void printopty(st_share_data *sd, st_DoptTabItem *opty);

//generate n tenants data to B0List
void gendata(int n);
int gendatabatch(st_Resdata *r, int did);
int genDatabatch_for_Maxcost(st_Resdata *r,st_share_data *sd);
int getabrand(int a, int b);
void Output1rd2Indatafile(FILE *fp,st_Resdata *rd);

#endif




