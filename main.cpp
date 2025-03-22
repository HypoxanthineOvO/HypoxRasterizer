#include <iostream>
#include "configs.hpp"
#include "rasterizer.hpp"

std::string config_path;

int main(int argc, char const *argv[]) {
    puts("==================== HypoxRasterizer ====================");
    if (argc > 1) {
        config_path = argv[1];
    }
    else {
        std::cout << "Please provide the config file path: ";
        std::cin >> config_path;
    }
    
    // Create a Rasterizer
    Rasterizer rast(config_path);
    

    // Pass the Rasterizer
    rast.Pass();
    return 0;
}