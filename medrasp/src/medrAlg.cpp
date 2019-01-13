
#include "../inc/medrAlg.h"


using namespace std;



/*
||---------------------------------------------------------||
Algorithm 1: Algorithm DOPT(I): Dynamic Programming for EDR
//alpha: be the cost of BES usage per kWh
// gama: a ratio called Power Usage Eectiveness (PUE)  
// between the total energy consumption range from 1.1 to 2.
//=================================
//input b, OUTPUT A for DOPT
//input B0List in sd, build Tabular AA, and output the optimal 
// para: ak , if ak>0 do FPTAS else do DOPT（ak==-1)
*/

st_DoptTabItem algDOPT(st_share_data *sd, st_DoptTabItem *AA, double ak)
{
	int i, s0, jc, s, si, ci;

	int n,w,nc,bId,ocost;
	
	double yy=99999999,yic,y;
	double alpha,gama;
	st_Resdata *curRd;
	//st_share_data *sd = _dpathis->sd;
	st_TenantBid *B0;
	st_DoptTabItem aic, ai1c, rsc;
	st_Wage inpara;
	
	inpara=sd->wage;
	alpha=inpara.ALPHA;
	gama=inpara.GAMA;

	curRd=&sd->RData[sd->curAucId];
	B0=sd->B0List;//Get the source bids data

	/*
	if(ak<0){//ak>0 FPTAS has prepared B0List already.
		//Copy the source blist to B0List for DOPT
		copyBlist(B0,curRd->blist,curRd->bNum);
	}
	*/

	n=sd->ItemTotal; //Get the number tenant bids
	nc=sd->CostTotal; //get the total cost of all bids
	w=sd->wage.W_EDR; //get the edr target W


	setEmptyItem(&rsc);
	/*
	for (jc = 0; jc <= nc; jc++)
		setEmptyItem(&AA[jc]);
	*/
	
	i=0;
	while(i<n){

		si = B0[i].size; // b[i].size;
		ci = B0[i].cost; // b[i].cost

		bId= B0[i].bID;

		for (jc = nc; jc>=0; jc--){
			s0 = AA[jc].maxTotalSize;  //A(i-1,c)
			
			if(jc < ci)
				ai1c = AA[jc];
     		else 
			{
				s = si + AA[jc - ci].maxTotalSize;//s_i+1 + A(i,c-c_i+1)
				//Max{s,s0}
				if (s > s0) {
					ai1c = AA[jc - ci];
					ai1c.maxTotalSize = s;
					ai1c.akTotalcost=ai1c.akTotalcost+ci;//general cost
					ai1c.totcost=ai1c.totcost+ci; //general cost

					//Original cost, the cost before dividing by ak
					ai1c.totowncost += B0[i].ownCost;
					//Original bidID
					ai1c.winBidIDs.push_back(bId);
					ai1c.itemCount=ai1c.winBidIDs.size();;
				}
				else ai1c = AA[jc];
							
				ai1c.a_k = ak;
				ai1c.oid=i;
			}
			
			AA[jc] = ai1c;//Store to A.
			
		}//for jc

		i++;
	}

	rsc=AA[0];
	//if(DEBUG)  printf("nc:%d->",nc);
	for (jc = 0; jc <= nc; jc++){

		y=gama*AA[jc].maxTotalSize;

		if(y<w) {
			if(ak>0)
				AA[jc].y_ic=(int)(alpha*(w-y)/ak)+jc;
			else
				AA[jc].y_ic=(int)(alpha*(w-y))+jc;
	
			AA[jc].y_oic=(int)(alpha*(w-y))+AA[jc].totowncost;
		}	
		else
		{
			AA[jc].y_ic=jc;
			AA[jc].y_oic=AA[jc].totowncost;
		}


		if(yy>AA[jc].y_ic){
			yy=AA[jc].y_ic;
			//if(DEBUG) printf("yy:%.2f  ",yy);
			rsc=AA[jc];
		}

		//if(DEBUG) printf(" bidsize:%d,AA[jc].y_ic: %.2f",(int)rsc.winBidIDs.size(),rsc.y_ic);
	}


	return rsc;
}	


