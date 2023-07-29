//ZRedisHash.h
#include"ZRedisConnection.h"
#include<map>

class ZRedisHash :public ZRedisBase
{
public:
    ZRedisHash() = default;
    ~ZRedisHash() = default;

    template <typename T>
    RedisStatus Hmset(std::string key,std::map<std::string,T> value){
        stringstream ss;
        ss << "HMSET " << key << " ";

        typename std::map<std::string,T>::iterator it = value.begin();
        for(it;it != value.end();it++)
        {
            ss << it->first << " " << it->second << " ";
        }
        std::string cmd = ss.str();
        RedisResult res;
        RedisStatus s = ZRedisConnection::ExecCmd(cmd.c_str(),res);
        return s;
    }


    template <typename T>
    RedisStatus Hdel(std::string key,std::vector<T> value){
        std::stringstream ss;
        ss << "HDEL " << key << " ";

        typename std::vector<T>::iterator it = value.begin();
        for(it;it != value.end();it++)
        {
            ss << *it << " " << *it << " ";
        }
        std::string cmd = ss.str();
        RedisResult res;
        RedisStatus s = ZRedisConnection::ExecCmd(cmd.c_str(),res);
        return s;
    }

    template <typename T>
    RedisStatus Hset(std::string key, std::string field,T t) {
        std::stringstream ss;
        ss << "HSET " << key << " " << field << " " << t;

        std::string cmd = ss.str();
        RedisResult res;
        RedisStatus s = ZRedisConnection::ExecCmd(cmd.c_str(),res);
        return s;
    }