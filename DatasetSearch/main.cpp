#include<iostream>
#include<fstream>
#include<set>
#include<map>
#include<algorithm>
#include<queue>
#include<bits/stdc++.h>
#include"Frechet.h"
#include"ZRedisConnectionPool.h"
#include"DouglasPeucker.h"
using namespace std;

map<char,set<int>> table1; //index对应的不满足的position_code
map<int,set<int>> table2; //position_code对应不满足的position_code
map<int,set<char>> table3;//position_code对应的不满足的index


struct TrajectorySimilarity {
    string trajectoryId;
    double similarity;

    // 重载小于运算符，用于比较轨迹相似度
    bool operator<(const TrajectorySimilarity& other) const {
        // 按照相似度的降序进行排序
        return similarity > other.similarity;
    }
};

struct SpaceSimilarity {
    int index;
    double minsimilarity;

    // 重载小于运算符，用于比较轨迹相似度
    bool operator<(const SpaceSimilarity& other) const {
        // 按照相似度的降序进行排序
        if(minsimilarity==other.minsimilarity){
            return index>other.index;
        }
        return minsimilarity > other.minsimilarity;
    }
};

struct smallerSpaceSimilarity {
    int index;
    int positioncode;
    double minsimilarity;

    // 重载小于运算符，用于比较轨迹相似度
    bool operator<(const smallerSpaceSimilarity& other) const {
        // 按照相似度的降序进行排序
        return minsimilarity > other.minsimilarity;
    }
};

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

set<string> search_metadata(shared_ptr<ZRedisConnection> con){
    set<string> metadata;
    string name="mobike_shanghai_sample_updated";
    metadata.insert(name);
    RedisResult res;
    string cmd="get "+name+":description";
    con->ExecCmd(cmd,res);
    string descrip=res.strdata;
    metadata.insert(descrip);
    cmd="get "+name+":size";
    con->ExecCmd(cmd,res);
    string size=res.strdata;
    metadata.insert(size);
    cmd="zrange mobike_shanghai_sample_updated:fields 0 -1";
    con->ExecCmd(cmd,res);
    for(int i=0;i<res.vecdata.size();i++){
        metadata.insert(res.vecdata[i]);
    }
    return metadata;
}

set<string> search_by_userid(shared_ptr<ZRedisConnection> con,string userid){
    set<string> order_id;
    string cmd="smembers user_id:"+userid;
    RedisResult res;
    con->ExecCmd(cmd,res);
    for(int i=0;i<res.vecdata.size();i++){
        order_id.insert(res.vecdata[i]);
    }
    return order_id;
} 

set<string> search_by_bikeid(shared_ptr<ZRedisConnection> con,string bikeid){
    set<string> order_id;
    string cmd="smembers bike_id:"+bikeid;
    RedisResult res;
    con->ExecCmd(cmd,res);
    for(int i=0;i<res.vecdata.size();i++){
        order_id.insert(res.vecdata[i]);
    }
    return order_id;
}

set<string> search_by_grid(shared_ptr<ZRedisConnection> con,string grid){
    set<string> order_id;
    string cmd="smembers grid:"+grid;
    RedisResult res;
    con->ExecCmd(cmd,res);
    for(int i=0;i<res.vecdata.size();i++){
        order_id.insert(res.vecdata[i]);
    }
    return order_id;
    
}

int changeFirstDigit_2(int num) {
    std::string numStr = std::to_string(num);  // 将整数转换为字符串
    numStr[0] = '2';  // 将字符串的第一个字符改为'2'
    return std::stoi(numStr);  // 将字符串转换回整数并返回
}

int changeFirstDigit_1(int num) {
    std::string numStr = std::to_string(num);  // 将整数转换为字符串
    numStr[0] = '1';  // 将字符串的第一个字符改为'2'
    return std::stoi(numStr);  // 将字符串转换回整数并返回
}

