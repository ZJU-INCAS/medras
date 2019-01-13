//Dataprocess 

#include "../inc/medrDP.h"

using namespace std;






/*
Function: generate tenants
bi£º¡¾200-2000$¡¿
ei£º¡¾0-10MW¡¿---10K KW
alpha: from 180 to 350 step 30      be the cost of BES usage per kWh
gama = 1.6
Demand: 100MW,120 - 200, per 20          
Created by cjh at Sep.2,2016
*/

void gendatatoB0list(st_share_data *sd, int n){
	int i,c=0;
	st_TenantBid b;
	
	for (i = 0; i < n; i++){
		b.size = getabrand(1, 10);///random(10);//0-10MW
		//while(b.size==0) b.size=getrand(0,10);
		b.cost =b.size * getabrand(67,133);//
		b.fptasPayment = 0;
		b.vcgPayment=0;
		b.selected = 0;
		b.rwc = 0;
		sd->B0List[i]=b;
		//printf("%d,%d,%d\n",i,b.size,b.cost);

		if (DEBUG){
			printf("%d->(%4d|%8d|%4d)\n", i + 1,
				sd->B0List[i].size,
				sd->B0List[i].cost,
				sd->B0List[i].fptasPayment);
		}

		sd->B1List[i] = sd->B0List[i];

		c += sd->B0List[i].cost;
	}
	sd->ItemTotal = n;
	sd->CostTotal = c;
}



//generate n tenants data to RData
int gendatabatch(st_Resdata *r, int did)
{
	int i,k,c,tc,bn,s,ts,maxcc,mincc,num,n;
	double ratio,eps,alpha,gama;
	FILE *fp;
	const char *ftest[20]={
		"test01","test02","test03",	"test04","test05",
		"test06","test07","test08",	"test09","test10",
		"test11","test12","test13",	"test14","test15",
		"test16","test17","test18",	"test19","test20"
     };
    char ppath[100]="./data/";
	strcat(ppath,ftest[did-1]);
	strcat(ppath,"/indata.txt");

	printf("----------------------------------\n");
	printf("Input alpha(150--250), gama(1.0-2.0), eps(0.1--0.9) 3 numbers split with one space:");
	scanf("%lf%lf%lf",&alpha,&gama,&eps);
	printf("Input number of rdata:");
	scanf("%d",&n);
	printf("Input number of tenant:");
	scanf("%d",&num);//租户的个数


	printf("Gendata file is: %s.\n",ppath);

	if ((fp = fopen(ppath, "a")) == NULL) {
		printf("Error to open file");
		exit(1);
	};
	
	//if (DEBUG) {
		printf("generate new tenant data: \n");
		printf("id  W  alpha   gama   eps  ten_num\n----------------------\nTenants list: \n\n");
	//}
	
	i = 0;
	while (i<n)//n组数据
	{
		r[i].oid = i;
		
		//gen alpha,gama,eps
		r[i].wage.GAMA = gama;
		r[i].wage.ALPHA = alpha;
		r[i].wage.EPS = eps;

		if(DEBUG){
			printf("%d %.0f %.0f %.2f %.2f %d \n",r[i].oid, r[i].wage.W_EDR, 
				r[i].wage.ALPHA, r[i].wage.GAMA, r[i].wage.EPS, r[i].bNum);
		}
	
		bn = num; //tenant number
		r[i].bNum = bn;
		//blist, bn tenants data

		tc = 0;ts = 0;
        mincc=999999;maxcc=-10000;
		
		for (k = 0; k < bn; k++)
		{
			//1-10,11-20,21-30,31-40,41-50,51-60,61-70,71-80,81-90,91-100
			s = getabrand(1,15); 
			r[i].blist[k].size=s;
			c = s * getabrand(67,133);
			r[i].blist[k].cost=c;

			//fscanf(fp, "%d%d",&r[i].blist[k].size,&r[i].blist[k].cost);
			
			if (DEBUG){
				printf("%d %d | ", r[i].blist[k].size,r[i].blist[k].cost);
        		if((k+1) % 10==0) printf("\n");
			}
			
			tc = tc + c;
			ts = ts + s;
            
           		 if(mincc>c) mincc=c;
            		if(maxcc<c) maxcc=c;
		}

		
		r[i].totalcost = tc;
		r[i].totalsize = ts;
       	        r[i].maxcost = maxcc;
	        r[i].mincost = mincc;

        //gen  w
		ratio = 0.5; //getabrand(4,12)/10.0;
		r[i].wage.W_EDR = ts * ratio; //取size总和的0.2~1.6

		r[i].soc_all = 0;
		r[i].soc_operator = 0;
		r[i].soc_tenant = 0;
		r[i].yy_besonly = 0;
		r[i].yy_dopt = 0;
		r[i].yy_fptas = 0;
	
		Output1rd2Indatafile(fp,&r[i]);//output to a file
		
		if(DEBUG) printf("\n");
		i++;
	}

	
	fclose(fp);
	return (i);
}



