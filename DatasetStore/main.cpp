#include"Datasets.h"

int main()
{
    Datasets sys=Datasets();
    sys.Add("/home/njucs/redis/test.csv",sys.con);
    
    return 0;
}