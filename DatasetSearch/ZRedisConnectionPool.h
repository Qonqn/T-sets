#include"ZRedisConnection.h"
//#include"ZRedisHash.h"
#include<mutex>
#include<memory>


class ZRedisConnectionPool {

public:
    /**
     * 初始化连接池
     * @param ip    服务ip
     * @param port  服务端口
     * @param pwd   服务密码
     * @param db    库
     * @param pool_size 连接池大小 
     * @return 
     */
    static bool init(std::string ip,int port,std::string pwd,int db=0,int pool_size = 2);

    /**
     * 获取连接池的一个连接
     * @return 
     */
    static std::shared_ptr<ZRedisConnection> Get();

    /**
     * 将获取的连接返回连接池
     * @param con 
     */
    static void Back(std::shared_ptr<ZRedisConnection> &con);

    
    /**
     * 获取当前连接池可用连接资源数
     * @return 
     */
    static int size();

private:
    static std::mutex mtx_;         //资源锁，防止多线程操作连接池
    static std::vector<std::shared_ptr<ZRedisConnection>> connect_pool_;  //连接池容器

};

std::vector<std::shared_ptr<ZRedisConnection>> ZRedisConnectionPool::connect_pool_;
std::mutex ZRedisConnectionPool::mtx_;

bool ZRedisConnectionPool::init(std::string ip, int port, std::string pwd,int db, int pool_size) {
    bool flag = true;
    if(pool_size <= 0)
    {
        flag = false;
    }
    mtx_.lock();
    for (int i = 0; i < pool_size; ++i) {
        //实例化连接
        ZRedisConnection* con = new ZRedisConnection();
        if(con->Connect(ip,port,pwd,db))
        {
            //放入连接池
            connect_pool_.push_back(std::shared_ptr<ZRedisConnection>(con));
        }else{
            cout << "connect redirs :" <<ip << ":" << port << pwd << "failed";
        }
    }
    int real_pool_size = connect_pool_.size();
    mtx_.unlock();
    if(pool_size > real_pool_size)
    {
        cout << "redis pool init failed! hope pool size:" << pool_size << "real size is " << real_pool_size<<endl;
        flag = false;
    }else{
        cout << "redis pool init success! pool size:" << real_pool_size<<endl;
    }

    return flag;
}


std::shared_ptr<ZRedisConnection> ZRedisConnectionPool::Get() {

    mtx_.lock();
    if(connect_pool_.size() == 0)
    {
        throw "Zzredis pool is empty";
    }
    //从连接容器里返回一个连接
    std::shared_ptr<ZRedisConnection> tmp = connect_pool_.front();
    connect_pool_.erase(connect_pool_.begin());
    mtx_.unlock();
    return tmp;
}

void ZRedisConnectionPool::Back(std::shared_ptr<ZRedisConnection> &con) {
    mtx_.lock();
    //归还到容器
    connect_pool_.push_back(con);
    mtx_.unlock();
}

int ZRedisConnectionPool::size() {

    mtx_.lock();
    int size = connect_pool_.size();
    mtx_.unlock();
    return size;
}