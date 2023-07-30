#include<iostream>
#include<vector>
#include<math.h>
#include<functional>
#include<string>
using namespace std;

double distance(const std::pair<double, double>& point1, const std::pair<double, double>& point2) {
    double dx = point2.first - point1.first;
    double dy = point2.second - point1.second;
    return std::sqrt(dx * dx + dy * dy);
}

double computeDistance(const std::vector<std::pair<double, double>>& trajectory1, const std::vector<std::pair<double, double>>& trajectory2) {
    std::vector<std::vector<double>> distances(trajectory1.size(), std::vector<double>(trajectory2.size(), -1.0));

    std::function<double(size_t, size_t)> computeDistance = [&](size_t i, size_t j) -> double {
        if (distances[i][j] > -1.0) {
            return distances[i][j];
        }

        if (i == 0 && j == 0) {
            distances[i][j] = distance(trajectory1[i], trajectory2[j]);
        }
        else if (i > 0 && j == 0) {
            distances[i][j] = std::max(computeDistance(i - 1, j), distance(trajectory1[i], trajectory2[j]));
        }
        else if (i == 0 && j > 0) {
            distances[i][j] = std::max(computeDistance(i, j - 1), distance(trajectory1[i], trajectory2[j]));
        }
        else {
            double minDistance = std::min(std::min(computeDistance(i - 1, j), computeDistance(i - 1, j - 1)), computeDistance(i, j - 1));
            distances[i][j] = std::max(minDistance, distance(trajectory1[i], trajectory2[j]));
        }

        return distances[i][j];
    };

    double frechetDistance = computeDistance(trajectory1.size() - 1, trajectory2.size() - 1);

    return frechetDistance;
}