int space_index(int index,vector<std::pair<double, double>>& trajectory,double x_min,double y_min,double x_max,double y_max){
    bool isExistPoint[4];
    for (int i =0; i < 4; i++) {
        isExistPoint[i] = 0;
    }
    double x_mid = (x_min + x_max) / 2.0;
    double y_mid = (y_min + y_max) / 2.0;
    for (const auto& point:trajectory) {
        double x = point.first;
        double y = point.second;
            
        if (x <= x_mid && y <= y_mid) {
            isExistPoint[0]=1;
        }
        else if (x > x_mid && y <= y_mid) {
            isExistPoint[1] = 1;
        }
        else if (x <= x_mid && y > y_mid) {
            isExistPoint[2] = 1;
        }
        else {
            isExistPoint[3] = 1;
        }
    }
    if (isExistPoint[0] && !isExistPoint[1] && !isExistPoint[2] && !isExistPoint[3]) {
        index = index * 10;
        return space_index(index,trajectory, x_min,y_min,x_mid,y_mid);
    }
    else if (isExistPoint[1] && !isExistPoint[0] && !isExistPoint[2] && !isExistPoint[3]) {
        index = index * 10+1;
        return space_index(index, trajectory, x_mid,y_min,x_max,y_mid);
    }
    else if (isExistPoint[2] && !isExistPoint[0] && !isExistPoint[1] && !isExistPoint[3]) {
        index = index * 10+2;
        return space_index(index, trajectory, x_min,y_mid,x_mid,y_max);
    }
    else if (isExistPoint[3] && !isExistPoint[0] && !isExistPoint[1] && !isExistPoint[2]) {
        index = index * 10+3;
        return space_index(index, trajectory,x_mid,y_mid,x_max,y_max);
    }
    else {
        if (isExistPoint[0] && isExistPoint[1] && !isExistPoint[2] && !isExistPoint[3]) {
            return index*10+1;
        }
        else if (isExistPoint[0] && !isExistPoint[1] && isExistPoint[2] && !isExistPoint[3]) {
            return index*10+2; 
        }
        else if (isExistPoint[0] && !isExistPoint[1] && !isExistPoint[2] && isExistPoint[3]) {
            return index*10+3;
        }
        else if (!isExistPoint[0] && isExistPoint[1] && isExistPoint[2] && !isExistPoint[3]) {
            return index*10+4;
        }
        else if (!isExistPoint[0] && isExistPoint[1] && !isExistPoint[2] && isExistPoint[3]) {
            return index*10+5;
        }
        else if (!isExistPoint[0] && !isExistPoint[1] && isExistPoint[2] && isExistPoint[3]) {
            return index*10+6;
        }
        else if (!isExistPoint[0] && isExistPoint[1] && isExistPoint[2] && isExistPoint[3]) {
            return index*10+7;
        }
        else if (isExistPoint[0] && !isExistPoint[1] && isExistPoint[2] && isExistPoint[3]) {
            return index*10+8;
        }
        else if (isExistPoint[0] && isExistPoint[1] && !isExistPoint[2] && isExistPoint[3]) {
            return index*10+9;
        }
        else if (isExistPoint[0] && isExistPoint[1] && isExistPoint[2] && !isExistPoint[3]) {
            return index*10;
        }
        else if (isExistPoint[0] && isExistPoint[1] && isExistPoint[2] && isExistPoint[3]) {
            return changeFirstDigit_2(index);
        }
    }
    return index;
}

std::vector<std::pair<double, double>> split_trajectory_points(const std::string& trajectory_str) {
    std::vector<std::pair<double, double>> trajectory_points;
    std::istringstream iss(trajectory_str);
    std::string point_str;
    
    while (std::getline(iss, point_str, ';')) {
        std::istringstream point_iss(point_str);
        std::string coordinate_str;
        double x, y;

        if (std::getline(point_iss, coordinate_str, ',')) {
            //cout<<coordinate_str<<endl;
            x = std::stod(coordinate_str);
        }
        
        if (std::getline(point_iss, coordinate_str, ',')) {
            //cout<<coordinate_str<<endl;
            y = std::stod(coordinate_str);
        }
        
        trajectory_points.emplace_back(x, y);
    }
    
    return trajectory_points;
}

bool check_point_within_range(const std::vector<std::pair<double, double>>& trajectory_points, double x_min, double x_max, double y_min, double y_max) {
    for (const auto& point : trajectory_points) {
        double x = point.first;
        double y = point.second;
        if (x >= x_min && x <= x_max && y >= y_min && y <= y_max) {
            return true;
        }
    }
    return false;
}

