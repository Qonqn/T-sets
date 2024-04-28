#include<iostream>
#include<vector>
#include<sys/stat.h>
#include<algorithm>
#include<fstream>
#include<string>
#include<map>
#include<thread>
#include"ZRedisConnectionPool.h"
using namespace std;

void Store(string path,shared_ptr<ZRedisConnection> con,int db_id){
	ZRedisConnectionPool::init("127.0.0.1",6379,"123456",db_id);
	con = ZRedisConnectionPool::Get();
	ifstream infile(path);
	string line;
	auto start = std::chrono::high_resolution_clock::now();
    
	int num=0;
    getline(infile, line);
    while (getline(infile, line))
	{
		line.pop_back();
        stringstream ss(line);
        string key, startTime1,startTime2,endTime1,endTime2;

        // 使用逗号分割每一行数据
        getline(ss, key, ',');
        getline(ss, startTime1, ' ');
		getline(ss, startTime2, ',');
        getline(ss, endTime1, ' ');
		getline(ss, endTime2, ',');
        string time_range=startTime1+"-"+startTime2+"#"+endTime1+"-"+endTime2;
		RedisResult res;
		string cmd;
		cmd="set "+key+" "+time_range;
        cout<<cmd<<endl;
		con->ExecCmd(cmd,res);		
		num++;	
	}	
	cout<<num<<endl;
	auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	cout<<duration.count()<<"ms"<<endl;
    infile.close();
}

int main() {
    shared_ptr<ZRedisConnection> con;	
    Store("/home/njucs/redis/Store/output_info_porto.csv",con,4);
    return 0;
}
