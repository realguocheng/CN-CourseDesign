#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <queue>
#include <time.h>
#include <fstream>

#define QN 3
#define BASIC 20
#define packet_size 10000
#define Nbtime 0.03
#define N 20

using namespace std;

double lanmada  = 1.0;//到达间隔分布参数
double mu = 3.1;//服务时间分布参数

struct Packet
{
	double arrive;//到达时间 
	double start;//开始被服务时间 
	double depart;//离开时间 
	int size;
};

Packet packet[QN][packet_size];//每个队列的来包 
queue<int> packetqueue[QN];//三个等待队列，存储包id 

int qsize[QN];//每轮调度开始时队列的长度 
int rank[QN];//每轮调度开始时队列按照长度从大到小的顺序 

double qsize_count[QN];//求解平均队长时，队列的积分和 
int readyin[QN];//每个队列下一个将要到达的包的id 

int queue_len_p[QN][packet_size];//包到达时，特定队列长度出现的次数 
int wait_time_p[QN][packet_size];//包等待时间在两个相邻整数之间出现的次数 
//负指数分布 
inline double exprand(double lanmada)
{
	int rnd;
	double r,x;
	while(1)
	{
		rnd=rand();
		if(rnd!=0 && rnd != RAND_MAX)
		break;
	}
	r=double(rnd)/RAND_MAX;
	x=(-1/lanmada)*log(r);
	return x;
}
//变量和数组初始化 
void init(){
	
	srand((unsigned)time(NULL));
	int i,j;
	
	//假定初始状态时每个队列均有一个包 
	for(i = 0;i < QN;i++){
		packet[i][0].arrive = 0.0;
		packet[i][0].size = rand() % N + 1;
	}
		
	//提前生成所有包的到达时间 
	for(i = 0;i < QN;i++)
		for(j = 1;j < packet_size;j++){
			packet[i][j].arrive = packet[i][j-1].arrive + exprand(i*0.1+lanmada);
			packet[i][j].size = rand() % N + 1;
		}
	for(i = 0;i < QN;i++){
		qsize_count[i] = 0.0;
		rank[i] = i;
		readyin[i] = 1;
	}
		
	for(i = 0;i < QN;i++){
		for(j = 0;j < packet_size;j++){
			wait_time_p[i][j] = 0;
			queue_len_p[i][j] = 0;
		}
	}
}

