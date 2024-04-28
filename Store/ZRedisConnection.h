#include"ZRedisBase.h"
#include<sstream>
//redis 的执行状态
enum RedisStatus
{
    M_REDIS_OK = 0, //执行成功
    M_CONNECT_FAIL = -1, //连接redis失败
    M_CONTEXT_ERROR = -2, //RedisContext返回错误
    M_REPLY_ERROR = -3, //redisReply错误
    M_EXE_COMMAND_ERROR = -4, //redis命令执行错误
    M_NIL_ERROR = -5 //nil
};

//type pf redisreply
enum Redisreply_type{
    redis_reply_null=0,
    redis_reply_string=1,
    redis_reply_integer=2,
    redis_reply_array=3,
    redis_reply_invalid=-1
};

typedef struct st_redisResult
{
    int type;
    size_t inter;
    std::string strdata;
    std::vector<std::string> vecdata;
}RedisResult;

class ZRedisConnection
{

public:
    ZRedisConnection();
    ~ZRedisConnection();
    /**
     * 连接服务
     * @param addr  地址
     * @param port  端口
     * @param pwd   密码
     * @param db    数据库
     * @return      是否成功
     */
    bool Connect(const std::string &addr, int port, const std::string &pwd = "",int db=0);
    
    
    /**
     * 是否连接
     * @return 
     */
    bool IsConnect();
    
    /**
     * 解析结果
     * @param result 
     * @param reply 
     * @return 
     */
    RedisStatus ParseReplay(RedisResult &result, redisReply *reply = NULL);
    
    /**
     * 执行命令
     * @param cmd 
     * @param res 
     * @return 
     */
    RedisStatus ExecCmd(const std::string &cmd, RedisResult &res);
private:
    /**
     * 使用密码
     * @param psw 
     * @return 
     */
    int ConnectAuth(const std::string &psw);
    /**
     * 释放命令执行结果的内存
     * @param reply 
     * @return 
     */
    int FreeRedisReply(redisReply *reply);
    
    /**
     * 解析错误
     * @param reply 
     * @return 
     */
    RedisStatus CheckErr(redisReply *reply = NULL);
    
    /**
     * 选择数据库
     * @param db 
     * @return 
     */
    int SelectDb(const int &db);

public:

    redisContext* pm_rct; //redis结构体
    redisReply* pm_rr; //返回结构体

    std::string addr_; //IP地址
    int port_; //端口号
    std::string pwd_; //密码

private:
    bool is_connect_ = false;
};

ZRedisConnection::ZRedisConnection(){
}


bool ZRedisConnection::Connect(const std::string &addr, int port, const std::string &pwd,int db) {
    addr_ = addr;
    port_ = port;
    pwd_ = pwd;
    pm_rct = redisConnect(addr_.c_str(), port_);
    //struct of rediscontext
    if (pm_rct->err)
    {
        is_connect_ = false;
    }
    if (!pwd_.empty())
    {
        ConnectAuth(pwd_);
    }
    SelectDb(db);

    is_connect_ = true;
    return is_connect_;
}

/*
使用密码登录
psw：登录密码
成功返回M_REDIS_OK，失败返回<0
*/
int ZRedisConnection::ConnectAuth(const std::string &psw)
{
    std::string cmd = "auth " + psw;
    //pm_rr = (redisReply*)redisCommand(pm_rct, cmd.c_str());
    return 0;
}


int ZRedisConnection::SelectDb(const int &db)
{
    std::string cmd = "select " + std::to_string(db);
    pm_rr = (redisReply*)redisCommand(pm_rct, cmd.c_str());
    return 0;
}

RedisStatus ZRedisConnection::ExecCmd(const std::string &cmd,RedisResult &res)
{
    pm_rr = (redisReply*)redisCommand(pm_rct, cmd.c_str());
    //ddprintf("pm__rr's type is %d\n",pm_rr->type);
    /*
    REDIS_REPLY_STRING  ： 1 
　　 REDIS_REPLY_ARRAY ： 2
　　 REDIS_REPLY_INTEGER ：3 
　　 REDIS_REPLY_NIL  ： 4
　　 REDIS_REPLY_STATUS ： 5
　　 REDIS_REPLY_ERROR ： 6
    */
    RedisStatus s = ParseReplay(res);
}

ZRedisConnection::~ZRedisConnection()
{
    redisFree(pm_rct);
}

bool ZRedisConnection::IsConnect()
{
    return is_connect_;
}

RedisStatus ZRedisConnection::CheckErr(redisReply *reply)
{
    if(reply == NULL)
    {
        reply = pm_rr;
    }
    if (pm_rct->err)
    {
//        error_msg = pm_rct->errstr;
        ZRedisBase::SetError(pm_rct->errstr);
        return M_CONTEXT_ERROR;
    }
    if (reply == NULL)
    {
//        error_msg = "auth redisReply is NULL";
        ZRedisBase::SetError("auth redisReply is NULL");
        return M_REPLY_ERROR;
    }
    return M_REDIS_OK;
}

RedisStatus ZRedisConnection::ParseReplay(RedisResult &result,redisReply *reply)
{
    if(reply == NULL)
    {
        reply = pm_rr;
    }
    RedisStatus s = CheckErr(reply);
    if(s != M_REDIS_OK)
    {
        FreeRedisReply(reply);
        return s;
    }

    switch(reply->type)
    {
        case REDIS_REPLY_STATUS:
            s = M_REDIS_OK;
            result.type = reply->type;
            result.strdata = reply->str;
            break;
        case REDIS_REPLY_ERROR:
            s = M_REPLY_ERROR;
            result.type = redis_reply_null;
            result.strdata =reply->str;
            ZRedisBase::SetError(reply->str);
            break;
        case REDIS_REPLY_STRING:
            s = M_REDIS_OK;
            result.type = redis_reply_string;
            result.strdata = reply->str;
            break;
        case REDIS_REPLY_INTEGER:
            s = M_REDIS_OK;
            result.type = redis_reply_integer;
            result.inter = reply->integer;
            break;
        case REDIS_REPLY_ARRAY:
            s = M_REDIS_OK;
            result.type = redis_reply_array;
            for (int i = 0; i < reply->elements; i ++)
            {
                if(reply->element[i]->type == REDIS_REPLY_NIL)
                {
                    result.vecdata.push_back("NIL");
                }else{
                    if(reply->element[i]->str==NULL)
                    {
                        result.vecdata.push_back("null");
                    }else{
                        result.vecdata.push_back(reply->element[i]->str);
                    }
                }

            }
            break;
        case REDIS_REPLY_NIL:
            s = M_NIL_ERROR;
            result.type = redis_reply_null;
            result.strdata = "REDIS_REPLY_NIL";
            if(reply->str == NULL)
            {
                ZRedisBase::SetError("REDIS_REPLY_NIL");
            }else{
                ZRedisBase::SetError(reply->str);
            }
            break;
        default:
            s = M_REPLY_ERROR;
            result.type = redis_reply_invalid;
            break;
    }

    FreeRedisReply(reply);
    return s;
}

int ZRedisConnection::FreeRedisReply(redisReply *reply)
{
    if (reply)
    {
        freeReplyObject(reply);
        reply = NULL;
    }
    return 0;
}
