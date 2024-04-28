#include<iostream>
#include<vector>
#include<sys/stat.h>
#include<algorithm>
#include<fstream>
#include<string>
#include<map>
#include"ZRedisConnectionPool.h"

using namespace std;

void Store(string path,shared_ptr<ZRedisConnection> con,int db_id){
	ZRedisConnectionPool::init("127.0.0.1",6379,"123456",db_id);
	con = ZRedisConnectionPool::Get();
	ifstream infile(path);
	string line;
	auto start = std::chrono::high_resolution_clock::now();
    
	int num=0;
    while (getline(infile, line))
	{
		istringstream sin(line);
		vector<string> fields;
		vector<string> points;
		string field;		    
		while (getline(sin, field, ','))
		{	
			fields.push_back(field);
		}
		string traj_id=fields[0];
		string points_str=fields[1];
		string mbr_str=fields[2];
		string position_code=fields[3];
		string space_code=fields[4];
		space_code.pop_back();
		string key=space_code+"#"+traj_id;
		RedisResult res;
		string cmd;
		cmd="hset "+key+" points_str "+points_str;
		//cout<<cmd<<endl;
		con->ExecCmd(cmd,res);		
		cmd="hset "+key+" mbr_str "+mbr_str;
		//cout<<cmd<<endl;
		con->ExecCmd(cmd,res);
		cmd="hset "+key+" position_code "+position_code;
		//cout<<cmd<<endl;
		con->ExecCmd(cmd,res);		
		num++;	
	}	
	cout<<num<<endl;
	auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	cout<<duration.count()<<"ms"<<endl;
}

int main(){
    shared_ptr<ZRedisConnection> con;	
    Store("/home/njucs/redis/Store/porto_dp_0.0005_quad.csv",con,4);
}