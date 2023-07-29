#pragma once
#include<string.h>
#include<string>
#include<iostream>
#include <fstream>
#include <sstream>
#include<vector>
#include <sys/stat.h>
#include<algorithm>
//#include<json/json.h>
#include<map>
#include"ZRedisConnectionPool.h"
//using namespace Json;
using namespace std;

class Dataset {
	string source;
	enum Entity_type { taxi, bike, check_in } entity_type;
	enum File_type { json, csv, txt, xls } file_type;
	int entity_num;
	int record_num;
	int point_num;
	int total_len;
	size_t size;
	vector<string> features;
	vector<string> example;
public:
	string name;
	Dataset(string path,shared_ptr<ZRedisConnection> con,int db_id);
	void load_dataset(string path, string type);
	void store_dataset();
	void print();
	void visualization();
};

size_t getFileSize(const char* fileName) {

	if (fileName == NULL) {
		return 0;
	}

	
	struct stat statbuf;

	stat(fileName, &statbuf);

	size_t filesize = statbuf.st_size;

	return filesize;
}

void Split(const std::string& src, const std::string& separator, std::vector<std::string>& dest) //字符串分割到数组
{

//参数1：要分割的字符串；参数2：作为分隔符的字符；参数3：存放分割后的字符串的vector向量

string str = src;
string substring;
string::size_type start = 0, index;
dest.clear();
index = str.find_first_of(separator, start);
do
{
if (index != string::npos)
{
substring = str.substr(start, index - start);
dest.push_back(substring);
start = index + separator.size();
index = str.find(separator, start);
if (start == string::npos) break;
}
} while (index != string::npos);

//the last part
substring = str.substr(start);
dest.push_back(substring);

}

Dataset::Dataset(string path,shared_ptr<ZRedisConnection> con,int db_id) {
	RedisResult res;
    RedisStatus status = con->ExecCmd("keys *",res);
	string file_type_str;
	int name_index = 0;
	int type_index = 0;
	size = getFileSize(path.c_str());
	cout << size << endl;
	for (int i = path.length() - 1; i >= 0; i--) {
		if (path[i] == '.') {
			file_type_str = path.substr(i + 1, path.length() - 1 - i);
			type_index = i;
		}
		if (path[i] == '/') {
			name_index = i + 1;
			break;
		}
	}
	if (file_type_str == "csv") {
		File_type file_type = csv;
		name = path.substr(name_index, type_index - name_index);
		RedisResult res;
		string metadata_cmd="hset "+to_string(db_id)+" name"+" "+name;
		//cout<<metadata_cmd<<endl;
		con->ExecCmd(metadata_cmd,res);

		
		string cmd_prefix="zadd "+name+":fields";
		ifstream infile(path);
		string line;
		getline(infile, line);
		//cout<<"first line is "<<line<<endl;
		istringstream sin(line);
		string field;
		int score=0;

		while (getline(sin, field, ','))
		{
			features.push_back(field);
			string cmd=cmd_prefix+" "+to_string(score)+" "+field;
			//cout<<cmd<<endl;
			con->ExecCmd(cmd,res);
			score++;
		}
		//handle features
        
		string cmd="set "+name+":description "+
		           "\"This data set provides the data of the motorcycle in Shanghai in August 2017, which has been desensitized for research.\"";
		con->ExecCmd(cmd,res);
		int count = 0;
		while (getline(infile, line))
		{
			istringstream sin(line);
			vector<string> fields;
			vector<string> track;
			string field;
			int data_seq=0;
			string order_id="";
			while (getline(sin, field, '"'))
			{	
				fields.push_back(field);
			}
			RedisResult res;
			string cmd;
			vector<string> res1;
			Split(fields[0], ",", res1);

			order_id="order_id:"+res1[0];
			//order_id为key 
			string bike_id=res1[1];
			string user_id=res1[2];
			string start_time=res1[3];
			replace(start_time.begin(),start_time.end(),' ','T');
			//space in start_time and end_time ,we need del it
			string end_time=res1[6];
            replace(end_time.begin(),end_time.end(),' ','T');


			vector<string> res2;
			Split(fields[1], "#", res2);
			string start_loc = res1[4] + "," + res1[5];
			track.push_back(start_loc);
			for (int i = 0; i < res1.size(); i++) {
				track.push_back(res2[i]);
			}
			string end_loc = res1[7] + "," + res1[8];
			track.push_back(end_loc);
			string track_str="";
			for(int i=0;i<track.size();i++){
				track_str+=track[i]+";";
			}
			track_str.pop_back();

			cmd="hset "+order_id+" bikeid "+bike_id;
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" userid "+user_id;
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" start_time "+start_time;
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" end_time "+end_time;
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" track "+track_str;
			con->ExecCmd(cmd,res);
			//处理以order_id为key的信息输入

            cmd="sadd bike_id:"+bike_id+" "+order_id;
            con->ExecCmd(cmd,res);
            cmd="sadd user_id:"+user_id+" "+order_id;
            con->ExecCmd(cmd,res);
            //处理bikeid与userid为key的信息输入 

            if(count==0){
				vector<vector<string>> grid;
				vector<string> test;
			    test.push_back("83");
				grid.push_back(test);
			    for(int j=0;j<grid[count].size();j++){
				   cmd="sadd grid:"+grid[count][j]+" "+order_id;
				   con->ExecCmd(cmd,res);
				}
			}
			
			//处理key为网格id的信息输入

			count++;
			if(count==1)
			  break;
		}
		record_num = count;
	}
	else if (file_type_str == "json") {
		/*ifstream ifs(path);
		Reader rd;
		Value root;
		rd.parse(ifs, root);
		if (root.isObject()) {
			cout << "obj" << endl;
			Json::Value::Members mem = root.getMemberNames();
			for (auto iter = mem.begin(); iter != mem.end(); iter++) {
				printf("%s : ", (*iter).c_str());       // ��ӡ����
			}
		}*/


	}
	else if (file_type_str == "txt") {
		string features_str;
		cout << "please input the features: " << endl;
		getline(cin, features_str);
		istringstream str(features_str);
		while (str >> features_str) {
			features.push_back(features_str);
		}

		FILE* fd;
		int count = 0;
		if (fd = fopen(path.c_str(), "r"))
		{
			while (!feof(fd))
			{
				if ('\n' == fgetc(fd))
				{
					count++;
				}
			}
		}
		record_num = count;
		if (fd)
		{
			fclose(fd);
		}
	}



}

void Dataset::store_dataset() {



}

void Dataset::load_dataset(string path, string type) {


}

void Dataset::print() {
	cout << "name: ";
	cout << name << endl;
	cout << "features: ";
	for (int i = 0; i < this->features.size(); i++) {
		cout << this->features[i] << " ";
	}
	cout << endl;
	cout << "num_rows: ";
	cout << this->record_num << endl;
}

void Dataset::visualization() {

}


