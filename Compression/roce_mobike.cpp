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
using namespace std; 

double distance(const std::pair<double, double>& point1, const std::pair<double, double>& point2) {
    double dx = point2.first - point1.first;
    double dy = point2.second - point1.second;
    return std::sqrt(dx * dx + dy * dy);
}

// 计算斜率
double calculateSlope(const std::pair<double, double>& p1, const std::pair<double, double>& p2) {
    if (p1.first == p2.first) {
        // 垂直线的斜率为无穷大，返回一个足够大的值表示无穷大
        return std::numeric_limits<double>::infinity();
    }
    else {
        return (p2.second - p1.second) / (p2.first - p1.first);
    }
}

// 计算垂直距离
double calculatePerpendicularDistance(const std::pair<double, double>& p1, const std::pair<double, double>& p2, const std::pair<double, double>& p3) {
    double slope = calculateSlope(p2, p3);

    if (std::isinf(slope)) {
        // 如果直线是垂直的，直接计算x轴方向的距离
        return fabs(p1.first - p2.first);
    }

    // 直线的方程式为 y = mx + b，计算b
    double intercept = p2.second - slope * p2.first;

    // 直线的方程式为 y = mx + b，将垂直距离带入方程并计算
    double perpendicularDistance = fabs(slope * p1.first - p1.second + intercept) / sqrt(slope * slope + 1);

    return perpendicularDistance;
}

double innerProduct(const std::pair<double, double>& v1, const std::pair<double, double>& v2) {
    return v1.first * v2.first + v1.second * v2.second;
}

double psed(const std::pair<double, double>& p1, const std::pair<double, double>& p2, const std::pair<double, double>& p3) {
    std::pair<double, double> v1 = std::make_pair(p1.first - p2.first, p1.second - p2.second);
    std::pair<double, double> v2 = std::make_pair(p3.first - p2.first, p3.second - p2.second);
    std::pair<double, double> v3 = std::make_pair(p3.first - p1.first, p3.second - p1.second);
    if (innerProduct(v1,v2)<0||innerProduct(v2,v3)<0) {
        return std::min(distance(p1, p2), distance(p1, p3));
    }
    return calculatePerpendicularDistance(p1, p2, p3);
}


class CircularRegion {
public:
    std::pair<double, double> center;  // Center of the circular region
    double radius;                   // Radius of the circular region

    // Constructor to initialize the circular region
    CircularRegion(double centerX, double centerY, double r) : center(centerX, centerY), radius(r) {}

    // Function to check if a point is inside the circular region
    bool isInsideCircularRegion(double x, double y) const {
        // Calculate distance from the center of the circular region
        double distance = std::sqrt(std::pow(x - center.first, 2) + std::pow(y - center.second, 2));

        // Check if the point is inside the circular region
        return (distance <= radius);
    }
};

class TangentRegion {
    
public:
    pair<double, double> point;
    std::pair<double, double> tangent1;  // First tangent point
    std::pair<double, double> tangent2;  // Second tangent point
    // Constructor to initialize the tangent region with two tangent points
    TangentRegion(const std::pair<double, double>& p, const std::pair<double, double>& t1, const std::pair<double, double>& t2)
        : point(p),tangent1(t1), tangent2(t2) {}

    // Function to check if a point is inside the tangent region
    bool isInsideTangentRegion(double x, double y) const {
        // Check if the point is inside the tangent region
        const double epsilon = 1e-8;
        if (abs(x-point.first)<epsilon||abs(tangent1.first-point.first)<epsilon||abs(tangent2.first-point.first)<epsilon) {
            double t = (x - point.first) / (y - point.second);
            double t1 =  (tangent1.first - point.first)/ (tangent1.second - point.second);
            double t2 = (tangent2.first - point.first) / (tangent2.second - point.second);
            if (t1 < t2 && t >= t1 && t <= t2) {
                return true;
            }
            if (t1 > t2 && t <= t1 && t >= t2) {
                return true;
            }
            return false;
        }
        double k = (y - point.second) / (x - point.first);
        double k1 = (tangent1.second - point.second) / (tangent1.first - point.first);
        double k2 = (tangent2.second - point.second) / (tangent2.first - point.first);
        if ((tangent1.first-point.first)*(tangent2.first-point.first) >= 0) {
            if ((tangent1.first - point.first) * (x - point.first) < 0) {
                return false;
            }
            if (k1 < k2 && k >= k1 && k <= k2) {
                return true;
            }
            if (k1 > k2 && k <= k1 && k >= k2) {
                return true;
            }
            return false;
        }
        else {
            if ((tangent1.second - point.second) * (y - point.second) < 0) {
                return false;
            }
            if (k1 >= 0&&(k>=k1||k<=k2)) {
                return true;
            }
            if (k1 <= 0 && (k <= k1 && k >= k2)) {
                return true;
            }
            return false;
        }
    }

