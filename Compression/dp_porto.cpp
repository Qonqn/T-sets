#include<iostream>
#include<vector>
#include<string>
#include<sstream>
#include<cmath>
#include<iomanip>
#include<algorithm>
#include <chrono>
#include<fstream>
#include<set>
#include<unordered_set>

using namespace std;

struct Point {
    double x;
    double y;

    Point(double x, double y) : x(x), y(y) {}
    bool operator==(const Point &p) {
        return (x==p.x && y==p.y);
    }
    
};


double distance(const Point& point1, const Point& point2) {
    double dx = point2.x - point1.x;
    double dy = point2.y - point1.y;
    return std::sqrt(dx * dx + dy * dy);
}

// 计算斜率
double calculateSlope(const Point& p1, const Point& p2) {
    if (p1.x == p2.x) {
        // 垂直线的斜率为无穷大，返回一个足够大的值表示无穷大
        return std::numeric_limits<double>::infinity();
    }
    else {
        return (p2.y - p1.y) / (p2.x - p1.x);
    }
}

// 计算垂直距离
double calculatePerpendicularDistance(const Point& p1, const Point& p2, const Point& p3) {
    double slope = calculateSlope(p2, p3);

    if (std::isinf(slope)) {
        // 如果直线是垂直的，直接计算x轴方向的距离
        return fabs(p1.x - p2.x);
    }

    // 直线的方程式为 y = mx + b，计算b
    double intercept = p2.y - slope * p2.x;

    // 直线的方程式为 y = mx + b，将垂直距离带入方程并计算
    double perpendicularDistance = fabs(slope * p1.x - p1.y + intercept) / sqrt(slope * slope + 1);

    return perpendicularDistance;
}

