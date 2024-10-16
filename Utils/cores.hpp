#ifndef CORES_HPP
#define CORES_HPP
#include "constant.hpp"

#include <Eigen/Core>
#include <Eigen/Dense>
#include <memory>

/* Eigen Types */
// Template Types
template <typename T>
using Vector = Eigen::Matrix<T, Eigen::Dynamic, 1>;
template <typename T>
using Matrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

// Float Types
using Vec2i = Eigen::Vector2i;
using Vec3i = Eigen::Vector3i;
using Vec4i = Eigen::Vector4i;

using Vec2f = Eigen::Vector2f;
using Vec3f = Eigen::Vector3f;
using Vec4f = Eigen::Vector4f;

using Mat3f = Eigen::Matrix3f;
using Mat4f = Eigen::Matrix4f;

using VecXf = Eigen::VectorXf;
using MatXf = Eigen::MatrixXf;



#endif // CORES_HPP