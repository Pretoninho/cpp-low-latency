// Corruption silencieuse : deux threads incrémentent le MÊME int normal
// (non-atomique), sans aucune synchronisation. Le résultat final est
// presque toujours FAUX (inférieur à la somme attendue), sans crash,
// sans avertissement.
//
// Compiler et exécuter :
//   g++ -std=c++20 -O2 -pthread -Wall -Wextra -o data_race 01_data_race.cpp && ./data_race

#include <cstdint>
#include <iostream>
#include <thread>

constexpr std::int64_t ITERATIONS = 20'000'000;

int main() {
    int counter = 0; // int normal, PAS std::atomic

    std::thread t1([&] {
        for (std::int64_t i = 0; i < ITERATIONS; ++i) counter++;
    });
    std::thread t2([&] {
        for (std::int64_t i = 0; i < ITERATIONS; ++i) counter++;
    });

    t1.join();
    t2.join();

    std::int64_t expected = 2 * ITERATIONS;
    std::cout << "attendu : " << expected << "\n";
    std::cout << "obtenu  : " << counter << "\n";
    std::cout << "increments perdus : " << (expected - counter) << "\n";
    return 0;
}
