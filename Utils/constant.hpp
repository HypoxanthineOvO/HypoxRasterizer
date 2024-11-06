#ifndef CONSTANT_HPP_
#define CONSTANT_HPP_

/* Constants */
// EPSILON
#define PARALLEL_EPS 1e-8 //whether two vectors are parallel
#define LENGTH_EPS 1e-8 //whether the vector has length 0
// Coordinate System
#define REF_UP Vec3f(0, 1, 0)
#define REF_RIGHT Vec3f(1, 0, 0)
// Scene
#define AMBIENT Vec3f(0.1, 0.1, 0.1)
// Default Values
#define DEFAULT_WIDTH 100
#define DEFAULT_HEIGHT 100
#define DEFAULT_FAR 20
#define DEFAULT_FOV 120
// Shadow Map
#define DEFAULT_SHADOW_MAP_RESOLUTION 1024
#define SHADOW_MAP_BIAS 1e-3
#endif // CONSTANT_HPP_