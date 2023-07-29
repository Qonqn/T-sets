#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <vector>
#include<map>
 
#include <hiredis/hiredis.h>
using namespace std;



class ZRedisBase
{
public:
    ZRedisBase();
    ~ZRedisBase();

    virtual std::string GetErr();

    static void SetError(std::string error);

private:
    static std::string err_msg_;
};

std::string ZRedisBase::err_msg_;
ZRedisBase::ZRedisBase()
{
}

ZRedisBase::~ZRedisBase()
{
}


std::string ZRedisBase::GetErr(){
    return err_msg_;
}

void ZRedisBase::SetError(std::string error) {
    err_msg_ = error;
}