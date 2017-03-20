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

double lanmada  = 1.0;//�������ֲ�����
double mu = 3.1;//����ʱ��ֲ�����

struct Packet
{
	double arrive;//����ʱ�� 
	double start;//��ʼ������ʱ�� 
	double depart;//�뿪ʱ�� 
	int size;
};

Packet packet[QN][packet_size];//ÿ�����е����� 
queue<int> packetqueue[QN];//�����ȴ����У��洢��id 

int qsize[QN];//ÿ�ֵ��ȿ�ʼʱ���еĳ��� 
int rank[QN];//ÿ�ֵ��ȿ�ʼʱ���а��ճ��ȴӴ�С��˳�� 

double qsize_count[QN];//���ƽ���ӳ�ʱ�����еĻ��ֺ� 
int readyin[QN];//ÿ��������һ����Ҫ����İ���id 

int queue_len_p[QN][packet_size];//������ʱ���ض����г��ȳ��ֵĴ��� 
int wait_time_p[QN][packet_size];//���ȴ�ʱ����������������֮����ֵĴ��� 
//��ָ���ֲ� 
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
//�����������ʼ�� 
void init(){
	
	srand((unsigned)time(NULL));
	int i,j;
	
	//�ٶ���ʼ״̬ʱÿ�����о���һ���� 
	for(i = 0;i < QN;i++){
		packet[i][0].arrive = 0.0;
		packet[i][0].size = rand() % N + 1;
	}
		
	//��ǰ�������а��ĵ���ʱ�� 
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
	double t = 0.0;//ϵͳʱ�� 
	
	int sum = packet_size * QN;//���������а��ĸ��� 
	int seviced_count = 0;//�Ѿ�������İ��ĸ��� 
	double stime = 0.0;//��¼����������ÿһ������ʱ�� 
	int front,back;//������¼���е�ͷ��βid 
	/*
		��ʼʱÿ�����о���һ����		
	*/
	for(i = 0;i < QN;i++){
		packetqueue[i].push(0);
		queue_len_p[i][0]++; 
	}
		
	int turn = 0;//ÿ�ֵı�ţ�turn = 0���һ�ֵĿ�ʼ�����һ�����У�turn = 2 ��Ǹ��ַ�������һ������ 
	int currentq;//��ǰ���к� 
	int bd = 0;//ÿ������ÿ�ַ���ʱҪ����İ����ļ����� 
	int null_queue = 0;//��¼�������ֶ���Ϊ�յ���� 

	int lso[QN];//���ÿ��ÿ������Ҫ����İ��� 
	/* ���濪ʼ��
		����ṹ��
			����������Է�������ÿ�η���Ϊ��׼�� 
			��������ÿ�ε���ʱѡ��һ�������з���
			��ʱ����һ������ʱ�䣬Ȼ���ٷ���ʱ���ڣ�
			���Ƿ���а�������У�����ӵ����С�
			������ɺ��ѡ����һ�������з��� 
			���������еİ�֮�󣬳������ 
		�㷨������
			ʹ����ת�����ȣ�
			����ÿ���з����˳����ÿ�ֿ�ʼʱ�Ķ��г��ȣ�
			ϵͳ�趨��һ����׼����������̵Ķ��У� ���
			�䳤�ȴ��ڱ�׼��������ֶԸö���Ҫһ�δ���
			��׼������������ȫ�������ꡣ��ע�⣬����ʹ�õĶӳ���ÿ�ֿ�ʼʱ�ĸ����еĳ��ȣ�
			�����м䳤�Ķ��У����ö��г��Ⱥ���̶��г��������
			�õ�����ֵ���Ըö��еĳ��ȣ�����Ǹö���Ҫ������İ��� 
			������Ҫ�Ⱥ͸ö��еĳ������Ƚϣ���������˶ӳ�������� 
			������Ϊ�ӳ�����������У�������ͬ��
			------------------------
			ÿ�ֿ�ʼʱ 
			1.������̶��г���Ϊ0���������̶���ֱ���������м����
			�ĳ��Ⱥͱ�׼�����Ƚϣ�����������׼��������С�������ӳ�
			������������з�����м���жӳ�����ֵ����������Ҫ�ȺͶӳ����Ƚϡ�
			2.������̶��к��м���о�Ϊ0�����������а������жӳ�����Ϊ0��
			�������̶��е���������
			3.�������ж���ȫΪ0���������ѡ��һ������İ�������в����� 
	*/
	while(seviced_count < sum){
		if(bd == 0){//�л�����
			if(turn == 0){
				for(i = 0;i < QN;i++){//ͳ��ÿ�ֿ�ʼʱ�Ķӳ�
					qsize[i] = 0;
					if(packetqueue[i].size() != 0){
						for(j = packetqueue[i].front();j < readyin[i];j++)
							qsize[i] += packet[i][j].size;
					}
				}
				//���նӳ��������� 
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
				//ÿ�����з������ļ��� 
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
		//���г���Ϊ0ֱ�������Ĳ��� 
		if(bd == 0 && qsize[currentq] == 0){
			turn = (turn+1)%QN;
			null_queue++;
			if(null_queue == 3){//�������о�Ϊ�յ���� 
				double p = 100000000.0;
				int q;
				for(i = 0;i < QN;i++){
					if(readyin[i] < packet_size)
					if(packet[i][readyin[i]].arrive < p){
						p = packet[i][readyin[i]].arrive;
						q = i;
					}
				}
				packetqueue[q].push(readyin[q]);//ѡ��һ�������
				//�й�ͳ�Ʊ����ĸ��� 
				queue_len_p[q][0]++;
				t = packet[q][readyin[q]++].arrive;
				null_queue = 0;
				turn  = 0;
			}
			continue;
		}
		null_queue = 0;
		stime = packet[currentq][packetqueue[currentq].front()].size * Nbtime;//����ʱ�� 
		//cout<<stime<<endl;
		//ͳ��ƽ���ӳ�
		for(i = 0;i < QN;i++)
			qsize_count[i] += stime*(double)packetqueue[i].size();
		//��ʼ����ѡ������İ� 
		front = packetqueue[currentq].front();
		//cout<<currentq<<" "<<front<<" "<<count<<" "<<readyin[currentq]<<endl;
		packet[currentq][front].start = t;
		packet[currentq][front].depart = t + stime;
		packetqueue[currentq].pop();
		t += stime;
		//�����������ӵ�������Ҫ����İ� 
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
		//������ָ���İ������л����� 
		if(bd >= lso[turn]){
			turn = (turn+1)%QN;
			bd = 0;
		}
	}
	//�ܵķ���ʱ�� 
	cout<<"simulation results:"<<endl<<endl;
	cout<<"total simulation time "<<t/1000000<<" s"<<endl;
	cout<<"------------------------------------"<<endl;
	//ƽ���ȴ�ʱ�� 
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
	//ƽ�����г��� 
	for(i = 0;i < QN;i++)
		cout<<"queue "<<i<<" average queue length "<<qsize_count[i]/t<<endl;
	//д����г��ȵĸ��ʷֲ�������ı��ļ��� 
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
	//���㲢д��ȴ�ʱ��ĸ��ʷֲ�������ı��ļ��� 
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


