// Hiérarchie mémoire : coût d'un accès séquentiel (préfetchable) vs un accès
// en chaîne de pointeurs (imprévisible, chaque accès dépend du précédent).
//
// Compiler et exécuter :
//   g++ -std=c++20 -O2 -Wall -Wextra -o cache_locality 01_cache_locality.cpp && ./cache_locality

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

int main() {
    // 32M int = 128 Mio : largement plus gros que le L3 (33 Mio sur cette
    // machine), pour être sûr de sortir du cache à chaque tour.
    constexpr std::size_t N = 32 * 1024 * 1024;
    std::vector<int> data(N);
    std::iota(data.begin(), data.end(), 0);

    // -- parcours séquentiel : data[0], data[1], data[2], ... --
    auto t0 = std::chrono::steady_clock::now();
    long long sum_seq = 0;
    for (std::size_t i = 0; i < N; ++i) {
        sum_seq += data[i];
    }
    auto t1 = std::chrono::steady_clock::now();

    // -- construction d'un cycle aléatoire sur les indices [0, N) --
    // next[i] = indice suivant à visiter depuis i. Un seul grand cycle qui
    // passe par tous les indices une fois, mais dans un ordre imprévisible.
    std::vector<std::size_t> perm(N);
    std::iota(perm.begin(), perm.end(), 0);
    std::mt19937_64 rng(42);
    std::shuffle(perm.begin(), perm.end(), rng);

    std::vector<std::size_t> next(N);
    for (std::size_t i = 0; i < N; ++i) {
        next[perm[i]] = perm[(i + 1) % N];
    }

    // -- parcours en chaîne : l'adresse suivante dépend de la valeur lue --
    auto t2 = std::chrono::steady_clock::now();
    long long sum_rand = 0;
    std::size_t idx = 0;
    for (std::size_t i = 0; i < N; ++i) {
        sum_rand += data[idx];
        idx = next[idx];
    }
    auto t3 = std::chrono::steady_clock::now();

    double seq_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double rand_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

    std::cout << "sequentiel : " << seq_ms << " ms (sum=" << sum_seq << ")\n";
    std::cout << "aleatoire  : " << rand_ms << " ms (sum=" << sum_rand << ")\n";
    std::cout << "ratio      : " << (rand_ms / seq_ms) << "x plus lent\n";

    return 0;
}
