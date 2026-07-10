// Contrepartie du benchmark précédent : le coût d'INSERTION, pas de lecture.
// std::map insère en O(log n) (pas de décalage, juste rattacher un nœud).
// La flat map insère en O(n) : il faut décaler tous les éléments après le
// point d'insertion pour garder le vecteur trié.
//
// Compiler et exécuter :
//   g++ -std=c++20 -O2 -Wall -Wextra -o flatmap_vs_map_insert 02_flatmap_vs_map_insert.cpp && ./flatmap_vs_map_insert

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <vector>

constexpr std::size_t N_INITIAL = 10'000; // niveaux de prix déjà en place
constexpr std::size_t N_INSERTS = 2'000;  // nouveaux niveaux à insérer

int main() {
    // clés paires déjà en place ; on insère des clés impaires (jamais présentes)
    std::vector<int> initial_keys(N_INITIAL);
    for (std::size_t i = 0; i < N_INITIAL; ++i) initial_keys[i] = static_cast<int>(2 * i);

    std::map<int, double> tree_map;
    for (int k : initial_keys) tree_map[k] = k * 1.5;

    std::vector<std::pair<int, double>> flat_map;
    flat_map.reserve(N_INITIAL + N_INSERTS);
    for (int k : initial_keys) flat_map.emplace_back(k, k * 1.5);

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, static_cast<int>(N_INITIAL) - 1);
    std::vector<int> new_keys(N_INSERTS);
    for (auto& k : new_keys) k = 2 * dist(rng) + 1; // clé impaire = jamais présente

    // -- insertion dans std::map --
    auto t0 = std::chrono::steady_clock::now();
    for (int k : new_keys) {
        tree_map[k] = k * 1.5;
    }
    auto t1 = std::chrono::steady_clock::now();

    // -- insertion dans la flat map (recherche + décalage) --
    auto t2 = std::chrono::steady_clock::now();
    for (int k : new_keys) {
        auto it = std::lower_bound(
            flat_map.begin(), flat_map.end(), k,
            [](const std::pair<int, double>& p, int key) { return p.first < key; });
        flat_map.insert(it, {k, k * 1.5}); // décale tout ce qui suit `it`
    }
    auto t3 = std::chrono::steady_clock::now();

    double tree_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double flat_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

    std::cout << "tailles finales : std::map=" << tree_map.size() << ", flat map=" << flat_map.size() << "\n";
    std::cout << "std::map  : " << tree_ms << " ms\n";
    std::cout << "flat map  : " << flat_ms << " ms\n";
    std::cout << "ratio     : " << (flat_ms / tree_ms) << "x plus lent (flat map vs std::map, en INSERTION)\n";
    return 0;
}
