#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "geometry.hpp" 

int main() {
    Triangle triangle(Vec3f(0.25, 0.5, 1), Vec3f(0.75, 0.75, 1), Vec3f(0.75, 0.25, 1));

    std::ifstream inputFile("./Tests/TriangleTestByPython/input.txt");
    std::ofstream outputFile("./Tests/TriangleTestByPython/output.txt");
    std::string line;
    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        float x, y, z;
        if (!(iss >> x >> y >> z)) {
            break;
        }
        Vec3f point(x, y, z);
        bool isInside = triangle.isInside(point);
        
        outputFile << (isInside ? 1 : 0) << "\n";
    }

    inputFile.close();
    outputFile.close();
    return 0;
}
