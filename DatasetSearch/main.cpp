#include<iostream>
#include"ZRedisConnectionPool.h"
using namespace std;

int main(){
    ZRedisConnectionPool::init("127.0.0.1",6379,"123456",2);
	shared_ptr<ZRedisConnection> con = ZRedisConnectionPool::Get();
    string cmd;
    getline(cin,cmd);
    RedisResult res;
    RedisStatus status = con->ExecCmd(cmd,res);
    for(int i=0;i<res.vecdata.size();i++){
        cout<<res.vecdata[i]<<" ";
    }
    cout<<endl;
    return 0;
}