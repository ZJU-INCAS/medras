//#pragma once
#ifndef DPRS_DATA_H_INCLUDED
#define DPRS_DATA_H_INCLUDED

#include <vector>


#define N 1000  //Number of Tenant Items
#define NC 100  //The amount of Cost
#define MAX_LOG2_KK 20 //2^KK maxinum log2(maxCost)
#define NN 1000 //模拟数据组数
#define NP 100
#define NCM 1000000  //for max sum(cost): sigma cost_i 
#define VERYBIG 99999999
#define DEBUG 0

#define COMPILE_LINUX 0
#define COMPILE_MAC 1

using namespace std;

//Bids for tenants
typedef struct{
	int size;//si: amount of planned energy reduction
	int cost;//bi: the claimed cost due to such a reduction
	int ownCost;  //ci: own cost of tenant i
	int fptasPayment;  //final obtained payment reward
	int vcgPayment;
	std::vector<int> costPays;
	int paycount;
	int fptasUtility; //utility for tenant i
	int vcgUtility;
	int selected; //for optimal Lbound
	int bID;  //bid id save the original tenant bid id
	double rwc;
} st_TenantBid;


typedef struct{
	int id;        //id of tenant
	char tenName[100];
	char tenTel[100];
	char tenAddr[200];
	st_TenantBid Bid;//int plannedER;//si: amount of planned energy reduction
} st_Tenant;


//s(i,c)cost is exactly c and whose total size is maximized
typedef struct{
	int oid;
	int maxTotalSize; //size is maximized, total size of all selected items
    int totcost;     //akTotalcost
    int akTotalcost; //The total cost concluded in FPTAS with ak
    int totowncost; //cost for maxsize
	int itemCount;  //1....i, itemcount
	
	double y_ic; //yic for DPA : the current social cost conluded without using ak
	double a_k;  // for FPTAS ak
    double y_oic; //total cost among all selected items including by a_k
	
	std::vector<int> winBidIDs; //all ids of subset with maxtotalsize

} st_DoptTabItem;


//Fixed parameters for bid system
typedef struct{
	double W_EDR;
	double ALPHA;
	double GAMA;
	double EPS;
}st_Wage;


typedef struct {
	int oid;
	int bNum;
	st_TenantBid blist[N];/*//租户的报价信息*/
	st_Wage wage;/*W_edr,alpha,gama and eps*/
	double yy_dopt, yy_fptas;
	double yy_besonly;/*/alpha*W;*/
	double approxratios;/*/yy_fptas/yy_dopt, yy_fptas/yy_optlb*/
	double soc_operator;/*/alpha*y+sum(Pi(B)),i=1..N*/
	double soc_all;/*/alpha*y+sum(ci),y=W-gama*sum(si);i=1..N*/
	double soc_tenant;/*/sum(ci-Pi(B)),i=1..N*/
	double totalcost,maxcost,mincost;
	double totalsize;
	double timeopt, timefptas,timepayfptas,timevcgpay;
	st_DoptTabItem dopt_sc;
	st_DoptTabItem fptas_sc;
	st_DoptTabItem dopt_vcg;
} st_Resdata;


/*/Gloabal share data*/
typedef struct{
	
	int ItemTotal; /*/Tenant items count*/
	long CostTotal; /*/Cost total of all items*/
	int LogCMax;
    long maxcost,mincost;
    double ak;
    //int W;

    char url_inputfile[100];// the url of input file
    char url_outputfile[100];// the url of output file

	st_TenantBid B0List[N];/*bids for tenants*/
	st_TenantBid B1List[N];/*for alg2*/
	
	
	st_DoptTabItem AM[NCM];/*For Algorithm II*/
	//st_SAIC AKM[NCM];
	/*/Save the data for AKI 保存AKI求解后的所有结果*/

	st_Tenant TenaList[N]; /*N tenants*/
	st_Wage wage; /*记录W(EDR),alpha，gama and eps的值*/
	

	st_Resdata RData[NN]; /*/Store NN groups of source data for batch processing */
	int NumOfAuc; /*/The number of auctions data*/
	int curAucId; /*/The current aucid for bid*/
	int start_aid,end_aid;
	
} st_share_data;

#endif