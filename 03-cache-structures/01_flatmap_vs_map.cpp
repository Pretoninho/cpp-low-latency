// Flat map (vector trié + recherche dichotomique) vs std::map (arbre
// rouge-noir) : même jeu de clés, mêmes lookups, pour isoler l'effet de
// la localité mémoire sur la vitesse de recherche.
//
// Compiler et exécuter :
//   g++ -std=c++20 -O2 -Wall -Wextra -o flatmap_vs_map 01_flatmap_vs_map.cpp && ./flatmap_vs_map

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <vector>

constexpr std::size_t N_KEYS = 10'000;     // ex: niveaux de prix d'un order book
constexpr std::size_t N_LOOKUPS = 2'000'000;

int main() {
    std::vector<int> keys(N_KEYS);
    std::iota(keys.begin(), keys.end(), 0);

    std::map<int, double> tree_map;
    for (int k : keys) tree_map[k] = k * 1.5;

    std::vector<std::pair<int, double>> flat_map;
    flat_map.reserve(N_KEYS);
    for (int k : keys) flat_map.emplace_back(k, k * 1.5); // déjà trié (clés 0..N-1)

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, static_cast<int>(N_KEYS) - 1);
    std::vector<int> lookup_keys(N_LOOKUPS);
    for (auto& k : lookup_keys) k = dist(rng);

    // -- std::map --
    auto t0 = std::chrono::steady_clock::now();
    double sum_tree = 0;
    for (int k : lookup_keys) {
        sum_tree += tree_map.find(k)->second;
    }
    auto t1 = std::chrono::steady_clock::now();

    // -- flat map (recherche dichotomique) --
    auto t2 = std::chrono::steady_clock::now();
    double sum_flat = 0;
    for (int k : lookup_keys) {
        auto it = std::lower_bound(
            flat_map.begin(), flat_map.end(), k,
            [](const std::pair<int, double>& p, int key) { return p.first < key; });
        sum_flat += it->second;
    }
    auto t3 = std::chrono::steady_clock::now();

    double tree_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double flat_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

    std::cout << "std::map  : " << tree_ms << " ms (sum=" << sum_tree << ")\n";
    std::cout << "flat map  : " << flat_ms << " ms (sum=" << sum_flat << ")\n";
    std::cout << "ratio     : " << (tree_ms / flat_ms) << "x plus lent (std::map vs flat map)\n";
    return 0;
}