    static TangentRegion computeOverlap(const TangentRegion& region1, const TangentRegion& region2) {
        // Ensure that the two regions have the same point
        if (region1.point != region2.point) {
            // Handle error or return a default TangentRegion
            cout << "region1.point != region2.point" << endl;
            return TangentRegion(std::make_pair(0.0, 0.0), std::make_pair(0.0, 0.0), std::make_pair(0.0, 0.0));
        }

        // Find the overlapping tangents
        double x = region1.point.first;
        double y = region1.point.second;

        double k1 = (region1.tangent1.second - y) / (region1.tangent1.first - x);
        double k2 = (region1.tangent2.second - y) / (region1.tangent2.first - x);
        double k3 = (region2.tangent1.second - y) / (region2.tangent1.first - x);
        double k4 = (region2.tangent2.second - y) / (region2.tangent2.first - x);

        double numbers[] = { k1, k2, k3, k4 };
        int size = sizeof(numbers) / sizeof(numbers[0]);

        // 对数组进行降序排序
        std::sort(numbers, numbers + size, std::greater<double>());
        //cout<<region1.tangent1.first - x<<" "<<region1.tangent2.first - x<<" "<<region2.tangent1.first - x<<" "<<region2.tangent2.first - x<<endl;
        vector<pair<double, double>> tangents;
        if ((region1.tangent1.first - x) * (region1.tangent2.first - x) >= 0&& (region2.tangent1.first - x) * (region2.tangent2.first - x) >= 0) {
            if (k1 == numbers[1] || k1 == numbers[2]) {
                tangents.push_back(region1.tangent1);
            }
            if (k2 == numbers[1] || k2 == numbers[2]) {
                tangents.push_back(region1.tangent2);
            }
            if (k3 == numbers[1] || k3 == numbers[2]) {
                tangents.push_back(region2.tangent1);
            }
            if (k4 == numbers[1] || k4 == numbers[2]) {
                tangents.push_back(region2.tangent2);
            }
        }
        else if ((region1.tangent1.first - x) * (region1.tangent2.first - x) <= 0 && (region2.tangent1.first - x) * (region2.tangent2.first - x) <= 0) {
            if (k1 == numbers[0] || k1 == numbers[3]) {
                tangents.push_back(region1.tangent1);
            }
            if (k2 == numbers[0] || k2 == numbers[3]) {
                tangents.push_back(region1.tangent2);
            }
            if (k3 == numbers[0] || k3 == numbers[3]) {
                tangents.push_back(region2.tangent1);
            }
            if (k4 == numbers[0] || k4 == numbers[3]) {
                tangents.push_back(region2.tangent2);
            }
        }
        else {
            bool flag=0;
            int pos_num = 0;
            if (region1.tangent1.first - x >= 0)
                pos_num++;
            if (region1.tangent2.first - x >= 0)
                pos_num++;
            if (region2.tangent1.first - x >= 0)
                pos_num++;
            if (region2.tangent2.first - x >= 0)
                pos_num++;
            if(pos_num==1){
                if (region1.tangent1.first - x >= 0&&k1>=0)
                    flag=1;
                if (region1.tangent2.first - x >= 0&&k2>=0)
                    flag=1;
                if (region2.tangent1.first - x >= 0&&k3>=0)
                    flag=1;
                if (region2.tangent2.first - x >= 0&&k4>=0)
                    flag=1;
            }
            if(pos_num==3){
                if (region1.tangent1.first - x < 0&&k1>=0)
                    flag=1;
                if (region1.tangent2.first - x < 0&&k2>=0)
                    flag=1;
                if (region2.tangent1.first - x < 0&&k3>=0)
                    flag=1;
                if (region2.tangent2.first - x < 0&&k4>=0)
                    flag=1;
            }
            if (pos_num == 0 || pos_num == 4||pos_num==2) {
                cout << "pos_num is "<<pos_num  << endl;
            }
            if (!flag) {
                if (k1 == numbers[0] || k1 == numbers[1]) {
                    tangents.push_back(region1.tangent1);
                }
                if (k2 == numbers[0] || k2 == numbers[1]) {
                    tangents.push_back(region1.tangent2);
                }
                if (k3 == numbers[0] || k3 == numbers[1]) {
                    tangents.push_back(region2.tangent1);
                }
                if (k4 == numbers[0] || k4 == numbers[1]) {
                    tangents.push_back(region2.tangent2);
                }
            }
            if (flag) {
                if (k1 == numbers[2] || k1 == numbers[3]) {
                    tangents.push_back(region1.tangent1);
                }
                if (k2 == numbers[2] || k2 == numbers[3]) {
                    tangents.push_back(region1.tangent2);
                }
                if (k3 == numbers[2] || k3 == numbers[3]) {
                    tangents.push_back(region2.tangent1);
                }
                if (k4 == numbers[2] || k4 == numbers[3]) {
                    tangents.push_back(region2.tangent2);
                }
            }
        }
        
        //cout << "切点个数为 "<<tangents.size() << endl;
        TangentRegion overlapRegion(region1.point, tangents[0], tangents[1]);

        return overlapRegion;
    }
};

