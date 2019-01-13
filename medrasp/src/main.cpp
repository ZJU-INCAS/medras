/*********************************************************************
The mechanism for smart grid or colocation datacenters

输入B={bi|(si,ci),i=1..n},w, 在B中选择一个子集合B', 最小化社会开销：
min z=alpha * y + sum(ci)，满足：
y + gama * sum(si) >= W,         （1）
B' <- B,                          (2) 
y = W - gama * sum(si)

基本过程：
1.读入:
租户数据，W，及参数alpha，gama，eps；

2.计算
1）最优解的B'；
2）payment；
3）utility；
4）social cost: colocation operator, tenants;

3.Output：
1)Approximation Ratios
y(DOPT)/y(FPTAS)

2)Agents' No-Negative Utilities

3)Social Cost Reduction Compared to \BES only"
y(FPTAS)/y(BESonly)

4)Truthfulness

*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

#include <cmath>


//if linux then remove #if
//#if COMPILE_LINUX 
	#include <sys/time.h>
//#endif

#if COMPILE_MACOS
	#include <time.h>
#endif

#include "string.h"

#include "../inc/sys.h"
#include "../inc/medrDP.h"
#include "../inc/medrAlg.h"

//#include "inc/dataprocess.h"

using namespace std;


//======Global valuables==========
//{
st_share_data g_sd;	//global data

long tpstart, tpend;//for calculate runtime of a function
struct  timeval tvstart,tvend;
char timestr[100];
//}


//========Dataprocess: input output===========
//{ Begin dp

// } End dp.

//=====Input output=======

//======Functions declaration region
// {
/*st_DoptTabItem algAKI(int k, st_share_data *sd);
st_DoptTabItem algDOPT(st_share_data *sd, st_DoptTabItem *AA,double ak);
st_DoptTabItem algFPTAS(st_share_data *sd);
void algPay(st_share_data *sd);
*/

void CalcUtility(st_Resdata *rd);

void doAuction(st_share_data *sd);
void do_DOPT(st_share_data *sd);
void do_FPTAS(st_share_data *sd);
void do_fptasPay(st_share_data *sd);
void do_vcgPay(st_share_data *sd);

double getTime(long tpstart, long tpend);

void printLog(const char *log);

void printBlist(st_Resdata *curRd,int num);

double getcpuruntime();
void printCurrentTime();

//st_DoptTabItem DPA_AM(st_share_data *sd);//input B0£¬out 
//st_DoptTabItem DPA_AKM(st_share_data *sd);//input B0£¬out 

// }


//==============================================
int main(int argc, const char *argv[])
{
	 char infile[60];
	 char outfile[60];
	 char ss[10];
	 int x;
	
	int aucid,i, startid, endid;
	
	printf("Wellcome to MEDR tool V10. \n");
	printf("Please attention, your input command format as below:\n");
	printf("Command line format is like \"mab #1 #2 #3 #4\"\n");
	printf("   #1: inputfile_path.\n");
	printf("   #2: outfile_path.\n");
	printf("   #3 and #4: a region start and end number\n amongst all auction samples.\n");
	printf("=========================================================\n");

	
	switch(argc){
		case 2://generate data set
			printf("%s\n", argv[1]);

			strcpy(g_sd.url_outputfile,argv[1]);

			genDatabatch_for_Maxcost(g_sd.RData,&g_sd);

		case 5:
		//if(argc==5){

			strcpy(g_sd.url_inputfile,argv[1]);
			strcpy(g_sd.url_outputfile,argv[2]);
			strcpy(ss,argv[3]);

			//Calculate the serial integer number amongst all auction samples
			aucid=0;
			for(i=0;ss[i]!='\0';i++){
				aucid=aucid*10+ss[i]-'0';
			}
			startid=aucid;

			strcpy(ss,argv[4]);
			aucid=0;
			for(i=0;ss[i]!='\0';i++){
				aucid=aucid*10+ss[i]-'0';
			}
			
			endid=aucid;

			g_sd.curAucId=aucid;
			g_sd.start_aid=startid;
			g_sd.end_aid=endid;

			printf("The machine takes %.6f seconds to do 10^9 iterations.",getcpuruntime());
		

			printf("You have input startid, endid: %d --- %d \n",startid,endid);
			printf("Press 1 to continue.......\n");
			scanf("%d",&i);
			//Invoke auction function to do auction bidding decision
			doAuction(&g_sd);

			printf("Bidding decision sucessfully!\nThe program begins shutdown now！Waiting for a while!\n");
		//}		
			break;

		default:
			printf("========================================================\n");
			printf("Input error! The number of command line parameters must be 5.\n");
			printf("Please check your input command format as below:\n");
			printf("Command format is like \"mab #1 #2 #3 #4\"\n");
			printf("   #1: inputfile_path.\n");
			printf("   #2: outfile_path.\n");
			printf("   #3 and #4: a region start and end number\n amongst all auction samples.\n");
			printf("=========================================================\n");
			
			printCurrentTime();
			//scanf("%d",&x);
			//if(x==1)
			printf("The machine takes %.6f seconds to do 10^9 iterations.",getcpuruntime());
		//else
			exit(0);

	}
	
}