//generate n tenants data to RData
int genDatabatch_for_Maxcost(st_Resdata *r,st_share_data *sd)
{
	int i,k,c,st,tc,bn,s,ts,maxcc,mincc,num,n;
	double ratio,eps,alpha,gama;
	FILE *fp;
	
	printf("----------------------------------\n");
	printf("Input alpha(150--250), gama(1.0-2.0), eps(0.1--0.9) 3 numbers split with one space:");
	scanf("%lf%lf%lf",&alpha,&gama,&eps);
	printf("Input number of rdata:");
	scanf("%d",&n);
	printf("Input number of tenant:");
	scanf("%d",&num);//租户的个数
	//printf("Input size range of tenant:");
	//scanf("%d");
	printf("Gendata file is: %s.\n",sd->url_outputfile);

	if ((fp = fopen(sd->url_outputfile, "a")) == NULL) {
		printf("Error to open file");
		exit(1);
	};
	
	//if (DEBUG) {
	printf("generate new tenant data: \n");
	printf("id  W  alpha   gama   eps  ten_num\n----------------------\nTenants list: \n\n");
	//}
	
	i = 0;
	while (i<n)//n组数据
	{
		r[i].oid = i;
		
		//gen alpha,gama,eps
		r[i].wage.GAMA = gama;
		r[i].wage.ALPHA = alpha;
		r[i].wage.EPS = eps;

		if(DEBUG){
			printf("%d %.0f %.0f %.2f %.2f %d \n",r[i].oid, r[i].wage.W_EDR, 
				r[i].wage.ALPHA, r[i].wage.GAMA, r[i].wage.EPS, r[i].bNum);
		}
	
		bn = num; //tenant number
		r[i].bNum = bn;
		//blist, bn tenants data

		tc = 0;ts = 0;
        mincc=999999;maxcc=-10000;
		
		st=(i%10 + 1) *10;
		
		for (k = 0; k < bn; k++)
		{
			//1-10,11-20,21-30,31-40,41-50,51-60,61-70,71-80,81-90,91-100
			s = getabrand(1,st); 
			r[i].blist[k].size=s;
			c = s * getabrand(67,133);
			r[i].blist[k].cost=c;

			//fscanf(fp, "%d%d",&r[i].blist[k].size,&r[i].blist[k].cost);
			
			if (DEBUG){
				printf("%d %d | ", r[i].blist[k].size,r[i].blist[k].cost);
        		if((k+1) % 10==0) printf("\n");
			}
			
			tc = tc + c;
			ts = ts + s;
            
           		 if(mincc>c) mincc=c;
            		if(maxcc<c) maxcc=c;
		}

		
		r[i].totalcost = tc;
		r[i].totalsize = ts;
       	        r[i].maxcost = maxcc;
	        r[i].mincost = mincc;

        //gen  w
		ratio = 0.5; //getabrand(4,12)/10.0;
		r[i].wage.W_EDR = ts * ratio; //取size总和的0.2~1.6

		r[i].soc_all = 0;
		r[i].soc_operator = 0;
		r[i].soc_tenant = 0;
		r[i].yy_besonly = 0;
		r[i].yy_dopt = 0;
		r[i].yy_fptas = 0;
	
		Output1rd2Indatafile(fp,&r[i]);//output to a file
		
		if(DEBUG) printf("\n");
		i++;
	}

	
	fclose(fp);
	return (i);
}



int getabrand(int a, int b){
	int x;
	x = a+rand()%100/100.0*(b-a);
	return(x);
}

///////////////////////////////////////////////////////////////
/*return the sum of all size of items*/

