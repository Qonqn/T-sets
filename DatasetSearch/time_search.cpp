#include<iostream>
#include<fstream>
#include<set>
#include<map>
#include<algorithm>
#include<queue>
#include<bits/stdc++.h>
#include <chrono>
#include<ctime>
#include"ZRedisConnectionPool.h"
using namespace std;

const long long bin_len=24*3600;

// 解析时间字符串为时间戳
time_t parseTimeString(const std::string& timeStr) {
    struct tm tm = {0};
    sscanf(timeStr.c_str(), "%d-%d-%d-%d:%d:%d",
           &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
           &tm.tm_hour, &tm.tm_min, &tm.tm_sec);

    tm.tm_year -= 1900; // 年份需要减去1900
    tm.tm_mon -= 1;     // 月份从0开始

    return mktime(&tm);
}

// 计算时间点在整个数据集时间范围内的小时偏移量
int calculateHourIndex(time_t datasetStart, time_t timePoint) {
    // 计算时间点和数据集开始时间之间的秒数差
    double diffSeconds = difftime(timePoint, datasetStart);

    // 将秒数差转换为小时数
    double diffHours = diffSeconds / bin_len; 

    // 计算小时段索引（向下取整）
    int hourIndex = static_cast<int>(diffHours);

    return hourIndex;
}

// 计算时间范围在整个数据集时间范围内的小时段
void calculate_timebin(std::vector<int>& bins, time_t datasetStart, time_t datasetEnd,
                       time_t timeRangeStart, time_t timeRangeEnd) {
    
    // 计算时间范围的开始时间和结束时间分别在整个数据集时间范围的第几个时段
    int startHourIndex = calculateHourIndex(datasetStart, timeRangeStart);
    int endHourIndex = calculateHourIndex(datasetStart, timeRangeEnd);
    if(endHourIndex!=startHourIndex){
        // 将结果存储到 bins 向量中
        for (int i = startHourIndex; i <= endHourIndex; ++i) {
            bins.push_back(i);
        }
    }
    else{
        bins.push_back(startHourIndex);
    }
}

// 计算数据在其时间段内的索引值
std::string calculate_index_within_segment(std::time_t start_time, std::time_t end_time,
                                           std::time_t segment_start, std::time_t segment_end,
                                           std::string current_index) {
    if (end_time < segment_start || start_time > segment_end) {
        std::cout << "Error: Time range does not overlap with segment." << std::endl;
        return "";
    }

    // 计算当前时间段的中点
    std::time_t segment_midpoint = segment_start + (segment_end - segment_start) / 2;

    //cout<<segment_start<<" "<<segment_end<<" "<<start_time<<" "<<end_time<<endl;
    // 判断数据在时间段的前半部分或后半部分
    if (end_time < segment_midpoint) {
        current_index.push_back('0');
        return calculate_index_within_segment(start_time, end_time, segment_start, segment_midpoint,
                                              current_index);
    } else if(start_time > segment_midpoint){
        current_index.push_back('1');
        return calculate_index_within_segment(start_time, end_time, segment_midpoint, segment_end,
                                            current_index);
    }else{
        return current_index;
    }
}

// 计算给定段号对应的时间段开始时间和结束时间
pair<time_t, time_t> calculate_bin_time(int bin_num, time_t datasetStarttime) {
    // 一个时间段（段号）对应一个小时

    // 计算当前段号对应的开始时间和结束时间
    time_t bin_start_time = datasetStarttime + bin_num * bin_len;
    time_t bin_end_time = bin_start_time + bin_len;

    // 返回时间段的开始时间和结束时间
    return std::make_pair(bin_start_time, bin_end_time);
}

bool overlap(time_t tqs,time_t tqe,time_t start_time,time_t end_time){
    if (tqe <= start_time || tqs >= end_time) {
        return false;
    } else {
        return true;
    }
}

// 将时间字符串转换为时间戳
time_t stringToTimestamp(const std::string& timeStr) {
    struct tm timeInfo = {0};
    std::istringstream iss(timeStr);
    iss >> std::get_time(&timeInfo, "%Y-%m-%d-%H:%M:%S");
    return mktime(&timeInfo);
}

// 将形如 "start_time#end_time" 的时间范围字符串转换为时间戳表示的开始时间和结束时间
std::pair<time_t, time_t> parseTimeRange(const std::string& timeRangeStr) {
    // 查找分隔符 '#'
    size_t pos = timeRangeStr.find('#');
    if (pos == std::string::npos) {
        //cout<<timeRangeStr<<endl;
        throw std::invalid_argument("Invalid time range format");
    }

    // 提取开始时间和结束时间的子字符串
    std::string startTimeStr = timeRangeStr.substr(0, pos);
    std::string endTimeStr = timeRangeStr.substr(pos + 1);

    // 将开始时间和结束时间转换为时间戳
    time_t startTimeStamp = stringToTimestamp(startTimeStr);
    time_t endTimeStamp = stringToTimestamp(endTimeStr);

    return std::make_pair(startTimeStamp, endTimeStamp);
}

