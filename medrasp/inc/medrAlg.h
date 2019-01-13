//#pragma once
#ifndef MEDR_SYS_H_INCLUDED
#define MEDR_SYS_H_INCLUDED

#include <math.h>
#include <vector>
#include "../inc/sys.h"
#include "../inc/medrDP.h"


st_DoptTabItem algAKI(int k, st_share_data *sd);
st_DoptTabItem algDOPT(st_share_data *sd, st_DoptTabItem *AA,double ak);
st_DoptTabItem algFPTAS(st_share_data *sd);
void algPayFPTAS(st_share_data *sd);
void algPayVCG(st_share_data *sd);
void copyBlist(st_TenantBid *blist1, st_TenantBid *blist2, int bNum);
void initAM(st_share_data *sd);
int isIDInOptimal(st_DoptTabItem *sy, int idx);

void setEmptyItem(st_DoptTabItem *sci);

double algGreedy(st_share_data *sd);

#endif