class CombinedRegion {
private:
    TangentRegion tangentRegion;  // Tangent region excluding circular region
    CircularRegion circularRegion;  // Circular region to be excluded

public:
    // Constructor to initialize the combined region
    CombinedRegion(const std::pair<double, double>& p1, const std::pair<double, double>& p2, double r)
        : tangentRegion(p1,getTangent1(p1, p2, r), getTangent2(p1, p2, r)),
        circularRegion(p1.first, p1.second, distance(p1,p2)) {}

    CombinedRegion(TangentRegion intersectionTangentRegion,CircularRegion intersectionCircularRegion)
        : tangentRegion(intersectionTangentRegion),
        circularRegion(intersectionCircularRegion) {}

    void reconstruct(const std::pair<double, double>& p1, const std::pair<double, double>& p2, double r) {
        TangentRegion newtangentRegion(p1, getTangent1(p1, p2, r), getTangent2(p1, p2, r));
        tangentRegion = newtangentRegion;
        //cout << distance(p2, newtangentRegion.tangent1) << " " << distance(p2, newtangentRegion.tangent2) << endl;
        double d = (newtangentRegion.tangent1.first - p1.first)*(p2.first - p1.first) + (newtangentRegion.tangent1.second - p1.second)*(p2.second - p1.second);
        //cout << d << endl;
        CircularRegion  newcircularRegion(p1.first, p1.second, distance(p1,p2));
        circularRegion = newcircularRegion;
    }

    // Function to check if a point is inside the combined region
    bool isInsideCombinedRegion(double x, double y) const {
        // Check if the point is inside the combined region
        return tangentRegion.isInsideTangentRegion(x, y) && !circularRegion.isInsideCircularRegion(x, y);
    }

    // Function to get the intersection region with another CombinedRegion
    CombinedRegion getIntersection(const CombinedRegion& other) const {
        // Combine the tangent regions
        TangentRegion intersectionTangentRegion = tangentRegion.computeOverlap(tangentRegion,other.tangentRegion);

        // Combine the circular regions
        CircularRegion intersectionCircularRegion(circularRegion.center.first,circularRegion.center.second,
            std::max(circularRegion.radius, other.circularRegion.radius));

        // Return the intersection region
        return CombinedRegion(intersectionTangentRegion, intersectionCircularRegion);
    }

private:
    // Helper function to calculate the first tangent point
    static std::pair<double, double> getTangent1(const std::pair<double, double>& p1,
        const std::pair<double, double>& p2, double r) {
        pair<double, double> E;
        E.first = p1.first - p2.first;
        E.second = p1.second - p2.second;

        double t = r / sqrt(E.first * E.first + E.second * E.second);
        pair<double, double> F;
        F.first = E.first * t;
        F.second = E.second * t;
        double a = acos(t);
        pair<double, double> G;
        G.first = F.first * cos(a) - F.second * sin(a);
        G.second = F.first * sin(a) + F.second * cos(a);

        return make_pair(G.first + p2.first, G.second + p2.second);
    }

    // Helper function to calculate the second tangent point
    static std::pair<double, double> getTangent2(const std::pair<double, double>& p1,
        const std::pair<double, double>& p2, double r) {
        pair<double, double> E;
        E.first = p1.first - p2.first;
        E.second = p1.second - p2.second;

        double t = r / sqrt(E.first * E.first + E.second * E.second);
        pair<double, double> F;
        F.first = E.first * t;
        F.second = E.second * t;
        double a = -acos(t);
        pair<double, double> G;
        G.first = F.first * cos(a) - F.second * sin(a);
        G.second = F.first * sin(a) + F.second * cos(a);

        return make_pair(G.first + p2.first, G.second + p2.second);
    }
};

