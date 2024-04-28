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

void StoreWithMultithreading(string path, shared_ptr<ZRedisConnection> con, int db_id, int num_threads) {
    vector<thread> threads;
    ifstream infile(path);
    int total_lines = count(istreambuf_iterator<char>(infile), istreambuf_iterator<char>(), '\n');
    infile.clear();
    infile.seekg(0, ios::beg);
    int lines_per_thread = total_lines / num_threads;
    int start_line = 0;
    int end_line = 0;
    for (int i = 0; i < num_threads; ++i) {
        end_line = start_line + lines_per_thread;
        if (i == num_threads - 1) {
            end_line = total_lines; // Ensure the last thread processes all remaining lines
        }
        threads.emplace_back(StoreThread, path, con, db_id, start_line, end_line);
        start_line = end_line;
    }
    for (auto& t : threads) {
        t.join();
    }
}

int main(){
    shared_ptr<ZRedisConnection> con;	
    Store("/home/njucs/redis/shanghai_dp_30000_0.0005_quad.csv",con,2);
}