#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <queue>
#include <time.h>
#include <fstream>

#define ST 5//站点数
#define FN 10000 //每个站要发的帧数
#define SD 50//每次发包的时间槽数范围 
using namespace std;

double TS = 0.01; //时间槽单元 1us
double lanmada  = 1;//到达间隔分布参数
double mu = 5;//发送时间加上确认返回时间分布参数 
double P = 0.3;//p-坚持 

struct Frame{
	double arrive;//到达时间 
	double start;//开始发送时间
	int collsions;//碰撞次数 
}; 

struct Station{
	int timeslot;//时间槽数 
	int collisions;//队首帧碰撞次数
}; 
queue<int> q[ST];//等待队列 
Station station[ST]; //站点 
Frame frame[ST][FN];//帧 

int readyin[ST];//下一个将要加入队列的帧 
int coll[ST];//每个站发生碰撞的包数 
int ssend[ST];//每个站总的发包次数 

int wait_time_p[ST][FN];//统计时间概率分布 
int wait_line_p[ST][FN];//统计队列长度分布 

double totalength[ST];//类似积分队长总和 
//负指数分布 
inline double exprand(double lanmada)
{
	int rnd;
	double r,x;
	while(1)
	{
		rnd=rand();
		if(rnd != 0 && rnd != RAND_MAX)
		break;
	}
	r=double(rnd)/RAND_MAX;
	x=(-1/lanmada)*log(r);
	return x;
}