set<string> satisfy_trajs(shared_ptr<ZRedisConnection> con,int bin_num,string index,bool contain,time_t tqs,time_t tqe){
    set<string> keys;
    //cout<<index<<" ";s
    if(contain){
        RedisResult res;
        string cmd="keys "+to_string(bin_num)+"#*";
        con->ExecCmd(cmd,res);
        for(int i=0;i<res.vecdata.size();i++){
            keys.insert(res.vecdata[i]);
        }
    }
    else{
        for(int i=1;i<=index.size()-1;i++){
            string sub_index=index.substr(0,i);
            RedisResult res;
            string cmd="keys "+to_string(bin_num)+"#"+sub_index+"#*";
            con->ExecCmd(cmd,res);
            for(int j=0;j<res.vecdata.size();j++){
                RedisResult res1;
                string cmd="get "+res.vecdata[j];
                con->ExecCmd(cmd,res1);
                pair<time_t,time_t> timeRange=parseTimeRange(res1.strdata);
                if(overlap(tqs,tqe,timeRange.first,timeRange.second)){
                    keys.insert(res.vecdata[j]);
                }
            }
        }
        RedisResult res2;
        string cmd="keys "+to_string(bin_num)+"#"+index+"*";
        con->ExecCmd(cmd,res2);
        for(int j=0;j<res2.vecdata.size();j++){
            RedisResult res3;
            string cmd="get "+res2.vecdata[j];
            con->ExecCmd(cmd,res3);
            pair<time_t,time_t> timeRange=parseTimeRange(res3.strdata);
            if(overlap(tqs,tqe,timeRange.first,timeRange.second)){
                keys.insert(res2.vecdata[j]);
            }
        }
    }
    return keys;
}


set<string> search_by_time(shared_ptr<ZRedisConnection> con,string start_time,string end_time,string tqs,string tqe){
    set<string> res;
    vector<int> bins;
    // 解析时间字符串为时间戳
    time_t datasetStart = parseTimeString(start_time);
    time_t datasetEnd = parseTimeString(end_time);
    time_t timeRangeStart = parseTimeString(tqs);
    time_t timeRangeEnd = parseTimeString(tqe);
    calculate_timebin(bins,datasetStart,datasetEnd,timeRangeStart,timeRangeEnd);
    int bin_num=bins.size();
    for(int i=0;i<bin_num;i++){
        pair<time_t,time_t> segment_time=calculate_bin_time(bins[i],datasetStart);
        string index="1";
        index=calculate_index_within_segment(timeRangeStart,timeRangeEnd,segment_time.first,segment_time.second,index);
        cout<<index<<endl;
        cout<<bins[i]<<" "<<endl;
        
        if(i==0||i==bin_num-1){
            set<string> sub_res=satisfy_trajs(con,bins[i],index,false,timeRangeStart,timeRangeEnd);
            for (auto elem : sub_res) {
                res.insert(elem);
            }
        }
        else{
            set<string> sub_res=satisfy_trajs(con,bins[i],index,true,timeRangeStart,timeRangeEnd);
            for (auto elem : sub_res) {
                res.insert(elem);
            }
        }
    }

    return res;
}

double findMedian(std::vector<int64_t>& nums) {
    std::sort(nums.begin(), nums.end());
    
    int n = nums.size();
    if (n % 2 != 0) {
        // 如果vector的大小是奇数，中位数就是中间的元素
        return nums[n / 2];
    } else {
        // 如果vector的大小是偶数，中位数是中间两个元素的平均值
        return (nums[(n - 1) / 2] + nums[n / 2]) / 2.0;
    }
}

int main(){
    shared_ptr<ZRedisConnection> con;
    ZRedisConnectionPool::init("127.0.0.1",6379,"123456",4);
	con = ZRedisConnectionPool::Get();
    string overall_start_time="2013-07-01-00:00:53";
    string overall_end_time="2014-04-02-22:10:30";
    
    ifstream file("time_ranges_updated.csv");
    string line;
    int test_num=0;
    vector<int64_t> all_time;
    while (getline(file, line)) {
        if(test_num<50){
            test_num++;
            continue;
        }
        cout<<line<<endl;
        string tqs,tqe;
        size_t pos = line.find(',');

        if (pos != string::npos) {
        // 提取逗号之前的部分赋给 tqs
            tqs = line.substr(0, pos);
        
        // 提取逗号之后的部分赋给 tqe
            tqe = line.substr(pos + 1);
        
        }
        auto start_time = std::chrono::high_resolution_clock::now();
        set<string> results=search_by_time(con,overall_start_time,overall_end_time,tqs,tqe);
        auto end_time = std::chrono::high_resolution_clock::now();
        cout<<"size of results is "<<results.size()<<endl;
        for (auto elem : results) {
            cout<<elem<<" ";
        }
        // 计算程序执行时间（以毫秒为单位）
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        // 打印执行时间
        std::cout << "程序执行时间: " << duration.count() << " 毫秒" << std::endl;
        all_time.push_back(duration.count());
        if(test_num==60){
            break;
        }
        test_num++;
    }
    cout<<findMedian(all_time)<<endl;
    
}