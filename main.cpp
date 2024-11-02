#include <iostream>
#include "configs.hpp"
#include "rasterizer.hpp"

std::string config_path = "./configs/Base.json";

int main(int argc, char const *argv[]) {
    puts("==================== HypoxRasterizer ====================");
    
    
    // Create a Rasterizer
    Rasterizer rast(config_path);
    

    // Pass the Rasterizer
    rast.Pass();
    return 0;
}