bool check_trajectory_within_range(const std::vector<std::pair<double, double>>& trajectory_points, double x_min, double x_max, double y_min, double y_max) {
    for (const auto& point : trajectory_points) {
        double x = point.first;
        double y = point.second;
        if (x < x_min || x > x_max || y < y_min || y > y_max) {
            return false;
        }
    }
    return true;
}

set<string> Spatial_query_validation(shared_ptr<ZRedisConnection> con,set<string> initial_set,double x_min,double x_max,double y_min,double y_max){
    set<string> last_set;
    for(set<string>::iterator i=initial_set.begin();i!=initial_set.end();i++){
        RedisResult res;
        string cmd="hget "+*i+" track";
        //cout<<cmd<<endl;
        con->ExecCmd(cmd,res);
        string trajectory_str=res.strdata;
        vector<std::pair<double, double>> trajectory_points = split_trajectory_points(trajectory_str);
        if(check_trajectory_within_range(trajectory_points,x_min,x_max,y_min,y_max)){
            //cout<<*i<<endl;
            //cout<<"yes"<<endl;
            last_set.insert(*i);
        }
        else{
            //cout<<"no"<<endl;
        }
    }
    cout<<last_set.size()<<endl;
    return last_set;
}

set<string> search_by_space(shared_ptr<ZRedisConnection> con,double x_min,double x_max,double y_min,double y_max){
   set<string> keys;
   vector<pair<double, double>> space;
   int unsatisfy_num=0;
   int satisfy_num=0;
   space.push_back({x_min,y_min});
   space.push_back({x_min,y_max});
   space.push_back({x_max,y_max});
   space.push_back({x_max,y_min});
   int initial_index=space_index(1,space,120.487,30.999,121.809,31.477);
   //here is all traj range
   string initial_indexStr=to_string(initial_index);
   int index=0;
   int positioncode=0;
   if(initial_indexStr[0]!='2'){
    positioncode=initial_index%10;
    if(!positioncode) positioncode=10;
    index=initial_index/10;
   }
   else{
     positioncode=11;
     index=changeFirstDigit_1(initial_index);
   }
   string index_str=to_string(index);
   RedisResult res;
   //cout<<index_str.size()<<endl;
   for(int i=1;i<index_str.size();i++){
      string substr=index_str.substr(0,i);
      //cout<<"substr is "+substr<<endl;
      char next_num=index_str[i];
      //cout<<"next_num is "<<next_num<<endl;
      set<int> unsatisfyCode=table1[next_num];
      //cout<<unsatisfyCode.size()<<endl;
      string cmd="keys "+substr+"#*";
      RedisResult res3;
      con->ExecCmd(cmd,res3);
      //cout<<cmd<<endl;
      //cout<<res3.vecdata.size()<<endl;
      int count=0;
      for(int j=0;j<res3.vecdata.size();j++){
        //cout<<res.vecdata[j]<<endl;
        count++;
        RedisResult res1;
        cmd="hget "+res3.vecdata[j]+" position_code";
        con->ExecCmd(cmd,res1);
        //cout<<cmd<<endl;
        int code=stoi(res1.strdata);
        //cout<<code<<endl;
        if(!unsatisfyCode.count(code)){
            keys.insert(res3.vecdata[j]);
            satisfy_num++;
            //cout<<"satisfy"<<endl;
        }
        else{
            //cout<<"unsatisfy"<<endl;
            unsatisfy_num++;
        }
        //keys.insert(res.vecdata[j]);
      }
      //cout<<"count is "<<count<<endl;
   }
   string cmd="keys "+to_string(index)+"*";
   con->ExecCmd(cmd,res);
   set<int> unsatisfyCode=table2[positioncode];
   set<char> unsatisfyIndex=table3[positioncode];
   for(int i=0;i<res.vecdata.size();i++){
        //cout<<res.vecdata[i]<<endl;
        if(index_str.size()==res.vecdata[i].size()){
            cmd="hget "+res.vecdata[i]+" position_code";
            RedisResult res2;
            con->ExecCmd(cmd,res2);
            int code=stoi(res2.strdata);
            if(!unsatisfyCode.count(code)){
                keys.insert(res.vecdata[i]);
                satisfy_num++;
            }
            else{
                unsatisfy_num++;
            }
        }
        else{
            char that_num=res.vecdata[i][index_str.size()];
            if(!unsatisfyIndex.count(that_num)){
                keys.insert(res.vecdata[i]);
                satisfy_num++;
            }
            else{
                unsatisfy_num++;
            }   
        }   
    }
   //cout<<keys.size()<<endl;
   //cout<<unsatisfy_num<<endl;
   //cout<<satisfy_num<<endl;
   return Spatial_query_validation(con,keys,x_min,x_max,y_min,y_max);
}

