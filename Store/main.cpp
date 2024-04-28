#include"Datasets.h"

int main()
{
    Datasets sys=Datasets();
    auto start_time = std::chrono::high_resolution_clock::now();
    sys.Add("/home/njucs/redis/porto.csv","porto_output.csv",sys.con);
    auto end_time = std::chrono::high_resolution_clock::now();
    // 计算程序执行时间（以毫秒为单位）
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    // 打印执行时间
    std::cout << "程序执行时间: " << duration.count() << " 毫秒" << std::endl;
    return 0;
}