int inputData(st_share_data *sd)
{
	int i,k,c,tc,bn,s,ts,maxcc,mincc;
	st_Resdata *r;

	FILE *fp;
	
    
	if ((fp = fopen(sd->url_inputfile, "r")) == NULL) {
		printf("Error to open file");
		exit(1);
	};
	
	if (DEBUG) {
		printf("Load tenant data: \n");
		printf("id  W  alpha   gama   eps  ten_num\n----------------------\nTenants list: \n\n");
	}

	r=sd->RData;

	i = 0;
	while (!feof(fp))
	{
		fscanf(fp, "%d",&r[i].oid);
		//W,alpha,gama,eps^M
        fscanf(fp, "%lf%lf%lf%lf%d",
        	&r[i].wage.W_EDR, &r[i].wage.ALPHA,
        	&r[i].wage.GAMA, &r[i].wage.EPS,&r[i].bNum);
		
		if(DEBUG){
			printf("%d %d %.0f %.2f %.2f %d \n",i, (int)r[i].wage.W_EDR, 
				r[i].wage.ALPHA, r[i].wage.GAMA, r[i].wage.EPS, r[i].bNum);
		}

		bn = r[i].bNum;
		//blist, bn tenants data
		tc = 0;ts = 0;
        mincc=999999;maxcc=-10000;
		
		for (k = 0; k < bn; k++)
		{
			r[i].blist[k].bID=k; //set ID for each Bid
			fscanf(fp, "%d%d",&r[i].blist[k].size,&r[i].blist[k].cost);
			
			if (DEBUG){
				printf("%d %d | ", r[i].blist[k].size,r[i].blist[k].cost);
        		if((k+1) % 8==0) printf("\n");
			}
			
			r[i].blist[k].ownCost=r[i].blist[k].cost;//oldcost bakup

			c = r[i].blist[k].cost;
            s = r[i].blist[k].size;
			tc = tc + c;//CONCLUDE TOTAL COST
			ts = ts + s;//CONCLUDE TOTAL SIZE
            
            if(mincc>c) mincc=c;//CONCLUDE THE MININUM COST
            if(maxcc<c) maxcc=c;//CONCLUDE THE MAXIMUM COST
		}
		

		r[i].totalcost = tc;
		r[i].totalsize = ts;
		
        r[i].maxcost = maxcc;
        r[i].mincost = mincc;

		r[i].soc_all = 0;
		r[i].soc_operator = 0;
		r[i].soc_tenant = 0;
		r[i].yy_besonly = 0;
		r[i].yy_dopt = 0;
		r[i].yy_fptas = 0;
		//r[i].yy_optlb = 0;

		if(DEBUG) printf("\n");
		i++;
	}


	if (DEBUG){
		printf("----------------------\n");
		printf("Sucessfully:(Total Items : NN(%d). Total cost : WC(%d))\n", i, c);
		printf("----------------------\n");
	}

	fclose(fp);

	sd->NumOfAuc=i;

	return (i);
}



void initBListByRData(st_share_data *sd){
	int n,n_auc,i;	
	st_Resdata *rd;

	n_auc=sd->curAucId;
	rd=&sd->RData[n_auc];
	
	n=rd->bNum;
	for(i=0;i<n;i++)
		sd->B0List[i]=rd->blist[i];

	sd->ItemTotal=n;
	sd->CostTotal=rd->totalcost;
	sd->wage=rd->wage;
}



/*
fprintf(fp,"oid w alpha gama eps yydopt yyfptas fptas/dopt ");
    fprintf(fp,"1+eps timedopt timefptas timepayfptas timevcgpay yy_bes soc_all soc_operator soc_tenant bnum ");
	for (ii = 0; ii <  num; ii++){
		fprintf(fp,"size%d bid%d fptaspay%d fptasutility%d vcgpay%d vcgutility%d", 
			ii,ii,ii,ii,ii,ii);
	}
*/