void generate_dataset(shared_ptr<ZRedisConnection> con,set<string> order_id){
    ofstream dataFile;
	dataFile.open("test.csv", ios::out | ios::trunc);
    string cmd="zrange mobike_shanghai_sample_updated:fields 0 -1";
    RedisResult res;
    con->ExecCmd(cmd,res);
    cout<<endl;
    string first_line="";
    for(int i=0;i<res.vecdata.size();i++){
        first_line+=res.vecdata[i];
        first_line+=",";
    }
    first_line.pop_back();
    dataFile<<first_line;
    dataFile<<endl;
    //输入第一行的字段
    
    vector<string> fields;
    Split(first_line,",",fields);
    for(set<string>::iterator i=order_id.begin();i!=order_id.end();i++){
        string each_line=*i+",";
        for(int j=1;j<fields.size();j++){
            cmd="hget "+*i+" "+fields[j];
            con->ExecCmd(cmd,res);
            each_line+=res.strdata;
            each_line+=",";
        }
        each_line.pop_back();
        cout<<each_line;
        dataFile<<each_line;
        dataFile<<endl;
    }
    dataFile.close();
}

void initial_table(){
    set<int> index_0 {4,5,6,7};
    set<int> index_1 {2,3,6,8};
    set<int> index_2 {1,3,5,9};
    set<int> index_3 {1,2,4,10};
    table1['0']=index_0;
    table1['1']=index_1;
    table1['2']=index_2;
    table1['3']=index_3;

    set<int> position_code_1 {6};
    set<int> position_code_2 {5};
    set<int> position_code_3 {4};
    set<int> position_code_4 {3};
    set<int> position_code_5 {2};
    set<int> position_code_6 {1};
    set<int> empty_set;
    table2[1]=position_code_1;
    table2[2]=position_code_2;
    table2[3]=position_code_3;
    table2[4]=position_code_4;
    table2[5]=position_code_5;
    table2[6]=position_code_6;
    table2[7]=empty_set;
    table2[8]=empty_set;
    table2[9]=empty_set;
    table2[10]=empty_set;
    table2[11]=empty_set;
    
    table3[1]={2,3};
    table3[2]={1,3};
    table3[3]={1,2};
    table3[4]={0,3};
    table3[5]={0,2};
    table3[6]={0,1};
    table3[7]={0};
    table3[8]={1};
    table3[9]={2};
    table3[10]={3};
    table3[11]={};

}

vector<double> generate_space(vector<pair<double,double>> query_trajectory,double t){
    vector<double> space;

    double x_min = query_trajectory[0].first;
    double x_max = query_trajectory[0].first;
    double y_min = query_trajectory[0].second;
    double y_max = query_trajectory[0].second;

    // 遍历查询轨迹的每个点来计算最小边界矩形的边界
    for (const auto& point : query_trajectory) {
        double x = point.first;
        double y = point.second;
        
        // 更新最小边界矩形的边界
        if (x < x_min) {
            x_min = x;
        }
        if (x > x_max) {
            x_max = x;
        }
        if (y < y_min) {
            y_min = y;
        }
        if (y > y_max) {
            y_max = y;
        }
    }
    
    // 扩大最小边界矩形的边界
    x_min -= t;
    x_max += t;
    y_min -= t;
    y_max += t;

    // 返回扩大后的最小边界矩形
    space.push_back(x_min);
    space.push_back(x_max);
    space.push_back(y_min);
    space.push_back(y_max);
    return space;
}

