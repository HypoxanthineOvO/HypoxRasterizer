#ifndef RASTERIZER_HPP_
#define RASTERIZER_HPP_

#include "utils.hpp"

class Rasterizer {
public:
    /* Constructors */
    Rasterizer();

    // Pass
    void Pass();
    void VertexProcessing();
    void TriangleProcessing();
    void Rasterization();
    void FragmentProcessing();
    void DisplayToImage();
private:
};


#endif // RASTERIZER_HPP_