/*
int main(){

    char ss[10];
    int aucid,i;
    FILE *fpo;
    const char *argv[4]={"./mab","./data/test01/indata.txt","./data/test01/out.txt","0"};
    strcpy(g_sd.url_inputfile,argv[1]);
	strcpy(g_sd.url_outputfile,argv[2]);
	strcpy(ss,argv[3]);

	if((fpo=fopen(g_sd.url_outputfile,"a"))==NULL){
		printf("FILE open error!\n");
		exit(0);
	}



	//Calculate the serial integer number amongst all auction samples
	aucid=0;
	for(i=0;ss[i]!='\0';i++){
		aucid=aucid*10+ss[i]-'0';
	}
	
    g_sd.curAucId=12;

	//Invoke auction function to do auction bidding decision
	doAuctionBidding(&g_sd,fpo);

	fclose(fpo);

	//system("pause");
	printLog("Please input 1 to continue exit this program!");
	scanf("%d",&i);
}
*/

//========Implementation of functions===================================
//{----

void doAuction(st_share_data *sd)
{
	FILE *fp;

	int i,xi,jj,k, intAid;
	double y = 0, payc, s = 0, yc = 0, ys = 0, cc = 0;
	double tt,yy_optlb,optlbtime;

	st_DoptTabItem sac;
	st_Resdata *curRd;

	
	//===1. Load input file data
	printf("====>Loading data from the input file: %s.\n",sd->url_inputfile);			
	tpstart=(long)clock();
	
	#if COMPILE_LINUX 
		gettimeofday(&tvstart,NULL);
		tpstart=tvstart.tv_sec*1000000+tvstart.tv_usec;
	#endif

	//Input data
	inputData(sd);
	tpend=(long)clock();

	#if COMPILE_LINUX 
		gettimeofday(&tvend,NULL);
		tpend=tvend.tv_sec*1000000+tvend.tv_usec;
	#endif
	

	printf("\nTotal time of loadInputData is:%.6fs\n", getTime(tpstart,tpend));



	printf("The machine takes %.6f seconds to do 10^9 iterations.",getcpuruntime());
	


	printf("Press any key such as 1 to continue:\n"); 
	scanf("%d",&xi);
	
	if(xi!=1) exit(0);

	//Open an output file for saving all result info.
	if((fp=fopen(g_sd.url_outputfile,"a"))==NULL){
		printf("FILE %s open error!\n",g_sd.url_outputfile);
		exit(0);
	}

	//batch process
	for(intAid=sd->start_aid;intAid<=sd->end_aid;intAid++)
	{
		printf("--------------------------------------------\n\n");
		//==============
		sd->curAucId=intAid;

		curRd = &sd->RData[sd->curAucId];
		printf("====>Start auction of agent group No.#%d\n",intAid);

		printCurrentTime();

		if(curRd->bNum>9){
			printf("The number of bids is larger than 9.\n");
			printf("\n We only shows 9 ones......\n");
		}

		printf("--------------------------------------------\n\n");
		printf("The oid #%d, alpha: %.2f gama: %.2f, eps: %.3f. \nBids are as follows:\n",
			curRd->oid,curRd->wage.ALPHA,curRd->wage.GAMA,curRd->wage.EPS);
		//show blist 
		printBlist(curRd,9);
		
		printf("--------------------------------------------\n\n");
		//===3.invoke DOPT to obtain the optimal solution
		printf("====>Do_DOPT: group#%d-step 1: Invoke algDOPT to obtain optimal solution.\n",intAid);
		
		printCurrentTime();
		
		do_DOPT(&g_sd);

		printf("====Do_DOPT is over sucessfully!\n");
		printCurrentTime();
		
		printf("--------------------------------------------\n\n");


		//===4.do vcg_payment
		printf("====>Do_vcgPay: group#%d-step 2: Invoke do_vcgPay to set vcg_payment.\n",intAid);
		
		do_vcgPay(&g_sd);

		printf("====Do_vcgPay is over sucessfully!\n");
		printCurrentTime();
		
		printf("--------------------------------------------\n\n");


		//===5.Do FPTAS =====
		printf("====>Do_FPTAS: group#%d-step 3: Invoke Do_FPTAS to get optimal.\n",intAid);
		do_FPTAS(&g_sd);


		printf("====Do_FPTAS is over sucessfully!\n");
		printCurrentTime();
		
		printf("--------------------------------------------\n\n");		

		//=======================================
		//printf("Input 1 continue:\n");scanf("%d",&xi);

		//6.do fptas payment
		printf("====>do_fptasPay: group#%d-step 4: Invoke do_fptasPay to set fptasPayment.\n",intAid);
		printf("Payment will last a long time, please waiting.....\n");
		printf("Total %d items......\n",curRd->fptas_sc.itemCount);
	
		do_fptasPay(&g_sd);

		printf("====do_fptasPay is over sucessfully!\n");
		printCurrentTime();
		
		printf("--------------------------------------------\n\n");		
		
	    //7.===Output the result to output.text

	    printf("====>Writing: group#%d-step 5: Invoke print1rd2file to output.\n",intAid);
		printf("====>Writing to outfile......\n");
		if(intAid==sd->start_aid)
			printTitleInOutfile(fp,curRd->bNum);

		print1rd2file(fp, curRd);

		printf("--------------------------------------------\n\n");		

		printf("====>The auction of Group#%d is over......\n\n\n",intAid);
		printCurrentTime();
		
		printf("--------------------------------------------\n\n");		

	}// for Aid

	fclose(fp);
    //system("pause");
}


