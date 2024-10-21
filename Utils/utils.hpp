#ifndef UTILS_HPP_
#define UTILS_HPP_

#include "cores.hpp"
#include "image.hpp"
#include <iostream>
#include <cmath>
namespace utils {
    // Print Function for Eigen Types
    static void printVec(const VecXf& vec) {
        for (int i = 0; i < vec.size(); i++) {
            printf("%f ", vec(i));
        }
        printf("\n");
    }
    static void printMat(const MatXf& mat) {
        for (int i = 0; i < mat.rows(); i++) {
            for (int j = 0; j < mat.cols(); j++) {
                printf("%f ", mat(i, j));
            }
            printf("\n");
        }
        printf("\n");
    }
    static bool isequalVec(const VecXf& vec1, const VecXf& vec2) {
        if (vec1.size() != vec2.size()){
            printf("两个向量维度不一样");
            exit(0);
        }
        for (int i = 0; i < vec1.size(); i++) {
            if (std::abs((vec1(i) - vec2(i))) > PARALLEL_EPS)
                return false;
        }
        return true;
    }
};

#endif // UTILS_HPP_