//Alg3: Monone_fptas
st_DoptTabItem algFPTAS(st_share_data *sd)
{
	int k, ny, maxCost, minCost;
	int log2maxcost, log2mincost;
	double yybest=VERYBIG,yy;
	
	st_DoptTabItem si, opts[MAX_LOG2_KK], best;
	
	sd->maxcost=sd->RData[sd->curAucId].maxcost;
	sd->mincost=sd->RData[sd->curAucId].mincost;

	maxCost =sd->maxcost; //getMaxCost(sd->B0List, sd->ItemTotal);
	minCost =sd->mincost; //getMinCost(sd->B0List, sd->ItemTotal);
	
	log2maxcost = (int)(log(maxCost*1.0)/log(2.0)+1);
    log2mincost = (int)(log(minCost*1.0)/log(2.0)+1);

	if (DEBUG) 
		printf("DoAKI----Logcost:(min:%d,Max:%d).\n", 
			log2mincost, log2maxcost);
 	
	
	ny=0;

	for(k=log2mincost;k<=log2maxcost;k++){

		if(DEBUG) printf("processing k:%d\n", k);

		si=algAKI(k,sd); //invoke algAKI to obtain optimal for each ak.

		if(si.y_ic>0)	opts[ny++]=si;//save the obtimal to opts-array. 
		
    }


    //Obtain the optimal by the minimal of opts.
    best=opts[0];
    for(k=0;k<ny;k++)
    {
    	if(yybest>opts[k].y_oic){
    		best=opts[k];
    		yybest=best.y_oic;
    	}
    }
    
	//best = si;
	if(DEBUG){
		printf("best:y_ic,y_oic: %.4f,%.4f\n",best.y_ic,best.y_oic);
	}

	return best;
}



/*Algorithm 2: Algorithm AKI(k,I): Dynamic Programming for EDR*/
//alpha: be the cost of BES usage per kWh
// gama: a ratio called Power Usage Eectiveness (PUE)  
// between the total energy consumption range from 1.1 to 2.
//=================================
//input b, OUTPUT A for DOPT
//input B0List in sd, build Tabular AA, and output the optimal 

st_DoptTabItem algAKI(int k, st_share_data *sd)
{
	st_TenantBid bb;

	int n,n_auc,i,nn,newcost;	
	st_Resdata *curRd;
	st_TenantBid *B0;
	st_DoptTabItem aic,rsc;

	double ak,totcost=0.0;

	curRd=&sd->RData[sd->curAucId];

	n=curRd->bNum;
	
	B0=sd->B0List;

	ak=curRd->wage.EPS * pow(2.0,k)/(n+1);

	if(DEBUG){
		printf("====ak:%0.4f\n",ak );
	}

	//Get new bid set for auction to B0List
	nn=0;
	for(i=0;i<n;i++)
	{
		bb=curRd->blist[i];

		if(bb.cost<=(int)pow(2.0,k)){			
			//bb.ownCost=bb.cost;
			B0[nn]=bb;
			B0[nn].cost=(int)(bb.cost/ak);
			totcost+=B0[nn].cost; //get new totalcost with new cost

			if(DEBUG)
				printf("bid,cost(owncost),size:%d,%d(%d),%d\n",
				B0[nn].bID,B0[nn].cost,B0[nn].ownCost,B0[nn].size);	

			nn++;
		}
	}

	sd->ItemTotal=nn;
	sd->CostTotal=totcost;
	

	//invoke algDOPT to obatin the optimal
	if(nn>0){
		initAM(sd);
		/*for(i=0;i<totcost;i++){
			setEmptyItem(&sd->AM[i]);
		}*/
		//Invoke algDOPT obtain optimal
		rsc=algDOPT(sd,sd->AM,ak);

	}else
	{
		rsc.y_ic=-1;
	}

	if(DEBUG){
		printf("algDOPT result: y_ic*ak,y_oic:%.4f,%.4f\n",rsc.y_ic*ak,rsc.y_oic);
	}

	return rsc;
}	