void do_DOPT(st_share_data *sd)
{
	double tt;
	//clock_t tpstart,tpend;
	st_DoptTabItem dti;
	st_Resdata *curRd;

	curRd=&sd->RData[sd->curAucId];
	tpstart=(long)clock();

	#if COMPILE_LINUX 
		gettimeofday(&tvstart,NULL);
		tpstart=tvstart.tv_sec*1000000+tvstart.tv_usec;
	#endif

	


	//Prepare data for algDOPT: initial B0List in sd by RData[aucid].blist[N]
	//=====2. copy source RData[curAucid] to sd->B1List[n]
	//copyBlist(sd->B0List,curRd->blist,curRd->bNum);

	initBListByRData(sd);
	
	initAM(sd);//printf("Do initBListByRData over!\n");
	//invoke algDOPT to do 
	dti=algDOPT(sd,sd->AM,0);


	sd->RData[sd->curAucId].dopt_sc=dti;
	sd->RData[sd->curAucId].yy_dopt=dti.y_oic;



	tpend=(long)clock();
	//store result
	sd->RData[sd->curAucId].yy_dopt=dti.y_oic;

	#if COMPILE_LINUX 
		gettimeofday(&tvend,NULL);
		tpend=tvend.tv_sec*1000000+tvend.tv_usec;
	#endif
	

	tt=getTime(tpstart,tpend);
	sd->RData[sd->curAucId].timeopt=tt;

	printf("\nTotal time of DOPT is:%.6fs\n", tt);
	printf("Results: y_oic, y_ic are: %.3f,%.3f.\n", dti.y_oic,dti.y_ic);

}


