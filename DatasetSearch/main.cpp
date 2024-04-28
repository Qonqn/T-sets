#include<iostream>
#include<fstream>
#include<set>
#include<map>
#include<algorithm>
#include<queue>
#include<bits/stdc++.h>
#include <chrono>
#include"ZRedisConnectionPool.h"
using namespace std;


int main(){
    shared_ptr<ZRedisConnection> con;
    ZRedisConnectionPool::init("127.0.0.1",6379,"123456",3);
	con = ZRedisConnectionPool::Get();

    
    ifstream file("selected_trajectories.txt");
    string line;
    int test_num=0;
    vector<int64_t> all_time;
    while (getline(file, line)) {
        auto start_time = std::chrono::high_resolution_clock::now();
        string query_traj;
        query_traj=line;
        priority_queue<TrajectorySimilarity> top_k_simTraj=Top_k_similar_trajectory_search(con,query_traj,500);
        auto end_time = std::chrono::high_resolution_clock::now();
        // 计算程序执行时间（以毫秒为单位）
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        // 打印执行时间
        std::cout << "程序执行时间: " << duration.count() << " 毫秒" << std::endl;
        all_time.push_back(duration.count());
        while(!top_k_simTraj.empty()){
            TrajectorySimilarity res=top_k_simTraj.top();
            cout<<res.trajectoryId<<" "<<res.similarity<<endl;
            top_k_simTraj.pop();
        }
        if(test_num==100){
            break;
        }
        test_num++;
    }
    cout<<findMedian(all_time)<<endl;

    return 0;
}