void print1rd2file(FILE *fp,st_Resdata *rd)
{
	int i,k;
	double sUtiVcg,sUtiFptas;
   
	fprintf(fp, "%d ", rd->oid);
	fprintf(fp, "%d %.1f %.2f %.2f ",(int)rd->wage.W_EDR, 
		rd->wage.ALPHA, rd->wage.GAMA, rd->wage.EPS);
	
	fprintf(fp, "%.3f ", rd->yy_dopt);
	fprintf(fp, "%.3f %.5f %.2f ", rd->yy_fptas, 
		rd->approxratios,1+rd->wage.EPS);
	//timeopt, timefptas,timepayfptas,timevcgpay
	fprintf(fp, "%.8f ",rd->timeopt);
	fprintf(fp, "%.8f ", rd->timefptas);
	fprintf(fp, "%.8f ", rd->timepayfptas);
	fprintf(fp, "%.8f ", rd->timevcgpay);
	fprintf(fp, "%.2f ", rd->yy_besonly);
	fprintf(fp, "%.2f ", rd->soc_all);
	fprintf(fp, "%.2f ", rd->soc_operator);
	fprintf(fp, "%.2f ", rd->soc_tenant);
	
	fprintf(fp, "%d ", rd->bNum);

	sUtiFptas=0;
	for (i = 0; i < rd->bNum; i++){
		fprintf(fp,"%d %d %d %d %d %d ", 
			rd->blist[i].size, rd->blist[i].cost, rd->blist[i].fptasPayment,
			rd->blist[i].fptasUtility, rd->blist[i].vcgPayment, rd->blist[i].vcgUtility);
		//Cal. total utility of fptas
		if(rd->blist[i].fptasUtility>0)
			sUtiFptas+=rd->blist[i].fptasUtility;
		//cal. total utility of vcg
		if(rd->blist[i].vcgUtility>0)
			sUtiVcg+=rd->blist[i].vcgUtility;
	}
	
	fprintf(fp, "%.2f ", rd->totalcost);
	fprintf(fp, "%.2f ", rd->totalsize);
	
	fprintf(fp, "%.2f ", sUtiFptas);
	fprintf(fp, "%.2f ", sUtiVcg);
	
	fprintf(fp, "%d ", rd->fptas_sc.winBidIDs.size());
	fprintf(fp, "%d ", rd->dopt_sc.winBidIDs.size());
	
	//build fptas payitem info.
	for (i = 0; i < rd->bNum; i++){
		if (rd->blist[i].costPays.size() > 0){
			for (k = 0; k < rd->blist[i].costPays.size(); k++){
				if (k == 0)
					fprintf(fp, "T%d-pay(%d",i,rd->blist[i].costPays[k]);
				else
					fprintf(fp, ",%d", rd->blist[i].costPays[k]);
			}

			if (rd->blist[i].paycount > 0) fprintf(fp, ");");
		}
	}

	fprintf(fp, " ");
	//build vcg payitem info.
	sUtiVcg=0;
	for (i = 0; i < rd->bNum; i++){
		if (rd->blist[i].vcgPayment > 0){
			fprintf(fp, "T%d-pay(%d);",i,rd->blist[i].vcgPayment);
		}
	}
    
	fprintf(fp, "\n");
}



void printTitleInOutfile(FILE *fp, int num){
    int ii;
    fprintf(fp,"oid w alpha gama eps yydopt yyfptas yyfptas/yydopt ");
    fprintf(fp,"1+eps timedopt timefptas timepayfptas timevcgpay ");
    fprintf(fp, "yy_bes soc_all soc_operator soc_tenant bnum ");
	for (ii = 0; ii <  num; ii++){
		fprintf(fp,"size%d bid%d fptaspay%d fptasutility%d vcgpay%d vcgutility%d ", 
			ii,ii,ii,ii,ii,ii);
	}

	fprintf(fp, "totalcost totalsize tot_uti_fptas tot_uti_vcg  winc_fptas winc_vcg All_fptaspay_info All_vcgpay_info\n");
}



/*Name: print1rd2indata
Function: output a result data to a file.
*/
void Output1rd2Indatafile(FILE *fp,st_Resdata *rd){
	int i;
	fprintf(fp, "%d %.0f ", rd->oid,rd->wage.W_EDR);
	fprintf(fp, "%.1f %.2f %.2f %d ",rd->wage.ALPHA, rd->wage.GAMA, rd->wage.EPS,rd->bNum);

	for (i = 0; i < rd->bNum; i++){
		fprintf(fp,"%d %d ", rd->blist[i].size, rd->blist[i].cost);
	}
	
	fprintf(fp, "\n");
}



void printopty(st_share_data *sd, st_DoptTabItem *opty)
{

	int count,i,iidx;
	st_TenantBid b;
	count=opty->itemCount;
	for(i=0;i<count;i++){
		iidx=opty->winBidIDs[i];
		b= sd->RData[sd->curAucId].blist[iidx];
		printf("bid(%d): size %d,cost %d;",iidx, b.size,b.cost);
	}

}