int main(){
	init(); 
	int i,j,k;
	double t = 0.0;//系统时间 
	
	int sum = packet_size * QN;//三队列所有包的个数 
	int seviced_count = 0;//已经被服务的包的个数 
	double stime = 0.0;//记录服务器服务每一个包的时间 
	int front,back;//辅助记录队列的头和尾id 
	/*
		初始时每个队列均有一个包		
	*/
	for(i = 0;i < QN;i++){
		packetqueue[i].push(0);
		queue_len_p[i][0]++; 
	}
		
	int turn = 0;//每轮的编号，turn = 0标记一轮的开始服务第一个队列，turn = 2 标记该轮服务的最后一个队列 
	int currentq;//当前队列号 
	int bd = 0;//每个队列每轮服务时要服务的包数的计数器 
	int null_queue = 0;//记录连续出现队列为空的情况 

	int lso[QN];//存放每轮每个队列要服务的包数 
	/* 仿真开始：
		程序结构：
			程序的运行以服务器的每次服务为标准； 
			服务器在每次调度时选择一个包进行服务，
			此时会有一个服务时间，然后再服务时间内，
			查是否会有包到达，若有，则添加到队列。
			服务完成后会选择下一个包进行服务。 
			服务完所有的包之后，程序结束 
		算法描述：
			使用轮转法调度；
			但是每轮中服务的顺序按照每轮开始时的队列长度；
			系统设定了一个标准数，对于最短的队列， 如果
			其长度大于标准数，则该轮对该队列要一次处理
			标准数个包，否则全部处理完。（注意，所有使用的队长是每轮开始时的各队列的长度）
			对于中间长的队列，将该队列长度和最短队列长度相除，
			得到比例值乘以该队列的长度，这就是该队列要被服务的包数 
			，但是要先和该队列的长度作比较，如果超过了队长，则服务 
			服务量为队长。对于最长队列，做法相同。
			------------------------
			每轮开始时 
			1.对于最短队列长度为0的情况，最短队列直接跳过，中间队列
			的长度和标准数作比较，大于则服务标准数个包，小于则服务队长
			个包，而最长队列服务和中间队列队长比例值个包，但是要先和队长作比较。
			2.对于最短队列和中间队列均为0的情况，最长队列按照所有队长均不为0的
			情况下最短队列的做法调度
			3.对于所有队列全为0的情况，则选择一个最近的包到达队列并服务。 
	*/
	while(seviced_count < sum){
		if(bd == 0){//切换队列
			if(turn == 0){
				for(i = 0;i < QN;i++){//统计每轮开始时的队长
					qsize[i] = 0;
					if(packetqueue[i].size() != 0){
						for(j = packetqueue[i].front();j < readyin[i];j++)
							qsize[i] += packet[i][j].size;
					}
				}
				//按照队长进行排序 
				for(i = 1;i < QN;i++){
					for(j = i;j > 0;j--)
					{
						if(qsize[rank[j]] > qsize[rank[j-1]]){
							int temp = rank[j];
							rank[j] = rank[j-1];
							rank[j-1] = temp;
						}
					}
				}
				currentq = rank[0];	
				double needServed; 
				int cc = 0;
				//每条队列服务量的计算 
				if(qsize[rank[2]] != 0){
					needServed = (qsize[rank[2]] > BASIC) ? BASIC : qsize[rank[2]];
					for(i = packetqueue[rank[2]].front();i < readyin[rank[2]];i++){
						cc += packet[rank[2]][i].size;
						if(cc >= needServed){
							lso[2] = i - packetqueue[rank[2]].front() + 1;
							cc = 0; break;
						}
					}
					needServed = (qsize[rank[1]]/(double)qsize[rank[2]])*needServed;
					for(i = packetqueue[rank[1]].front();i < readyin[rank[1]];i++){
						cc += packet[rank[1]][i].size;
						if(cc >= needServed){
							lso[1] = i - packetqueue[rank[1]].front() + 1;
							cc = 0; break;
						}
					}
					needServed = (qsize[rank[0]]/(double)qsize[rank[1]])*needServed;
					for(i = packetqueue[rank[0]].front();i < readyin[rank[0]];i++){
						cc += packet[rank[0]][i].size;
						if(cc >= needServed){
							lso[0] = i - packetqueue[rank[0]].front() + 1;
							cc = 0; break;
						}
					}
				}else{
					if(qsize[rank[1]] != 0){
						needServed = (qsize[rank[1]] > BASIC) ? BASIC : qsize[rank[1]];
						for(i = packetqueue[rank[1]].front();i < readyin[rank[1]];i++){
							cc += packet[rank[1]][i].size;
							if(cc >= needServed){
								lso[1] = i - packetqueue[rank[1]].front() + 1;
								cc = 0; break;
							}
						}
						needServed = (qsize[rank[0]]/(double)qsize[rank[1]])*needServed;
						for(i = packetqueue[rank[0]].front();i < readyin[rank[0]];i++){
							cc += packet[rank[0]][i].size;
							if(cc >= needServed){
								lso[0] = i - packetqueue[rank[0]].front() + 1;
								cc = 0; break;
							}
						}
					}else{
						if(qsize[rank[0]] != 0){
							needServed = (qsize[rank[0]] > BASIC) ? BASIC : qsize[rank[0]];
							for(i = packetqueue[rank[0]].front();i < readyin[rank[0]];i++){
								cc += packet[rank[0]][i].size;
								if(cc >= needServed){
									lso[0] = i - packetqueue[rank[0]].front() + 1;
									cc = 0; break;
								}
							}
						}
					}
				} 
				
			}else if(turn == 1){
				currentq = rank[1];	
			}else{
				currentq = rank[2];	
			}
		}
		//队列长度为0直接跳过的部分 
		if(bd == 0 && qsize[currentq] == 0){
			turn = (turn+1)%QN;
			null_queue++;
			if(null_queue == 3){//三条队列均为空的情况 
				double p = 100000000.0;
				int q;
				for(i = 0;i < QN;i++){
					if(readyin[i] < packet_size)
					if(packet[i][readyin[i]].arrive < p){
						p = packet[i][readyin[i]].arrive;
						q = i;
					}
				}
				packetqueue[q].push(readyin[q]);//选择一个包入队
				//有关统计变量的更新 
				queue_len_p[q][0]++;
				t = packet[q][readyin[q]++].arrive;
				null_queue = 0;
				turn  = 0;
			}
			continue;
		}
		null_queue = 0;
		stime = packet[currentq][packetqueue[currentq].front()].size * Nbtime;//服务时间 
		//cout<<stime<<endl;
		//统计平均队长
		for(i = 0;i < QN;i++)
			qsize_count[i] += stime*(double)packetqueue[i].size();
		//开始服务选择出来的包 
		front = packetqueue[currentq].front();
		//cout<<currentq<<" "<<front<<" "<<count<<" "<<readyin[currentq]<<endl;
		packet[currentq][front].start = t;
		packet[currentq][front].depart = t + stime;
		packetqueue[currentq].pop();
		t += stime;
		//服务过程中添加到队列中要到达的包 
		for(i = 0;i < QN;i++){
			for(j = readyin[i];j < packet_size;j++){
				if(packet[i][j].arrive <= t){
					for(k = 0;k <= packetqueue[i].size();k++)
					queue_len_p[i][k]++;
					packetqueue[i].push(j);
				}
				else
					break;
			}
			if(packetqueue[i].size() != 0) 
				readyin[i] = packetqueue[i].back() + 1;
		}
		
		seviced_count++;
		bd++;
		//服务完指定的包数后，切换队列 
		if(bd >= lso[turn]){
			turn = (turn+1)%QN;
			bd = 0;
		}
	}
	//总的仿真时间 
	cout<<"simulation results:"<<endl<<endl;
	cout<<"total simulation time "<<t/1000000<<" s"<<endl;
	cout<<"------------------------------------"<<endl;
	//平均等待时间 
	double total_wait = 0.0;
	for(i = 0;i < QN;i++){
		int count = 0;
		for(j = 0;j < packet_size;j++){
			total_wait += packet[i][j].start - packet[i][j].arrive;
		}
		cout<<"queue "<<i<<" average wait time "<<total_wait/packet_size/1000000<<" s"<<endl;
		total_wait = 0.0;
	}
	cout<<"------------------------------------"<<endl;
	//平均队列长度 
	for(i = 0;i < QN;i++)
		cout<<"queue "<<i<<" average queue length "<<qsize_count[i]/t<<endl;
	//写入队列长度的概率分布情况到文本文件中 
	FILE *fp;
	for(i = 0;i < QN;i++){
    	if(i == 0)
    		fp = fopen("line0.txt","w");
    	else if(i == 1)
    		fp = fopen("line1.txt","w");
    	else
    		fp = fopen("line2.txt","w");
    	    
		for(j = 0;j < packet_size;j++)
    		fprintf(fp,"%d %f\n",j,queue_len_p[i][j]/(double)packet_size);
	}
	//计算并写入等待时间的概率分布情况到文本文件中 
	int temp;
	for(i = 0;i < QN;i++)
	for(j = 0;j < packet_size;j++){
		temp = packet[i][j].start - packet[i][j].arrive;
		//cout<<temp<<endl;
		for(k = 0;k <= temp;k++)
		wait_time_p[i][k]++;
	}
    for(i = 0;i < QN;i++){
    	if(i == 0)
    		fp = fopen("time0.txt","w");
    	else if(i == 1)
    		fp = fopen("time1.txt","w");
    	else
    		fp = fopen("time2.txt","w");
    	    
		for(j = 0;j < packet_size;j++)
    		fprintf(fp,"%d %f\n",j,wait_time_p[i][j]/(double)packet_size);
	}
	fclose(fp);
	return 0;
}