set<string> purn_1(shared_ptr<ZRedisConnection> con,set<string> trajectory,double Threshold,vector<pair<double,double>> query_traj){
    set<string> ans;
    for(set<string>::iterator i=trajectory.begin();i!=trajectory.end();i++){
        RedisResult res1;
        string cmd="hget "+*i+" start_location_x";
        con->ExecCmd(cmd,res1);
        double start_location_x=atof(res1.strdata.c_str());

        RedisResult res2;
        cmd="hget "+*i+" start_location_y";
        con->ExecCmd(cmd,res2);
        double start_location_y=atof(res2.strdata.c_str());

        if(distance(make_pair(start_location_x,start_location_y),query_traj[0])>Threshold)
           continue;

        RedisResult res3;
        cmd="hget "+*i+" end_location_x";
        con->ExecCmd(cmd,res3);
        double end_location_x=atof(res3.strdata.c_str());

        RedisResult res4;
        cmd="hget "+*i+" end_location_y";
        con->ExecCmd(cmd,res4);
        double end_location_y=atof(res4.strdata.c_str());

        if(distance(make_pair(end_location_x,end_location_y),query_traj[query_traj.size()-1])>Threshold)
           continue;
        ans.insert(*i);
    }
    return ans;

}

set<string> purn_2(shared_ptr<ZRedisConnection> con,set<string> trajectory,double Threshold,vector<pair<double,double>> query_traj){
    set<string> ans;
    for(set<string>::iterator i=trajectory.begin();i!=trajectory.end();i++){
        RedisResult res;
        string cmd="hget "+*i+" simplified_track";
        con->ExecCmd(cmd,res);
        //cout<<*i<<endl;
        if(res.strdata=="REDIS_REPLY_NIL"){
            ans.insert(*i);
            continue;
        }
        vector<pair<double,double>> simplified_track=split_trajectory_points(res.strdata);
        vector<pair<double,double>> simplified_query;
        DouglasPeucker(query_traj,0.001,simplified_query);
        
        
        if(computeDistance(simplified_track,simplified_query)<Threshold){
           ans.insert(*i);
        }
          
    }
    return ans;
}

set<string> similarity_verify(shared_ptr<ZRedisConnection> con,set<string> trajectory,double Threshold,vector<pair<double,double>> query_traj){
    set<string> ans;
    for(set<string>::iterator i=trajectory.begin();i!=trajectory.end();i++){
        RedisResult res;
        string cmd="hget "+*i+" track";
        con->ExecCmd(cmd,res);
        vector<pair<double,double>> track=split_trajectory_points(res.strdata);
        if(computeDistance(track,query_traj)<Threshold)
          ans.insert(*i);
    }
    return ans;
}

set<string> Tbsimilar_trajectory_search(shared_ptr<ZRedisConnection> con,string traj_str,double Threshold){
    vector<pair<double,double>> query_traj=split_trajectory_points(traj_str);
    vector<double> space_range=generate_space(query_traj,Threshold);
    set<string> Candidate_trajectory=search_by_space(con,space_range[0],space_range[1],space_range[2],space_range[3]);
    cout<<"Candidate_trajectory's size is'"<<Candidate_trajectory.size()<<endl;
    for(set<string>::iterator i=Candidate_trajectory.begin();i!=Candidate_trajectory.end();i++){
        cout<<*i<<endl;
    }
    set<string> Candidate_trajectory_1=purn_1(con,Candidate_trajectory,Threshold,query_traj);
    cout<<"Candidate_trajectory_1'size is "<<Candidate_trajectory_1.size()<<endl;
    for(set<string>::iterator i=Candidate_trajectory_1.begin();i!=Candidate_trajectory_1.end();i++){
        cout<<*i<<endl;
    }
    set<string> Candidate_trajectory_2=purn_2(con,Candidate_trajectory_1,Threshold,query_traj);
    cout<<"Candidate_trajectory_2'size is "<<Candidate_trajectory_2.size()<<endl;
    for(set<string>::iterator i=Candidate_trajectory_2.begin();i!=Candidate_trajectory_2.end();i++){
        cout<<*i<<endl;
    }
    return similarity_verify(con,Candidate_trajectory_2,Threshold,query_traj);
} 