////===alg5:algpay=====================================================

////========================================================
//For algrithm set Payment///////////////////////////////////////////////////////////////////
////========================================================
//Algorithm IV: Calucate payment
//b0list:输入的原始数据集合，b1list为通过eps处理后的数据，SI已知最优解
void algPayFPTAS(st_share_data *sd)
{
	int i, kk, idx, h, l, ei, bi, bi1, zi = 1;
	st_DoptTabItem optYY, optY;
	st_Resdata *curRd;
	st_TenantBid *b,*B1;

	int intWinBids; //The count number of winner bid tenant.
	double alpha, gama, eps;
	st_Wage inpa;

	inpa = sd->wage;
	alpha = inpa.ALPHA;
	gama = inpa.GAMA;
	eps = inpa.EPS;

	//=====1. Do FPTAS onetime to get the optimal in optYY
	optYY=algFPTAS(sd);

	//
	if(DEBUG){
		printopty(sd, &optYY);
		printf("algPay begins, alpha %.2f, gama %.2f, doptYY: y_oic:%.3f,itemcount %d\n", 
		alpha,gama, optYY.y_oic, optYY.itemCount);
	}

	//Get current rd
	curRd=&sd->RData[sd->curAucId];

	//=====2. copy source RData[curAucid] to sd->B1List[n]
	copyBlist(sd->B1List,curRd->blist,curRd->bNum);
	
	// For each winner bid, iteratively use the binary method 
	// to solve the maximum payment
	intWinBids=optYY.itemCount; //Get the count number of winner bid tenant.

	if(DEBUG) {
		printf("Begins proc. payment for each bid. Total itemcount: %d.\n",
			intWinBids);
	}

	for (i = 0; i < intWinBids; i++){//iteratively
        

		idx = optYY.winBidIDs[i];//bidIDs[i];

		if(DEBUG){
			printf("\nBegin proc.pay #%d (Total:%d) bidID:%d", 
		    	i, intWinBids, idx);
		}


		zi = 1;
	
		B1=&sd->B1List[idx];   // For backup source bid info.
		b=&curRd->blist[idx];  // Source bids used in current set pay process

		ei = b->size;
		bi = b->cost;

		h = (int)(alpha*gama*ei);//The maximun cost can pay for winner
		l = bi; //The minimal cost at lease to the cost of winner.

		if(DEBUG) printf("si:%d,ci:%d, pay: %d to...[", ei,bi,bi);
		
		b->costPays.clear();

        kk = 0;
		while (h - l >=1){
			bi1 =(int)((h + l) / 2);//set mid h+l to new cost

			/*if (h - l == 1){
			
				if (zi == 1){
					l++; bi1 = l;
				}
				else{
					h--; bi1 = h;
				}
			}
			*/
			b->cost=bi1;//alter the cost of bid to a new bi1

			optY = algFPTAS(sd);//Call algFPTAS to obtain new optimal bids
			
			//printopty(sd, &optY);

			//Is idx is still in new otpimal bids?
			zi = isIDInOptimal(&optY, idx);

			if (zi == 1){//yes

				//set new l(bi1) as pay cost for curbid
				b->costPays.push_back(bi1);
				kk=b->costPays.size();
				b->paycount=kk;
				
				if(h-l==1) break;

				l = bi1; //set l as new bi1
				if(DEBUG) printf(" %d, ",l);

			}
			else{
				h = bi1; //set h as new bi1
			}
			

			//if (zi == 0 && h == l){ l--; break; }

		}

		
		B1->fptasPayment = l;//记录结果
		b->fptasPayment=l;

		B1->cost = b->cost;// sd->B0List[idx].cost;//记录结果
		B1->size = b->size;// sd->B0List[idx].size;//记录结果
		
		b->cost=bi;//restore source bid cost info. b by bi
		b->size=ei;//restore sourec bid size info b by ei
		
		if(DEBUG) {
			printf("->%d \n",l);
			printf("] The %dth payitem is over!\n", i);
		}
	}

}


