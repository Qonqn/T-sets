#pragma once
#include<iostream>
#include<vector>
#include<sys/stat.h>
#include<algorithm>
#include<fstream>
#include<string>
//#include<json/json.h>
#include<map>
#include"ZRedisConnectionPool.h"
#include"DouglasPeucker.h"
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
	Dataset(string path,string path1,shared_ptr<ZRedisConnection> con,int db_id);
	void load_dataset(string path, string type);
	void store_dataset();
	void print();
	void visualization();
};

struct space_index{
	string order_id;
	string index;
	string position_code;
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
	do{
		if(index != string::npos){
			substring = str.substr(start, index - start);
            dest.push_back(substring);
            start = index + separator.size();
            index = str.find(separator, start);
			if (start == string::npos) break;
		}

	}while (index != string::npos);
	//the last part
    substring = str.substr(start);
    dest.push_back(substring);

}

vector<space_index> loadInfoFromCSV(const std::string& filename){
	vector<space_index> res;

    ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return res;
    }

    string line;
    while (getline(file, line)){
		if (!line.empty() && line.back() == '\r') {
            // 删除最后一个字符
            line.erase(line.size() - 1);
        }
		space_index new_info;
        vector<string> contents;
		Split(line," ",contents);
		new_info.order_id=contents[0];
		new_info.index=contents[1];
		new_info.position_code=contents[2];
        res.push_back(new_info);
	}
    return res;
}

vector<Point> parseStringToPoints(const std::string& input) {
    std::vector<Point> points;
    std::istringstream iss(input);
    std::string pointStr;

    while (std::getline(iss, pointStr, ';')) {
        std::istringstream pointIss(pointStr);
        std::string coordinateStr;
        Point point;

        std::getline(pointIss, coordinateStr, ',');
        point.x = std::stod(coordinateStr);

        std::getline(pointIss, coordinateStr, ',');
        point.y = std::stod(coordinateStr);

        points.push_back(point);
    }

    return points;
}

string convertPointsToString(const std::vector<Point>& points) {
    std::ostringstream oss;

    for (const auto& point : points) {
        oss << point.x << "," << point.y << ";";
    }

    // 移除最后一个分号
    std::string result = oss.str();
    if (!result.empty()) {
        result.pop_back();
    }

    return result;
}

