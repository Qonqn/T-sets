#include"AsqtSearch.h"
#include<fstream>

using namespace std;

int main()
{
    // 文件路径，请替换为实际的文件路径
    std::string filename = "shanghai-mobike-asqt.txt";
    std::ifstream asqtfile(filename);
    
    // 构建四叉树
    AsqtNode* root = buildAsqtTree(asqtfile);
    //printAsqtTree(root);
     
    shared_ptr<ZRedisConnection> con;
    ZRedisConnectionPool::init("127.0.0.1",6379,"123456",1);
	con = ZRedisConnectionPool::Get();
    //initial_table();
    ifstream file("testcase.txt");
    string line;
    int test_num=0;
    auto start_time = std::chrono::high_resolution_clock::now();
    while (getline(file, line)) {
        // 使用 stringstream 解析每一行
        vector<pair<double,double>> query_range;
        std::istringstream iss(line);
        double num;
        std::vector<double> numbers;

        // 读取四个小数
        while (iss >> num) {
            numbers.push_back(num);
        }
        // 检查是否成功读取了四个小数
        if (numbers.size() == 4) {
            // 在这里你可以处理读取到的四个小数
            // 例如，将它们存储到数组或进行其他操作
            for (const double& value : numbers) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
            double r_minx,r_miny,r_maxx,r_maxy=0;
            r_minx=numbers[0];
            r_maxx=numbers[1];
            r_miny=numbers[2];
            r_maxy=numbers[3];
            cout<<r_minx<<" "<<r_maxx<<" "<<r_miny<<" "<<r_maxy<<endl;
            
            query_range.push_back({r_minx,r_miny});
            query_range.push_back({r_maxx,r_miny});
            query_range.push_back({r_minx,r_maxy});
            query_range.push_back({r_maxx,r_maxy});
            set<string> trajs=search_by_space(con,root,query_range);
            for(set<string>::iterator i=trajs.begin();i!=trajs.end();i++){
                cout<<*i<<" ";
            }
            cout<<endl;
            cout<<trajs.size()<<endl;
        } else {
            std::cerr << "Error reading line: " << line << std::endl;
        }
        test_num++;
        if(test_num==100){
            auto end_time = std::chrono::high_resolution_clock::now();
            // 计算程序执行时间（以毫秒为单位）
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            // 打印执行时间
            std::cout << "程序执行时间: " << duration.count() << " 毫秒" << std::endl;
            break;
        }
    }


    return 0;
}
