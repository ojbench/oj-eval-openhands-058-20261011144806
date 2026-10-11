#include "src_minimal.hpp"
#include <iostream>

int main() {
    try {
        Pokedex dex("minimal_test.txt");
        
        // Test basic operations
        dex.pokeAdd("Bulbasaur", 1, "grass");
        dex.pokeAdd("Charmander", 2, "fire");
        dex.pokeAdd("Squirtle", 3, "water");
        
        // Test find
        std::cout << "Find 1: " << dex.pokeFind(1) << std::endl;
        std::cout << "Find 2: " << dex.pokeFind(2) << std::endl;
        std::cout << "Find 3: " << dex.pokeFind(3) << std::endl;
        
        // Test attack
        std::cout << "Fire vs Grass: " << dex.attack("fire", 1) << std::endl;
        std::cout << "Water vs Fire: " << dex.attack("water", 2) << std::endl;
        
        // Test iterator
        std::cout << "Iterator test:" << std::endl;
        for (auto it = dex.begin(); it != dex.end(); ++it) {
            std::cout << it->name << " (" << it->id << ")" << std::endl;
        }
        
    } catch (const std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}