double lowerbound(vector<pair<double,double>> query_traj,int space_key){

}

vector<double> index_to_space(int index){
    vector<double> res;
    double x_min=120.487;
    double y_min=30.999;
    double x_max=121.809;
    double y_max=31.477;
    //here is the root space
    string index_str=to_string(index);
    for(int i=1;i<index_str.size();i++){
        if(index_str[i]=='0'){
            y_max=(y_min+y_max)/2;
            x_max=(x_min+x_max)/2;
        }
        else if(index_str[i]=='1'){
            y_max=(y_min+y_max)/2;
            x_min=(x_min+x_max)/2;
        }
        else if(index_str[i]=='2'){
            y_min=(y_min+y_max)/2;
            x_max=(x_min+x_max)/2;
        }
        else{
            y_min=(y_min+y_max)/2;
            x_min=(x_min+x_max)/2;
        }
    }
    res.push_back(x_min);
    res.push_back(y_min);
    res.push_back(x_max);
    res.push_back(y_max);
    return res;
}

double minDistTS(int index,vector<pair<double,double>> query_traj){
    vector<double> space=index_to_space(index);
    double x_min=space[0];
    double y_min=space[1];
    double x_max=space[2];
    double y_max=space[3];
    double maxDistance = 0.0;
    
    for (const std::pair<double, double>& point : query_traj) {
        double x = point.first;
        double y = point.second;
        
        // 判断点是否在矩形范围内
        if (x >= x_min && x <= x_max && y >= y_min && y <= y_max) {
            // 点在矩形内，距离为0
            continue;
        }
        
        // 计算点到矩形四个角的距离
        double distances[4];
        distances[0] = distance(point,make_pair(x_min,y_min)); 
        distances[1] = distance(point,make_pair(x_max,y_min));
        distances[2] = distance(point,make_pair(x_min,y_max));
        distances[3] = distance(point,make_pair(x_max,y_max));
        
        // 找到最大距离
        double pointMaxDistance = distances[0];
        for (int i = 1; i < 4; ++i) {
            if (distances[i] > pointMaxDistance) {
                pointMaxDistance = distances[i];
            }
        }
        
        // 更新全局最大距离
        if (pointMaxDistance > maxDistance) {
            maxDistance = pointMaxDistance;
        }
    }
    
    return maxDistance;
}

double minDistTsS(int index,int positioncode,vector<pair<double,double>> query_traj){
    double res=0;
    double minDist[4];
    for(int i=0;i<4;i++){
        double smaller_index=index*10+i;
        minDist[i]=minDistTS(smaller_index,query_traj);
    }
    if(!minDistTS(index,query_traj)){
        vector<double> space=index_to_space(index);
        int initial_index=space_index(index,query_traj,space[0],space[1],space[2],space[3]);
        string initial_indexStr=to_string(initial_index);
        int position_code=0;
        if(initial_indexStr[0]!='2'){
           position_code=initial_index%10;
           if(!position_code) position_code=10;
        }
        else{
           position_code=11;
        }
        if(position_code==positioncode)
           return 0;
    }
    switch (positioncode)
    {
    case 1/* constant-expression */:
        /* code */
        res=max(minDist[0],minDist[1]);
        break;
    case 2:
        res=max(minDist[0],minDist[2]);
        break;
    case 3:
        res=max(minDist[0],minDist[3]);
        break;
    case 4:
        res=max(minDist[1],minDist[2]);
        break;
    case 5:
        res=max(minDist[1],minDist[3]);
        break;
    case 6:
        res=max(minDist[2],minDist[3]);
        break;
    case 7:
        res=max(max(minDist[1],minDist[2]),minDist[3]);
        break;
    case 8:
        res=max(max(minDist[0],minDist[2]),minDist[3]);
        break;
    case 9:
        res=max(max(minDist[0],minDist[1]),minDist[3]);
        break;
    case 10:
        res=max(max(minDist[0],minDist[1]),minDist[2]);
        break;
    case 11:
        res=max(max(minDist[0],minDist[1]),max(minDist[2],minDist[3]));
        break;
    default:
        break;
    }
    return res;
}

