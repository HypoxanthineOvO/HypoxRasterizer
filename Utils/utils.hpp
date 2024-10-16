#ifndef UTILS_HPP_
#define UTILS_HPP_

#include "cores.hpp"
#include <iostream>

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
};

#endif // UTILS_HPP_