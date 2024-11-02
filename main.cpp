#include <iostream>
#include "configs.hpp"
#include "rasterizer.hpp"

std::string config_path = "./configs/Base.json";

int main(int argc, char const *argv[]) {
    puts("==================== HypoxRasterizer ====================");
    if (argc > 1) {
        config_path = argv[1];
    }
    else {
        throw std::runtime_error("Please provide the config file path!");
    }
    
    // Create a Rasterizer
    Rasterizer rast(config_path);
    

    // Pass the Rasterizer
    rast.Pass();
    return 0;
}