double DistTT(shared_ptr<ZRedisConnection> con,string tid,vector<pair<double,double>> query_traj){
    //here tid is the complete index of traj
    RedisResult res;
    string cmd="hget "+tid+" track";
    con->ExecCmd(cmd,res);
    vector<pair<double,double>> traj=split_trajectory_points(res.strdata);
    return computeDistance(traj,query_traj);    
}

vector<string> trajInSmallerSpace(shared_ptr<ZRedisConnection> con,int index,int positioncode){
    vector<string> trajs;
    RedisResult res;
    string cmd="keys "+to_string(index)+"#*";
    con->ExecCmd(cmd,res);
    if(index==13212&&positioncode==1){
        cout<<res.vecdata.size()<<endl;
    }
    for(int i=0;i<res.vecdata.size();i++){
        cmd="hget "+res.vecdata[i]+" position_code";
        RedisResult res2;
        con->ExecCmd(cmd,res2);
        if(index==13212&&positioncode==1){
           cout<<res2.strdata<<endl;
        }
        
        if(res2.strdata==to_string(positioncode)){
            trajs.push_back(res.vecdata[i]);
        }
    }
    return trajs;
}

double updateQueue(priority_queue<TrajectorySimilarity>& q,int k){
   //todo:resize queue and return new threshold
   double threshold=0;
   if(q.size()<k)
       return (numeric_limits<double>::max)();
   else if(q.size()==k){
       priority_queue<TrajectorySimilarity> tq_copy = q;
       while (!tq_copy.empty()) {
          TrajectorySimilarity top_element = tq_copy.top();
          threshold=top_element.similarity;
          tq_copy.pop();
       }
       return threshold;
   }
   else{
       priority_queue<TrajectorySimilarity> tq_copy = q;
       for(int i=0;i<k;i++){
          TrajectorySimilarity top_element = q.top();
          cout<<top_element.similarity<<endl;
          threshold=top_element.similarity;
          q.pop();
       }
       q.pop();
       for(int i=0;i<k;i++){
          TrajectorySimilarity top_element = tq_copy.top();
          tq_copy.pop();
          q.push(top_element);
       }
       return threshold;
   }
}

priority_queue<TrajectorySimilarity> Top_k_similar_trajectory_search(shared_ptr<ZRedisConnection> con,string traj_str,int k){
    vector<pair<double,double>> query_traj=split_trajectory_points(traj_str);
    priority_queue<TrajectorySimilarity> trajectoryQueue;
    priority_queue<SpaceSimilarity> spaceQueue;
    SpaceSimilarity rootSpace;
    rootSpace.index=1;
    rootSpace.minsimilarity=minDistTS(1,query_traj);
    spaceQueue.push(rootSpace);
    priority_queue<smallerSpaceSimilarity> smallerSpaceQueue;
    double threshold=(numeric_limits<double>::max)();
    while(!spaceQueue.empty()){
        cout<<"current threshold is "<<threshold<<endl;
        SpaceSimilarity space=spaceQueue.top();
        cout<<"current space's index is "<<space.index<<endl;
        spaceQueue.pop();
        if(!smallerSpaceQueue.empty()){
            cout<<"smallerSpace minsim is "<< smallerSpaceQueue.top().minsimilarity<<endl;
            cout<<"Space minsim is "<<space.minsimilarity<<endl;
        }
        if(smallerSpaceQueue.empty()||smallerSpaceQueue.top().minsimilarity>=space.minsimilarity){
            if(space.minsimilarity>threshold)
               return trajectoryQueue;
            for(int i=1;i<=11;i++){
                smallerSpaceSimilarity smallerSpace;
                smallerSpace.index=space.index;
                smallerSpace.positioncode=i;
                smallerSpace.minsimilarity=minDistTsS(space.index,i,query_traj);
                cout<<smallerSpace.minsimilarity<<endl;
                smallerSpaceQueue.push(smallerSpace);
            }
            for(int i=0;i<=3;i++){
                SpaceSimilarity childSpace;
                childSpace.index=space.index*10+i;
                RedisResult res;
                string cmd="keys "+to_string(childSpace.index)+"#*";
                con->ExecCmd(cmd,res);
                int traj_count=res.vecdata.size();
                if(!traj_count) continue;
                childSpace.minsimilarity=minDistTS(childSpace.index,query_traj);
                cout<<childSpace.minsimilarity<<endl;
                spaceQueue.push(childSpace);
            }
        }
        else{
            while(!smallerSpaceQueue.empty() && smallerSpaceQueue.top().minsimilarity<=space.minsimilarity){
                smallerSpaceSimilarity smallerSpace=smallerSpaceQueue.top();
                smallerSpaceQueue.pop();
                cout<<smallerSpace.index<<" "<<smallerSpace.positioncode<<endl;
                if(smallerSpace.minsimilarity>threshold)
                   return trajectoryQueue;
                vector<string> candidateTid=trajInSmallerSpace(con,smallerSpace.index,smallerSpace.positioncode);
                cout<<"candidate traj's size is "<<candidateTid.size()<<endl;
                for(int i=0;i<candidateTid.size();i++){
                    double dist=DistTT(con,candidateTid[i],query_traj);
                    if(dist<threshold){
                        TrajectorySimilarity traj;
                        traj.trajectoryId=candidateTid[i];
                        
                        traj.similarity=dist;
                        trajectoryQueue.push(traj);
                        threshold=updateQueue(trajectoryQueue,k);
                        if(traj.trajectoryId=="13212#78387"){
                            cout<<"arrive here "<<dist<<endl;
                            cout<<"threshold is "<<threshold<<endl;
                        }
                    }
                }
            }
        }
    }
}

