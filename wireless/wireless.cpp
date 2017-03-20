#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <queue>
#include <time.h>
#include <fstream>

#define ST 5//վ����
#define FN 10000 //ÿ��վҪ����֡��
#define SD 50//ÿ�η�����ʱ�������Χ 
using namespace std;

double TS = 0.01; //ʱ��۵�Ԫ 1us
double lanmada  = 1;//�������ֲ�����
double mu = 5;//����ʱ�����ȷ�Ϸ���ʱ��ֲ����� 
double P = 0.3;//p-��� 

struct Frame{
	double arrive;//����ʱ�� 
	double start;//��ʼ����ʱ��
	int collsions;//��ײ���� 
}; 

struct Station{
	int timeslot;//ʱ����� 
	int collisions;//����֡��ײ����
}; 
queue<int> q[ST];//�ȴ����� 
Station station[ST]; //վ�� 
Frame frame[ST][FN];//֡ 

int readyin[ST];//��һ����Ҫ������е�֡ 
int coll[ST];//ÿ��վ������ײ�İ��� 
int ssend[ST];//ÿ��վ�ܵķ������� 

int wait_time_p[ST][FN];//ͳ��ʱ����ʷֲ� 
int wait_line_p[ST][FN];//ͳ�ƶ��г��ȷֲ� 

double totalength[ST];//���ƻ��ֶӳ��ܺ� 
//��ָ���ֲ� 
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
	//�ٶ���ʼ״̬ʱÿ�����о���һ���� 
	for(i = 0;i < ST;i++){
		frame[i][0].arrive = 0.0;
		frame[i][0].collsions = 0; 
	} 
		
	//��ǰ��������֡�ĵ���ʱ�� 
	for(i = 0;i < ST;i++)
		for(j = 1;j < FN;j++){
			frame[i][j].arrive = frame[i][j-1].arrive + exprand(lanmada);
			frame[i][j].collsions = 0;
		}
	//��ʼ������վ�� 
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
	init();//��ʼ�� 
	int i,j,k;
	double transtime;//����ʱ��+ȷ�Ϸ���ʱ�� 
	double systime = 0;//ϵͳʱ�� 
	int sum = FN * ST;//�ܵķ����� 
	int beTransed = 0;//�Ѿ����͵İ����������������� 
	int resend = 0;//��ʱ������ײ���� 
	//���濪ʼ
	int min;//���е�ǰվ�����Сtimeslot 
	int flag;//�ж��Ƿ���ж����ͬʱ���� 
	int index;//Ҫ���͵�վ��� 
	int null = 0;
	int choosed;//p-������Ƿ�ɹ�ȷ�������ı�־ 
	while(beTransed < sum){
		null = 0;
		choosed = 1;
		for(i = 0;i < ST;i++){//�ж��Ƿ�������о�Ϊ�� 
			if(q[i].size() != 0) null = 1;
		}
		if(null == 0){//�������վ�Ķ��о�Ϊ�գ����ǻ��а�û�н�����е���� 
			double a = 1000000000.0;
			int b;
			for(i = 0;i < ST;i++){//Ѱ��һ���뵱ǰϵͳʱ������İ����� 
				if(readyin[i] < FN)
				if(frame[i][readyin[i]].arrive < a){
					a = frame[i][readyin[i]].arrive;
					b = i;
				}
			}//����ͳ�Ʊ��� 
			systime = frame[b][readyin[b]].arrive;
			q[b].push(readyin[b]++);
			wait_line_p[b][0]++;
		}
		while(choosed){
		min = 5000;
		flag = 0;
		for(i = 0;i < ST;i++){//ѡ������г��Ȳ�Ϊ0�е�timeslot����Сֵ�Դ�ѡ��Ҫ���͵�վ 
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
		
		transtime = exprand(mu);//���ɴ���ʱ���ȷ�Ϸ���ʱ�� 
		for(i = 0;i < ST;i++){//ͳ���ܵĻ��ֶӳ� 
			totalength[i] += (min*TS + transtime)*q[i].size();
		}
		if(flag == 0){//δ������ͻ 
			systime += min * TS;
			frame[index][q[index].front()].start = systime;
			//cout<<"station "<<index<<" send "<<q[index].front()<<" ,start time"<<systime<<endl;
			q[index].pop();
			systime += transtime;
			
			for(i = 0;i < ST;i++){//ͬ����������վ�ļ����� 
				if(i != index && q[index].size() != 0)
					station[i].timeslot -= min;
			}

			station[index].timeslot = rand() % SD;//��������ʱ��� 
			station[index].collisions = 0;
			ssend[index]++; 
			beTransed++;
		}else{//������ͻ 
			systime += (min * TS + transtime);//��Ҫһ��ʱ��ȷ�ϳ�ͻ���� 
			for(i = 0;i < ST;i++){
				if(q[i].size() != 0){
				if(station[i].timeslot == min){
					//cout<<"station "<<i<<" send "<<q[i].front()<<" ,collision! send time: "<<systime-transtime<<endl;
					coll[i]++;
					ssend[i]++;
					frame[i][q[i].front()].collsions = ++station[i].collisions;
					if(station[i].collisions == 16){//��������ش����� 
						cout<<"station "<<index<<" send frame "<<q[i].front()<<" fail"<<endl;
						frame[i][q[i].front()].start = systime;
						q[i].pop();
						station[i].timeslot = rand() % SD;
						station[i].collisions = 0;
						beTransed++;
						continue;
					}
					
					if(station[i].collisions > 10)//ָ������ 
						resend = 10;
					else resend = station[i].collisions;
					int e = 1;
					for(j = 0;j < resend;j++)
						e *= 2;
					station[i].timeslot = rand()%e;
				}else{//ͬ����������վ��ļ����� 
					if(q[i].size() != 0)
					station[i].timeslot -= min;
				}
				}
			}
		}
		for(i = 0;i < ST;i++){//��ĳ�����������ʱ���ڣ���ӵ����������п��ܵ����� 
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
	/* ���ͳ�� */ 
	//�ܵĴ���ʱ��
	cout<<"total service time: "<<systime/1000000<<" s"<<endl; 
	//��ײ����
	for(i = 0;i < ST;i++){
		 cout<<"station "<<i<<" collision probability : "<<coll[i]/(double)ssend[i]<<endl;
	} 
	cout<<"------------------------------------------"<<endl;
	FILE *fp;
	//ƽ���ȴ�ʱ��
	int num = FN;
	double totaltime = 0;
	for(i = 0;i < ST;i++){
		for(j = 0;j < FN;j++){
			totaltime += frame[i][j].start - frame[i][j].arrive;
		}
		cout<<"station "<<i<<" mean waiting time: "<<totaltime/(double)FN<<endl;
		totaltime = 0; 
	}
	//��ȡ�ȴ�ʱ��ֲ����ݲ�д���ļ� 
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
	//ƽ���ȴ��ӳ�
	for(i = 0;i < ST;i++)
		cout<<"station "<<i<<" mean queue length: "<<totalength[i]/systime<<endl;
	//��ȡ�ȴ��ӳ��ֲ����ݲ�д���ļ� 
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
    //������ 
    cout<<"-------------------------------------------"<<endl;
    cout<<"the throughput of the whole network: "<<ST*FN/systime<<endl;
    fclose(fp);

	return 0;
}