Dataset::Dataset(string path,string path1,shared_ptr<ZRedisConnection> con,int db_id) {
	//path:数据集文件  path1:数据集空间信息文件 db_id:数据库id
	RedisResult res;
    
	/*vector<space_index> space_info=loadInfoFromCSV(path1);
	cout<<"space_info's size is "<<space_info.size()<<endl;
	//读入空间索引和positioncode
    
	size = getFileSize(path.c_str());
	//查看数据集大小
	*/

	string file_type_str;
	int name_index = 0;
	int type_index = 0;     
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
	//提取文件格式，当前只处理csv
	if (file_type_str == "csv") {
		File_type file_type = csv;
		name = path.substr(name_index, type_index - name_index);
    
		RedisResult res;
		ifstream infile(path);
		string line;
		/*
		string cmd_prefix="zadd "+name+":fields";

		getline(infile, line);
		line.pop_back();
		istringstream sin(line);
		string field;
		int score=0;

		while (getline(sin, field, ','))
		{
			features.push_back(field);
			string cmd=cmd_prefix+" "+to_string(score)+" "+field;
			con->ExecCmd(cmd,res);
			score++;
		}
		//存储csv列属性
        
		string cmd="set "+name+":description "+
		           "\"This data set provides the data of the motorcycle in Shanghai in August 2017, which has been desensitized for research.\"";
		con->ExecCmd(cmd,res);
        //存储数据集摘要

		cmd="set "+name+":size "+to_string(size);
		con->ExecCmd(cmd,res);
        //存储数据集大小
        */
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
            
			order_id=space_info[count].index+"#"+res1[0];
			//space_info+order_id为key 
			string start_time=res1[1];

			vector<string> res2;
			
			Split(fields[1], "#", res2);

			for (int i = 0; i < res2.size(); i++) {
				track.push_back(res2[i]);
			}
			string track_str="";
			string start_locx="";
			string start_locy="";
			string end_locx="";
			string end_locy="";
			for(int i=0;i<track.size();i++){
				if(i==0){
					vector<string> start_loc;
					Split(track[i],",",start_loc);
                    start_locx=start_loc[0];
					start_locy=start_loc[1];
				}
				if(i==track.size()-1){
					vector<string> end_loc;
					Split(track[i],",",end_loc);
                    end_locx=end_loc[0];
					end_locy=end_loc[1];
				}
				track_str+=track[i]+";";
			}
			track_str.pop_back();
             
            vector<Point> simplifiedList;
            double epsilon = 0.001;
			vector<Point> pointList=parseStringToPoints(track_str);
			DouglasPeucker(pointList, epsilon, simplifiedList);
			string simplified_track=convertPointsToString(simplifiedList);

			if(res1[0]!=space_info[count].order_id){
				cout<<"error"<<endl;
			}
			cmd="hset "+order_id+" start_time "+start_time;
			con->ExecCmd(cmd,res);

			cmd="hset "+order_id+" track "+track_str;
			con->ExecCmd(cmd,res);
			
			cmd="hset "+order_id+" start_location_x "+start_locx;
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" start_location_y "+start_locy;
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" end_location_x "+end_locx;
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" end_location_y "+end_locy;
			con->ExecCmd(cmd,res);
			
			cmd="hset "+order_id+" position_code "+space_info[count].position_code;
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" simplified_track "+simplified_track;
			con->ExecCmd(cmd,res);
			//处理以order_id为key的信息输入
			if(track_str.size()==0||space_info[count].position_code.size()==0){
				cout<<order_id<<" error"<<endl;
			}

			count++;
			sin.clear();
		}
		cout<<count<<endl;
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

/*
Dataset::Dataset(string path,string path1,shared_ptr<ZRedisConnection> con,int db_id) {
	//path:数据集文件  path1:数据集空间信息文件 db_id:数据库id
	RedisResult res;
    vector<space_index> space_info=loadInfoFromCSV(path1);   
	//读入空间索引和positioncode
    
	size = getFileSize(path.c_str());
	//查看数据集大小

	string file_type_str;
	int name_index = 0;
	int type_index = 0;     
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
	//提取文件格式，当前只处理csv
	if (file_type_str == "csv") {
		File_type file_type = csv;
		name = path.substr(name_index, type_index - name_index);
    
		RedisResult res;
		string cmd_prefix="zadd "+name+":fields";
		ifstream infile(path);
		string line;
		getline(infile, line);
		//cout<<"first line is "<<line<<endl;
		line.pop_back();
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
		//存储csv列属性
        
		string cmd="set "+name+":description "+
		           "\"This data set provides the data of the motorcycle in Shanghai in August 2017, which has been desensitized for research.\"";
		con->ExecCmd(cmd,res);
        //存储数据集摘要

		cmd="set "+name+":size "+to_string(size);
		con->ExecCmd(cmd,res);
        //存储数据集大小

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
            
			order_id=space_info[count].index+"#"+res1[0];
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
			for (int i = 0; i < res2.size(); i++) {
				track.push_back(res2[i]);
			}
			string end_loc = res1[7] + "," + res1[8];
			track.push_back(end_loc);
			string track_str="";
			for(int i=0;i<track.size();i++){
				track_str+=track[i]+";";
			}
			track_str.pop_back();
             
            vector<Point> simplifiedList;
            double epsilon = 0.001;
			vector<Point> pointList=parseStringToPoints(track_str);
			DouglasPeucker(pointList, epsilon, simplifiedList);
			string simplified_track=convertPointsToString(simplifiedList);

            cout<<order_id<<endl;
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
			cmd="hset "+order_id+" start_location_x "+res1[4];
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" start_location_y "+res1[5];
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" end_location_x "+res1[7];
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" end_location_y "+res1[8];
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" position_code "+space_info[count].position_code;
			con->ExecCmd(cmd,res);
			cmd="hset "+order_id+" simplified_track "+simplified_track;
			con->ExecCmd(cmd,res);
			//处理以order_id为key的信息输入
            
            if(order_id=="13#1630232"){
				cout<<"simplified_track is "<<simplified_track<<endl;
				break;
			}

			count++;
			sin.clear();
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
		}


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
*/


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