double innerProduct(const Point& v1, const Point& v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

double psed(const Point& p1, const Point& p2, const Point& p3) {
    Point v1(p1.x - p2.x, p1.y - p2.y);
    Point v2(p3.x - p2.x, p3.y - p2.y);
    Point v3(p3.x - p1.x, p3.y - p1.y);
    if (innerProduct(v1, v2) < 0 || innerProduct(v2, v3) < 0) {
        return std::min(distance(p1, p2), distance(p1, p3));
    }
    return calculatePerpendicularDistance(p1, p2, p3);
}

vector<Point> split_trajectory_points(const std::string& trajectory_str) {
    std::vector<Point> trajectory_points;
    std::istringstream iss(trajectory_str);
    std::string point_str;

    while (getline(iss, point_str, '#')) {
        std::istringstream point_iss(point_str);
        std::string coordinate_str;
        double x, y;

        if (std::getline(point_iss, coordinate_str, ',')) {
            //cout<<coordinate_str<<endl;
            x = std::stod(coordinate_str);
        }

        if (std::getline(point_iss, coordinate_str, ',')) {
            //cout<<coordinate_str<<endl;
            y = stod(coordinate_str);
        }

        trajectory_points.emplace_back(x, y);
    }
    return trajectory_points;
}

// Douglas-Peucker递归算法
void douglasPeucker(const std::vector<Point>& points, int start, int end, double epsilon, std::vector<Point>& result) {
    double maxDistance = 0;
    int index = 0;

    // 寻找区间内点到直线的最大垂直距离
    for (int i = start + 1; i < end; ++i) {
        double distance = psed(points[i], points[start], points[end]);
        if (distance > maxDistance) {
            maxDistance = distance;
            index = i;
        }
    }
    if (abs(maxDistance - 0.001863)<1e-8) {
        cout << "here" << endl;
    }
    // 如果最大距离大于阈值，则将该点加入结果，并分别对左右两个子区间递归进行抽稀
    if (maxDistance > epsilon) {
        douglasPeucker(points, start, index, epsilon, result);
        result.push_back(points[index]);
        douglasPeucker(points, index, end, epsilon, result);
        
    }  
}

// Douglas-Peucker算法的入口函数
std::vector<Point> douglasPeucker(const std::vector<Point>& points, double epsilon) {
    std::vector<Point> result;
    result.push_back(points[0]);
    douglasPeucker(points, 0, points.size() - 1, epsilon, result);
    result.push_back(points[points.size() - 1]);
    return result;
}

bool hasDuplicates(const std::vector<Point>& vec) {
    for (size_t i = 0; i < vec.size(); ++i) {
        for (size_t j = i + 1; j < vec.size(); ++j) {
            if (vec[i].x == vec[j].x &&vec[i].y==vec[j].y) {
                return true;  // 发现重复元素
            }
        }
    }
    return false;  // 未发现重复元素
}

int main() {
    string fname = "/home/njucs/redis/porto.csv";
    //以读入方式打开文件
    ifstream csv_data(fname, ios::in);

    ofstream dataFile;
	dataFile.open("porto_dp_30000_0.008.csv", ios::out | ios::trunc);
 
    if (!csv_data.is_open())
    {
        cout << "Error: opening file fail" << endl;
        exit(1);
    }
    else {
        string line;
    
        vector<string> words; //声明一个字符串向量
        string word;
        // ------------读取数据-----------------
        // 读取标题行
        getline(csv_data, line);
    
        int64_t time=0;
        int64_t error=0;
        double compress_rate=0;
        int num=0;
        istringstream sin;
        // 按行读取数据
        while (getline(csv_data, line))
        {
            // 清空vector及字符串流,只存当前行的数据
            words.clear();
            sin.clear();
    
            sin.str(line);
            //将字符串流sin中的字符读到字符串数组words中，以逗号为分隔符
            while (getline(sin, word, '"'))
            {
                words.push_back(word); //将每一格中的数据逐个push
            }
            string traj_id;
            sin.clear();
            sin.str(words[0]);
            getline(sin,traj_id,','); 
            string traj=words[1];
            //cout<<traj<<endl;
            vector<Point> inputPoints = split_trajectory_points(traj);
            // 调用Douglas-Peucker算法，抽稀曲线
            double epsilon = 0.008;
            auto start = std::chrono::high_resolution_clock::now();
            std::vector<Point> simplifiedPoints = douglasPeucker(inputPoints, epsilon);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            //std::cout << "程序执行时间: " << duration.count() << " 微秒" << std::endl;
            time+=duration.count();
            // 输出抽稀后的点集
            //std::cout << "抽稀后的点集：" << std::endl;
            for (const Point& p : simplifiedPoints) {
                //std::cout << "(" << p.x << ", " << p.y << ") ";
            }
            //std::cout << std::endl;
            
            Point start_point = simplifiedPoints[0];
            Point end_point=simplifiedPoints[1];
            int start_pos = 0;
            bool find_end = 0;
            bool flag=0;
            for (int i = 1; i < inputPoints.size(); i++) {
                auto pos = find(simplifiedPoints.begin(), simplifiedPoints.end(), inputPoints[i]);
                if (pos != simplifiedPoints.end()) {
                    end_point = inputPoints[i];
                    for (int j = start_pos; j < i; j++) {
                        double psed_val=psed(inputPoints[j], start_point, end_point);
                        if(psed_val>epsilon){
                            //cout << j+1 << ":" << psed_val <<endl;
                            flag=1;
                        }
                    }
                    start_point = inputPoints[i];
                    start_pos = i;
                }
            }
            if(flag&&!hasDuplicates(inputPoints)){
                error++;
                //cout<<traj<<endl;
            }
            else{
                dataFile<<traj_id<<","<<"\"";
                dataFile<< setprecision(6) << fixed <<simplifiedPoints[0].x<<","<<setprecision(6) << fixed <<simplifiedPoints[0].y;
                for(int i=1;i<simplifiedPoints.size();i++){
                    dataFile<<"#"<<setprecision(6) << fixed <<simplifiedPoints[i].x<<","<<setprecision(6) << fixed <<simplifiedPoints[i].y;
                }
                dataFile<<"\""<<endl;
            }
            if(inputPoints.size()<simplifiedPoints.size()){
                cout<<traj_id<<endl;
            }
            //cout<<inputPoints.size()<<" "<<simplifiedPoints.size()<<endl;
            compress_rate+=inputPoints.size()/simplifiedPoints.size();
            num++;
            if(num==30000)
                break;
        }
        cout<<"all run time is "<< std::setprecision(3) << std::fixed << double(time)/1000<<"ms"<<endl;
        //cout<<"all error num is "<<error<<endl;
        cout<<"average compress rate is "<<compress_rate/num<<endl;
        csv_data.close();
        }

    return 0;
}