vector<std::pair<double, double>> split_trajectory_points(const std::string& trajectory_str) {
    std::vector<std::pair<double, double>> trajectory_points;
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

vector<std::pair<double, double>> trajectory_compress(vector<std::pair<double, double>> traj,double threshold,double &rate) {
    //cout << "最初轨迹长度为 " << traj.size() << endl;
    vector<std::pair<double, double>> compressed_traj;
    compressed_traj.push_back(traj[0]);
    for (int i = 0; i < traj.size(); i++) {
        pair<double, double> start_point = traj[i];
        bool flag = 0;
        CombinedRegion candidateRegion(start_point, start_point, threshold);
        i = i + 1;
        while (i < traj.size() && distance(start_point, traj[i]) <= threshold)
            i++;
        if (i < traj.size() && !flag) {
            candidateRegion.reconstruct(start_point,traj[i],threshold);
            flag = 1;
            i++;
        }
        while (i < traj.size()&&(candidateRegion.isInsideCombinedRegion(traj[i].first,traj[i].second))) {
            CombinedRegion newRegion(start_point, traj[i], threshold);
            candidateRegion = candidateRegion.getIntersection(newRegion);
            i++;
        }
        i = i - 1;
        compressed_traj.push_back(traj[i]);
        if (i == traj.size() - 1) {
            break;
        }
        i = i - 1;
    }
    double compress_rate = double(traj.size())/double(compressed_traj.size());
    //cout << "压缩轨迹长度为 " << compressed_traj.size() << " " << "压缩比为 " << compress_rate << endl;
    rate+=compress_rate;
    return compressed_traj;
}

bool hasDuplicates(const std::vector<pair<double,double>>& vec) {
    for (size_t i = 0; i < vec.size(); ++i) {
        for (size_t j = i + 1; j < vec.size(); ++j) {
            if (vec[i].first == vec[j].first &&vec[i].second==vec[j].second) {
                return true;  // 发现重复元素
            }
        }
    }
    return false;  // 未发现重复元素
}

int main() {
     
    string fname = "/home/njucs/redis/mobike_shanghai_sample_updated.csv";
    //以读入方式打开文件
    ifstream csv_data(fname, ios::in);
    ofstream dataFile;
	dataFile.open("shanghai_roce_30000_0.01.csv", ios::out | ios::trunc);
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
            double threshold = 0.01;
            vector<pair<double, double>> initial_traj = split_trajectory_points(traj);
            auto start = std::chrono::high_resolution_clock::now();
            vector<pair<double, double>> compressed_traj = trajectory_compress(initial_traj,threshold,compress_rate);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            time+=duration.count();
            //std::cout << "程序执行时间: " << duration.count() << " 微秒" << std::endl;
            for (const auto& point : compressed_traj) {
                //std::cout << "(" << std::setprecision(6) << std::fixed << point.first << ", " << std::setprecision(6) << std::fixed << point.second << ")" << " ";
            }
            //cout << endl;
            pair<double, double> start_point = initial_traj[0];
            pair<double, double> end_point;
            int start_pos = 0;
            bool find_end = 0;
            bool flag=0;
            for (int i = 1; i < initial_traj.size(); i++) {
                auto pos = find(compressed_traj.begin(), compressed_traj.end(), initial_traj[i]);
                if (pos != compressed_traj.end()) {
                    end_point = initial_traj[i];
                    for (int j = start_pos; j < i; j++) {
                        double psed_val=psed(initial_traj[j], start_point, end_point);
                        if(psed_val>threshold){
                            cout << j+1 << ":" << psed_val <<endl;
                            flag=1;
                        }
                    }
                    start_point = initial_traj[i];
                    start_pos = i;
                }
            }
            if(flag&&!hasDuplicates(initial_traj)){
                error++;
                //cout<<traj<<endl;
                //cout<<endl;
            }
            if(!flag){
                dataFile<<traj_id<<","<<"\"";
                dataFile<< setprecision(6) << fixed <<compressed_traj[0].first<<","<<setprecision(6) << fixed <<compressed_traj[0].second;
                for(int i=1;i<compressed_traj.size();i++){
                    dataFile<<"#"<<setprecision(6) << fixed <<compressed_traj[i].first<<","<<setprecision(6) << fixed <<compressed_traj[i].second;
                }
                dataFile<<"\""<<endl;
            }

            
            //cout<<endl;
            num++;
            if(num==30000)
                break;
        }
        cout<<"all run time is "<< std::setprecision(3) << std::fixed << double(time)/1000<<"ms"<<endl;
        //cout<<"all error num is "<<error<<endl;
        cout<<"average compress rate is "<<compress_rate/num<<endl;
        csv_data.close();
    }

}