/*/===============================================
//Function: void algPayVCG(st_share_data *sd)
//Desc: 1)Invoke DOPT to obtain the optimal result winner tenant agents I and y_all;
		2)For each agent i in I, invoke DOPT with all agents expect agent i 
		to obtain y_agent_i;
		3)The payment of agent i is concluded by the formula: 
			pay(i)=y_agent_i-y_all + c_i;
//Parameters: 
		sd: global share_data
//Created by Jianhai Chen at Dec-27, 2018.
//Altered ...
*/
void algPayVCG(st_share_data *sd){

	st_DoptTabItem optItm, optItm_i;
	double y_agent_all,y_agent_i;
	int curAucId,i,agent_i,agent_i_cost;
	st_Resdata *curRd;


	curAucId=sd->curAucId;
	curRd=&sd->RData[curAucId];
	printf("curAucId:%d\n", curAucId);
	//Invoke DOPT obtain the optimal

	//1.Reload blist from global original data
	//copyBlist(sd->B0List,curRd->blist,curRd->bNum);
	initBListByRData(sd);

	initAM(sd);
	
	//2.Invoke algDopt to obtain optimal winner agents.
	optItm=algDOPT(sd, sd->AM,0);

	y_agent_all=optItm.y_oic;
	curRd->dopt_vcg=optItm;//Record the optimal of DOPT


	printf("y_agent_all: %.2f winbidsize: %d \n",optItm.y_oic, (int)optItm.winBidIDs.size());
	//for each agent_i, conclude its payment by y_agent_i-y_all+ agent_i.cost
	for(i=0;i<(int)optItm.winBidIDs.size();i++){
		agent_i=optItm.winBidIDs[i];
		//printf("agent_id: %d\n",agent_i);
		//reload original blist and update B0List
		copyBlist(sd->B0List,curRd->blist,curRd->bNum);
		
		//remove agent_i: 
		//1)change current agent_i to the last agent;
		sd->B0List[agent_i]=sd->B0List[curRd->bNum-1];
		curRd->bNum--;//remove the last agent;
		sd->ItemTotal--;
		agent_i_cost=sd->B0List[agent_i].cost;
		sd->CostTotal-=agent_i_cost;

		initAM(sd);
		//reinvoke algDOPT to obtain new optimal between agents without agent_i
		optItm_i=algDOPT(sd, sd->AM,-1);
		y_agent_i=optItm_i.y_oic;

		//printf("y_agent_i:%.2f\n", y_agent_i);
		//Update the pay of agent_i to be y_agent_i-y_all+ agent_i.cost
		curRd->blist[agent_i].vcgPayment=y_agent_i-y_agent_all+agent_i_cost;
		curRd->blist[agent_i].vcgUtility=curRd->blist[agent_i].vcgPayment-agent_i_cost;

		//restore globals
		curRd->bNum++;
		sd->ItemTotal++;
		sd->CostTotal+=agent_i_cost;
	}

}


