#include <iostream>
#include <vector>
#include <cmath>

// 定义点结构体
struct Point {
    double x;
    double y;
};

// 计算点p到线段(start, end)的垂直距离
double PerpendicularDistance(const Point& p, const Point& start, const Point& end) {
    double dx = end.x - start.x;
    double dy = end.y - start.y;
    double length = std::sqrt(dx * dx + dy * dy);
    double distance = 0.0;

    if (length != 0.0) {
        distance = std::abs(dy * p.x - dx * p.y + end.x * start.y - end.y * start.x) / length;
    }

    return distance;
}

// Douglas-Peucker算法
void DouglasPeucker(const std::vector<Point>& pointList, double epsilon, std::vector<Point>& simplifiedList, int startIndex = 0, int endIndex = -1) {
    if (endIndex == -1) {
        endIndex = pointList.size() - 1;
    }

    // 找到起点和终点
    Point start = pointList[startIndex];
    Point end = pointList[endIndex];

    // 初始化最大距离和对应的关键点索引
    double maxDistance = 0.0;
    int maxDistanceIndex = -1;

    // 计算每个点到起点和终点构成的线段的垂直距离
    for (int i = startIndex + 1; i < endIndex; i++) {
        double distance = PerpendicularDistance(pointList[i], start, end);
        if (distance > maxDistance) {
            maxDistance = distance;
            maxDistanceIndex = i;
        }
    }

    // 如果最大距离大于阈值epsilon，则递归处理两个子曲线
    if (maxDistance > epsilon) {
        // 递归处理左侧子曲线
        DouglasPeucker(pointList, epsilon, simplifiedList, startIndex, maxDistanceIndex);

        // 添加关键点
        simplifiedList.push_back(pointList[maxDistanceIndex]);

        // 递归处理右侧子曲线
        DouglasPeucker(pointList, epsilon, simplifiedList, maxDistanceIndex, endIndex);
    }
}