void do_FPTAS(st_share_data *sd)
{
	//clock_t tpstart,tpend;
	st_Resdata *curRd;
	st_DoptTabItem dti;

	curRd=&sd->RData[sd->curAucId];


	tpstart=(long)clock();

	#if COMPILE_LINUX 
		gettimeofday(&tvstart,NULL);
		tpstart=tvstart.tv_sec*1000000+tvstart.tv_usec;
	#endif
	//Prepare data for algFPTAS: 
	//initial B0List in sd by RData[aucid].blist[N]
	//initBListByRData(sd);
	//printLog("Do initBListByRData over!");
	//invoke algDOPT to do all
	dti=algFPTAS(sd);
	curRd->fptas_sc=dti;
	tpend=(long)clock();

	#if COMPILE_LINUX 
		gettimeofday(&tvend,NULL);
		tpend=tvend.tv_sec*1000000+tvend.tv_usec;
	#endif
	//Calc. total runtime
	curRd->timefptas=getTime(tpstart,tpend); 
	
	//store result
	//printf("Do algFPTAS is over sucessfully!\n");
	printf("Total runtime is:%.6f seconds.\n", curRd->timefptas);
	printf("Results: y_oic, y_ic are: %.3f, %.3f.\n", dti.y_oic, dti.y_ic);
	curRd->yy_fptas=dti.y_oic;
	curRd->yy_besonly=curRd->wage.ALPHA*curRd->wage.W_EDR;
	curRd->approxratios=curRd->yy_fptas/curRd->yy_dopt;

}


void do_fptasPay(st_share_data *sd)
{
	clock_t tpstart,tpend;
	double tt;
	st_Resdata *curRd;

	curRd=&sd->RData[sd->curAucId];

	//printf("\n\n\n-----------------------\n");
	tpstart=(long)clock();

	#if COMPILE_LINUX 
		gettimeofday(&tvstart,NULL);
		tpstart=tvstart.tv_sec*1000000+tvstart.tv_usec;
	#endif
	//Call AlgPay to do set payment for all bids.
	algPayFPTAS(sd);
	tpend=(long)clock();

	#if COMPILE_LINUX 
		gettimeofday(&tvend,NULL);
		tpend=tvend.tv_sec*1000000+tvend.tv_usec;
	#endif

	tt=getTime(tpstart,tpend);
	printf("The runtime of set payment is %.6f sec.\n\n\n", tt);
    curRd->timepayfptas=tt;
    
    CalcUtility(curRd);

    printf("Social cost of colocation operator: %.2f\n", curRd->soc_operator);
    printf("Social cost of tenant: %.2f\n", curRd->soc_tenant);
    printf("Social cost of total: %.2f\n", curRd->soc_all);
    
	//=====================================================
	printf("Fptas set Payment over!\n-----------------------\n\n\n");
}