//近似最优解的下界算法
//根据ci/si从小到大排序，依次从最小的开始选取租户，直到填满w
//输入B0，w，输出B2
double algGreedy(st_share_data *sd)
{
	int n = sd->ItemTotal;
	int curAucid;
	struct bbcc{
		int oid;
		double bc;
	}bo[N],tt;
	double ss=0,aa,wc,rc,yc,ys,yy,alpha,gama;
	double w;
	st_TenantBid sb;

	int i,j;

	for (i = 0; i < n; i++){
		bo[i].bc = 1.0*sd->B0List[i].cost / sd->B0List[i].size;
		bo[i].oid = i;
	}

	//sort bc
	for (i = 0; i < n - 1; i++)
	{
		for (j = i + 1; j < n;j++)
			if (bo[j].bc < bo[i].bc){
				tt = bo[i];
				bo[i] = bo[j];
				bo[j] = tt;
			}
	}

	//Bin packing
	//bin capacity is w/gama
	alpha = sd->wage.ALPHA;
	gama = sd->wage.GAMA;

	w=sd->wage.W_EDR;
	
	wc = 1.0*w / gama;
	rc = wc;
	i = 0;
	yc = 0;
	ys = 0;
	while (i < n){

		if(bo[i].bc>alpha) break;
		
		sb = sd->B0List[bo[i].oid];
		ss = sb.size;
        if(DEBUG)  printf("%d,%.2f,%d,wc:%.2f\n",i,ss,sb.cost,wc);

		if (ss <= wc){
			wc = wc - ss;//selected
			sd->B0List[bo[i].oid].selected = 1;//set selected to 1
			sd->B0List[bo[i].oid].rwc = 0;//set rwc to 0
			yc = yc + sb.cost;
			ys = ys + sb.size;
		}
		else{//租户填满
			aa = wc;//The last item selected part.
			wc = 0;
			sd->B0List[bo[i].oid].selected = 1;
			sd->B0List[bo[i].oid].rwc = aa;
			ys = ys + aa;
			yc = yc + aa*sb.cost / sb.size;
			break;
		}
        if(DEBUG) printf("yc:%.2f,ys:%.2f\n",yc,ys);	
		i++;
	}

	//Obtain the sum of cost for final
	
	if(w-gama*ys>0)
		yy = alpha * (w - gama * ys) + yc;
    else
        yy = yc;

	
	if (DEBUG) {
		printf("Sel-User size/cost: %.2f/%0.2f\n", ys, yc);
		printf("BES size:%.2f. Total social cost:%.2f\n", w - gama * ys,yy);
	}

	return yy;
}
 


//===============================================
//Fun.Name: void copyBlist(st_TenantBid *blist1, st_TenantBid *blist2)
//Fun.Desc: copy blist2 to blist1, v1.0
//Parameters: 
//			blist1,  the first blist
// 			blist2,  the second blist
//Created by Jianhai Chen at Dec-25, 2018.
//Altered ...
//-----------------------------------------------
void copyBlist(st_TenantBid *blist1, st_TenantBid *blist2, int bNum){
	int i;
	//=====2. copy source RData[curAucid] to sd->B1List[n]
	int ts,tc;
	ts=0;tc=0;
	for(i=0;i<bNum;i++){
		blist1[i]=blist2[i];
		ts+=blist2[i].size;
		tc+=blist2[i].cost;
	}

}



//--------------------------------------------
//------判断一个给定的itemID是否在最有解里面------
//

int isIDInOptimal(st_DoptTabItem *sy, int idx){
	int i;
	if(DEBUG) printf("IsIDInOptimal  sy: itemcount %lu, idx: %d \n",
		sy->winBidIDs.size(),idx);


	//printopty(&g_sd,sy);

	for (i = 0; i < sy->winBidIDs.size(); i++)
	{
		if (sy->winBidIDs[i] == idx) return 1;
	}
	return 0;
}



void initAM(st_share_data *sd){
	int i;
	//printf("CostTotal:%ld\n",sd->CostTotal);
	for(i=0;i<sd->CostTotal;i++){
		setEmptyItem(&sd->AM[i]);	
	}

}

void setEmptyItem(st_DoptTabItem *sci)
{
	sci->oid=0;
	sci->maxTotalSize=0;
	sci->totcost=0;
	sci->totowncost=0;
	sci->itemCount=0;
	sci->akTotalcost=0;
	sci->y_ic=0;
	sci->y_oic=0;
	sci->winBidIDs.clear();
	sci->a_k=0;
}


