#include "geometry.hpp"

int main() {
    Vertex v_0(0, 0, 0), v_1(0, 1, 0), v_2(0, 0, 1);
    Triangle tri(v_0, v_1, v_2);

    // Test `isInside` and `getInterpolationWeights`
    bool inside = tri.isInside(Vec3f(0, 0.2, 0.2));
    printf("Is Inside: %d\n", inside);
    
    Vec3f weights = tri.getInterpolationWeights(Vec3f(0, 0.2, 0.2));
    utils::printVec(weights);
}