void do_vcgPay(st_share_data *sd)
{
	clock_t tpstart,tpend;
	double tt;
	st_Resdata *curRd;

	curRd=&sd->RData[sd->curAucId];

	//printf("\n\n\n-----------------------\n");
	//printf("====>Group#%d-step 5: invoke alg_vcgPay to set vcgPayment of each agent...\n ",sd->curAucId);
	//printf("Payment will last a long time, please wait ...\n");
	//printf("Total %d items......\n",curRd->dopt_sc.itemCount);
	tpstart=clock();

	#if COMPILE_LINUX 
		gettimeofday(&tvstart,NULL);
		tpstart=tvstart.tv_sec*1000000+tvstart.tv_usec;
	#endif
	//Call AlgPay to do set payment for all bids.
	algPayVCG(sd);

	tpend=clock();

	#if COMPILE_LINUX 
		gettimeofday(&tvend,NULL);
		tpend=tvend.tv_sec*1000000+tvend.tv_usec;
	#endif
	tt=getTime(tpstart,tpend);
	printf("The runtime of set vcg payment is %.6f sec.\n\n\n", tt);
    curRd->timevcgpay=tt;
    
    //CalcUtility(curRd);

    //printf("Social cost of colocation operator: %.2f\n", curRd->soc_operator);
    //printf("Social cost of tenant: %.2f\n", curRd->soc_tenant);
    //printf("Social cost of total: %.2f\n", curRd->soc_all);
    
	//=====================================================
	printf("VCG Payment over!\n-----------------------\n\n\n");
	

}


void CalcUtility(st_Resdata *rd){
	int k,jj,payc;
	double y,s,yc,ys,cc,spay=0;
	st_TenantBid *bb;

	st_Resdata *curRd=rd;

	y = 0; s = 0; yc = 0; ys = 0; cc = 0;

	//Get sum(Payment)
	for (k = 0; k < curRd->bNum; k++)
	{
		bb=&curRd->blist[k];
		payc = bb->paycount;
		
		spay+=bb->fptasPayment;
		bb->fptasUtility=bb->fptasPayment-bb->cost;

		//Output all payments
		//printf("Tenant No #%d:(cost,size,payment):",k);
		//printf("(%d,%d,%d)\n", bb->cost,bb->size,bb->fptasPayment);
		//printf("Pays is(");
		//for (jj = 0; jj < payc; jj++)
		//	printf("%d ",bb->costPays[jj]);
		//printf(")\n");

		if (payc > 0){
			ys = ys + bb->fptasPayment; // the total of payment
			s = s + bb->size;  // the total size
			cc = cc + bb->cost; // the total of all cost
			yc = yc + (bb->cost	- bb->fptasPayment); //
		}

	}


	 // social cost of colocation operator
	curRd->soc_operator = curRd->wage.ALPHA 
			* (curRd->wage.W_EDR - curRd->wage.GAMA*s) + ys;

	// social cost of tenant
	curRd->soc_tenant = yc;//sum(ci-PiB)
	
	
	y=curRd->wage.W_EDR - curRd->wage.GAMA*s;
	if(y>0)
		curRd->soc_all = curRd->wage.ALPHA * y + cc;
	else
		curRd->soc_all=cc;

}


//==== getTime===============================
//return the number of second for runtime.
//===========================================
double getTime(long tpstart, long tpend)
{
	double akitime;
	akitime =(double)(tpend - tpstart)*1.0 / CLOCKS_PER_SEC;
	//printf("\nTotal time is:%.6fs\n", akitime);
	return akitime;
}



void printBlist(st_Resdata *curRd,int num){
	int i;
	printf(" No.#   (siz, cost) \n-----------------------------\n");
	for(i=0;i<num;i++){
		printf("No.%5d:（%8d,%10d）\n",i,
			curRd->blist[i].size,curRd->blist[i].cost);
	}
	printf("-----------------------------\n");
}


// Output information
void printLog(const char *log)
{
	if(DEBUG)
		printf("%s\n",log);
}


double getcpuruntime()
{
	long i;
	long x=0;
	tpstart=(long)clock();
	for(i=0;i<10000000000000000;i++)
		x++;
	printf("The total x: %ld",x);
	tpend=(long)clock();
	return(getTime(tpstart,tpend));
}

void printCurrentTime()
{
	time_t timep;
	time (&timep);
	//char tmp[64];
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S",localtime(&timep));

	printf("%s\n", timestr);

	//strcpy(timstr,tmp);
	//return timestr;
}

//----} end of function implementation



