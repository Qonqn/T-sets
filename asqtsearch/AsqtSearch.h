#include<iostream>
#include<fstream>
#include<set>
#include<map>
#include<algorithm>
#include<queue>
#include<bits/stdc++.h>
#include <chrono>
#include"ZRedisConnectionPool.h"
#include"AsqtBuild.h"
using namespace std;



map<char,set<int>> table1; //index对应的不满足的position_code
map<int,set<int>> table2; //position_code对应不满足的position_code
map<int,set<char>> table3;//position_code对应的不满足的index

int changeFirstDigit_2(long long num) {
    std::string numStr = std::to_string(num);  // 将整数转换为字符串
    numStr[0] = '2';  // 将字符串的第一个字符改为'2'
    return std::stoi(numStr);  // 将字符串转换回整数并返回
}

int changeFirstDigit_1(long long num) {
    std::string numStr = std::to_string(num);  // 将整数转换为字符串
    numStr[0] = '1';  // 将字符串的第一个字符改为'2'
    return std::stoi(numStr);  // 将字符串转换回整数并返回
}

long long space_index(long long index,vector<std::pair<double, double>>& trajectory,AsqtNode* node){
    bool isExistPoint[4];
    for (int i =0; i < 4; i++) {
        isExistPoint[i] = 0;
    }
    if(node->node_type==2){
        return index;
    }
    else if(node->node_type==1){
        double median=node->median;
        double median_1=node->median_1;
        double median_2=node->median_2;
        if(median_1==0||median_2==0){
            return index;
        }   
        for (const auto& point:trajectory){
            double x = point.first;
            double y = point.second;
            if (x <= median&& y <= median_1) {
                isExistPoint[0]=1;
            }
            else if (x >= median && y <= median_2) {
                isExistPoint[1] = 1;
            }
            else if (x <= median && y > median_1) {
                isExistPoint[2] = 1;
            }
            else {
                isExistPoint[3] = 1;
            }
        }
    }
    else{
        double median=node->median;
        double median_1=node->median_1;
        double median_2=node->median_2;
        if(median_1==0||median_2==0){
            return index;
        }   
        for (const auto& point:trajectory){
            double x = point.first;
            double y = point.second;
            if (y <= median&& x <= median_1) {
                isExistPoint[0]=1;
            }
            else if (y <= median && x > median_1) {
                isExistPoint[1] = 1;
            }
            else if (y >= median && x >= median_1) {
                isExistPoint[2] = 1;
            }
            else {
                isExistPoint[3] = 1;
            }
        }
    }
    if (isExistPoint[0] && !isExistPoint[1] && !isExistPoint[2] && !isExistPoint[3]) {
        index = index * 10;
        return space_index(index,trajectory, node->child[0]);
    }
    else if (isExistPoint[1] && !isExistPoint[0] && !isExistPoint[2] && !isExistPoint[3]) {
        index = index * 10+1;
        return space_index(index, trajectory, node->child[1]);
    }
    else if (isExistPoint[2] && !isExistPoint[0] && !isExistPoint[1] && !isExistPoint[3]) {
        index = index * 10+2;
        return space_index(index, trajectory, node->child[2]);
    }
    else if (isExistPoint[3] && !isExistPoint[0] && !isExistPoint[1] && !isExistPoint[2]) {
        index = index * 10+3;
        return space_index(index, trajectory,node->child[3]);
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
    
    while (std::getline(iss, point_str, '#')) {
        std::istringstream point_iss(point_str);
        std::string coordinate_str;
        double x, y;

        if (std::getline(point_iss, coordinate_str, '-')) {
            //cout<<coordinate_str<<endl;
            x = std::stod(coordinate_str);
        }
        
        if (std::getline(point_iss, coordinate_str, '-')) {
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
        string cmd="hget "+*i+" points_str";
        
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

set<string> search_by_space(shared_ptr<ZRedisConnection> con,AsqtNode* root,vector<pair<double,double>> query){
    set<string> keys;
    int unsatisfy_num=0;
    int satisfy_num=0;
    long long initial_index=space_index(1,query,root);
    //here is all traj range
    string initial_indexStr=to_string(initial_index);
    cout<<initial_indexStr<<endl;
    long long index=0;
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
    cout<<"position code is "<<positioncode<<endl;
    string index_str=to_string(index);
    RedisResult res;
    cout<<index_str<<endl;
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
            //cout<<res3.vecdata[j]<<" "<<res1.strdata<<endl;
            long long code=stoi(res1.strdata);
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
            cout<<res2.strdata<<endl;
            long long code=stoi(res2.strdata);
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
   cout<<"keys.size is "<<keys.size()<<endl;
   //cout<<unsatisfy_num<<endl;
   //cout<<satisfy_num<<endl;
   return Spatial_query_validation(con,keys,query[0].first,query[3].first,query[0].second,query[3].second);
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
    
    table3[1]={'2','3'};
    table3[2]={'1','3'};
    table3[3]={'1','2'};
    table3[4]={'0','3'};
    table3[5]={'0','2'};
    table3[6]={'0','1'};
    table3[7]={'0'};
    table3[8]={'1'};
    table3[9]={'2'};
    table3[10]={'3'};
    table3[11]={};

}