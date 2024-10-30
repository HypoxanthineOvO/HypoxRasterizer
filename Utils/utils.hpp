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

    // Util Function for Eigen Types
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

    // Specific Function
    static Mat4f generateTranslationMatrix(const Vec3f& translation) {
        // Generate Translation Matrix by parameters
        Mat4f translation_matrix = Mat4f::Identity();
        translation_matrix(0, 3) = translation(0);
        translation_matrix(1, 3) = translation(1);
        translation_matrix(2, 3) = translation(2);
        return translation_matrix;
    }
    static Mat4f generateRotationMatrix(const Vec3f& rotation) {
        // Generate Rotation Matrix by parameters. Input rotation is in degree
        float x = rotation(0) * M_PI / 180.0f,
                y = rotation(1) * M_PI / 180.0f,
                z = rotation(2) * M_PI / 180.0f;
        Mat4f rotation_matrix = Mat4f::Identity();
        Mat4f rotation_x = Mat4f::Identity();
        Mat4f rotation_y = Mat4f::Identity();
        Mat4f rotation_z = Mat4f::Identity();
        rotation_x(1, 1) = std::cos(x), rotation_x(1, 2) = -std::sin(x);
        rotation_x(2, 1) = std::sin(x), rotation_x(2, 2) = std::cos(x);

        rotation_y(0, 0) = std::cos(y), rotation_y(0, 2) = std::sin(y);
        rotation_y(2, 0) = -std::sin(y), rotation_y(2, 2) = std::cos(y);

        rotation_z(0, 0) = std::cos(z), rotation_z(0, 1) = -std::sin(z);
        rotation_z(1, 0) = std::sin(z), rotation_z(1, 1) = std::cos(z);

        rotation_matrix = rotation_z * rotation_y * rotation_x;
        return rotation_matrix;
    }
    static Mat4f generateScaleMatrix(const Vec3f& scale) {
        // Generate Scale Matrix by parameters
        Mat4f scale_matrix = Mat4f::Identity();
        scale_matrix(0, 0) = scale(0);
        scale_matrix(1, 1) = scale(1);
        scale_matrix(2, 2) = scale(2);
        return scale_matrix;
    }
    static Mat4f generateModelMatrix(
        const Vec3f& translation, const Vec3f& rotation, const Vec3f& scale
    ) {
        // Generate Model Matrix by parameters
        Mat4f translation_matrix = generateTranslationMatrix(translation);
        Mat4f rotation_matrix = generateRotationMatrix(rotation);
        Mat4f scale_matrix = generateScaleMatrix(scale);
        return translation_matrix * rotation_matrix * scale_matrix;
    }
};

#endif // UTILS_HPP_