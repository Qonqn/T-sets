#include"Datasets.h"

int main()
{
    Datasets sys=Datasets();
    sys.Add("/home/njucs/redis/mobike_shanghai_sample_updated.csv",sys.con);
    return 0;
}