void init(){
	srand((unsigned)time(NULL));
	int i,j;
	//假定初始状态时每个队列均有一个包 
	for(i = 0;i < ST;i++){
		frame[i][0].arrive = 0.0;
		frame[i][0].collsions = 0; 
	} 
		
	//提前生成所有帧的到达时间 
	for(i = 0;i < ST;i++)
		for(j = 1;j < FN;j++){
			frame[i][j].arrive = frame[i][j-1].arrive + exprand(lanmada);
			frame[i][j].collsions = 0;
		}
	//初始化所有站点 
	for(i = 0;i < ST;i++){
		station[i].collisions = 0;
		q[i].push(0);
		station[i].timeslot = rand()%SD;
	}	
	for(i = 0;i < ST;i++){
		readyin[i] = 1; 
		coll[i] = 0; 
		ssend[i] = 0;
		totalength[i] = 0;
	}
	for(j = 0;j < ST;j++)
	for(i = 0;i < FN;i++){
		wait_time_p[j][i] = 0;
		wait_line_p[j][i] = 0;
	}
	for(i = 0;i < ST;i++)
	wait_line_p[i][0]++;
		
}
int main(){
	init();//初始化 
	int i,j,k;
	double transtime;//传送时间+确认返回时间 
	double systime = 0;//系统时间 
	int sum = FN * ST;//总的服务量 
	int beTransed = 0;//已经发送的包数，包括放弃发送 
	int resend = 0;//临时储存碰撞次数 
	//仿真开始
	int min;//所有当前站点的最小timeslot 
	int flag;//判断是否会有多个包同时发送 
	int index;//要发送的站点号 
	int null = 0;
	int choosed;//p-坚持中是否成功确定发包的标志 
	while(beTransed < sum){
		null = 0;
		choosed = 1;
		for(i = 0;i < ST;i++){//判断是否五个队列均为空 
			if(q[i].size() != 0) null = 1;
		}
		if(null == 0){//对于五个站的队列均为空，但是还有包没有进入队列的情况 
			double a = 1000000000.0;
			int b;
			for(i = 0;i < ST;i++){//寻找一个离当前系统时间最近的包入列 
				if(readyin[i] < FN)
				if(frame[i][readyin[i]].arrive < a){
					a = frame[i][readyin[i]].arrive;
					b = i;
				}
			}//更新统计变量 
			systime = frame[b][readyin[b]].arrive;
			q[b].push(readyin[b]++);
			wait_line_p[b][0]++;
		}
		while(choosed){
		min = 5000;
		flag = 0;
		for(i = 0;i < ST;i++){//选择出队列长度不为0中的timeslot的最小值以此选出要发送的站 
			if(q[i].size() != 0){
				if(min == station[i].timeslot){
					flag = 1;
				}
				else if(station[i].timeslot < min){
					min = station[i].timeslot;
					index = i;
					flag = 0;
				}	
			}
		}
		if(flag == 0){
			if(rand()/(double)RAND_MAX > P){
				station[index].timeslot++;
		}else choosed = 0;
		}else{
			int count = 0;
			for(i = 0;i < ST;i++){
				if(station[i].timeslot == min){
					if(rand()/(double)RAND_MAX > P){
						station[i].timeslot++;
					}else{
						count++;
						index = i;
					}
				}
			}
			if(count == 1){
				flag = 0;
				choosed = 0;
			}else if(count > 1) choosed = 0;
		}
		
		}
		
		transtime = exprand(mu);//生成传送时间和确认返回时间 
		for(i = 0;i < ST;i++){//统计总的积分队长 
			totalength[i] += (min*TS + transtime)*q[i].size();
		}
		if(flag == 0){//未发生冲突 
			systime += min * TS;
			frame[index][q[index].front()].start = systime;
			//cout<<"station "<<index<<" send "<<q[index].front()<<" ,start time"<<systime<<endl;
			q[index].pop();
			systime += transtime;
			
			for(i = 0;i < ST;i++){//同步降低其他站的计数器 
				if(i != index && q[index].size() != 0)
					station[i].timeslot -= min;
			}

			station[index].timeslot = rand() % SD;//重新生成时间槽 
			station[index].collisions = 0;
			ssend[index]++; 
			beTransed++;
		}else{//发生冲突 
			systime += (min * TS + transtime);//需要一定时间确认冲突发生 
			for(i = 0;i < ST;i++){
				if(q[i].size() != 0){
				if(station[i].timeslot == min){
					//cout<<"station "<<i<<" send "<<q[i].front()<<" ,collision! send time: "<<systime-transtime<<endl;
					coll[i]++;
					ssend[i]++;
					frame[i][q[i].front()].collsions = ++station[i].collisions;
					if(station[i].collisions == 16){//超过最大重传次数 
						cout<<"station "<<index<<" send frame "<<q[i].front()<<" fail"<<endl;
						frame[i][q[i].front()].start = systime;
						q[i].pop();
						station[i].timeslot = rand() % SD;
						station[i].collisions = 0;
						beTransed++;
						continue;
					}
					
					if(station[i].collisions > 10)//指数后退 
						resend = 10;
					else resend = station[i].collisions;
					int e = 1;
					for(j = 0;j < resend;j++)
						e *= 2;
					station[i].timeslot = rand()%e;
				}else{//同步更新其它站点的计数器 
					if(q[i].size() != 0)
					station[i].timeslot -= min;
				}
				}
			}
		}
		for(i = 0;i < ST;i++){//在某包传输的整个时间内，添加到队列中所有可能的来包 
			for(j = readyin[i];j < FN;j++)
				if(frame[i][j].arrive <= systime){
					for(k = 0;k <= q[i].size();k++)
						wait_line_p[i][k]++;
					//cout<<j<<" come in station "<<i<<" ,arrive time: "<<frame[i][j].arrive<<endl;
					q[i].push(j);
					readyin[i]++;
				}else break;
		}
		//cout<<"-------------------------------------"<<endl;
		
	}
	/* 结果统计 */ 
	//总的传输时间
	cout<<"total service time: "<<systime/1000000<<" s"<<endl; 
	//碰撞概率
	for(i = 0;i < ST;i++){
		 cout<<"station "<<i<<" collision probability : "<<coll[i]/(double)ssend[i]<<endl;
	} 
	cout<<"------------------------------------------"<<endl;
	FILE *fp;
	//平均等待时间
	int num = FN;
	double totaltime = 0;
	for(i = 0;i < ST;i++){
		for(j = 0;j < FN;j++){
			totaltime += frame[i][j].start - frame[i][j].arrive;
		}
		cout<<"station "<<i<<" mean waiting time: "<<totaltime/(double)FN<<endl;
		totaltime = 0; 
	}
	//获取等待时间分布数据并写入文件 
	int temp;
	for(k = 0;k < ST;k++)
	for(i = 0;i < FN;i++){
		temp = (int)(frame[k][i].start - frame[k][i].arrive);
		for(j = 0;j <= temp;j++)
		wait_time_p[k][j]++;
	}
    for(i = 0;i < ST;i++){
		if(i == 0) fp = fopen("time20.txt","w");
		else if(i == 1) fp = fopen("time21.txt","w"); 
		else if(i == 2) fp = fopen("time22.txt","w");     
		else if(i == 3) fp = fopen("time23.txt","w");  
		else fp = fopen("time24.txt","w");  
		for(j = 0;j < FN;j++){
			fprintf(fp,"%d %f\n",j,wait_time_p[i][j]/(double)FN);
		}
	}
    cout<<"-------------------------------------------"<<endl;
	//平均等待队长
	for(i = 0;i < ST;i++)
		cout<<"station "<<i<<" mean queue length: "<<totalength[i]/systime<<endl;
	//获取等待队长分布数据并写入文件 
	for(i = 0;i < ST;i++){
		if(i == 0) fp = fopen("line20.txt","w");
		else if(i == 1) fp = fopen("line21.txt","w"); 
		else if(i == 2) fp = fopen("line22.txt","w"); 
		else if(i == 3) fp = fopen("line23.txt","w");  
		else fp = fopen("line24.txt","w");  
		for(j = 0;j < FN;j++){
			fprintf(fp,"%d %f\n",j,wait_line_p[i][j]/(double)FN);
		}
	}
    //吞吐量 
    cout<<"-------------------------------------------"<<endl;
    cout<<"the throughput of the whole network: "<<ST*FN/systime<<endl;
    fclose(fp);

	return 0;
}