int main(){
    ZRedisConnectionPool::init("127.0.0.1",6379,"123456",1);
    shared_ptr<ZRedisConnection> con=ZRedisConnectionPool::Get();
    initial_table();
    set<string> metadata=search_metadata(con);
    set<string> test;
    test.insert("13212#78387");
    //search_by_space(con,121.348,121.357,31.388,31.389);
    string query_traj="121.348,31.389;121.347,31.392;121.348,31.389;121.349,31.390;121.350,31.390;121.351,31.390;121.351,31.391;121.352,31.391;121.353,31.391;121.353,31.394;121.354,31.391;121.355,31.391;121.356,31.389;121.356,31.390;121.356,31.391;121.357,31.388;121.357,31.389;121.357,31.388";
    /*
    set<string> tb_simtraj=Tbsimilar_trajectory_search(con,query_traj,0.03);
    cout<<"tb_simtraj'size is "<<tb_simtraj.size()<<endl;
    for(set<string>::iterator i=tb_simtraj.begin();i!=tb_simtraj.end();i++){
        cout<<*i<<endl;
    }
    */
    priority_queue<TrajectorySimilarity> top_k_simTraj=Top_k_similar_trajectory_search(con,query_traj,50);
    
    while(!top_k_simTraj.empty()){
        TrajectorySimilarity res=top_k_simTraj.top();
        cout<<res.trajectoryId<<" "<<res.similarity<<endl;
        top_k_simTraj.pop();
    }
    
    //generate_dataset(con,test);
    /*for(set<string>::iterator it=metadata.begin();it!=metadata.end();it++){
        cout<<*it<<endl;
    }
    set<string> orderid_1=search_by_userid(con,"10080");
    set<string> orderid_2=search_by_bikeid(con,"158357");
    for(set<string>::iterator it=orderid_1.begin();it!=orderid_1.end();it++){
        cout<<*it<<" ";
    }
    cout<<endl;
    for(set<string>::iterator it=orderid_2.begin();it!=orderid_2.end();it++){
        cout<<*it<<" ";
    }
    cout<<endl;
    vector<string> grid;
    set<string> orderid_3;
    for(int i=0;i<grid.size();i++){
        set<string> res=search_by_grid(con,grid[i]);
        set_union(orderid_3.begin(),orderid_3.end(),res.begin(),res.end(),inserter(orderid_3,orderid_3.begin()));
    }
    for(set<string>::iterator it=orderid_3.begin();it!=orderid_3.end();it++){
        cout<<*it<<" ";
    }*/

    return 0;
}