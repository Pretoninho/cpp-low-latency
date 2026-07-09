// Prédiction de branchement : même tableau, même somme conditionnelle,
// une fois non trié, une fois trié — pour isoler l'effet du branchement
// sur les performances (aucune autre variable ne change).
//
// Compiler et exécuter :
//   g++ -std=c++20 -O2 -Wall -Wextra -o branch_prediction 03_branch_prediction.cpp && ./branch_prediction

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

constexpr std::size_t N = 32 * 1024 * 1024;
constexpr int THRESHOLD = 128;

long long sum_above_threshold(const std::vector<int>& data) {
    long long sum = 0;
    for (std::size_t i = 0; i < N; ++i) {
        if (data[i] >= THRESHOLD) { // <-- le branchement dont l'issue dépend de la donnée
            sum += data[i];
        }
    }
    return sum;
}

int main() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);

    std::vector<int> data(N);
    for (auto& v : data) v = dist(rng);

    auto t0 = std::chrono::steady_clock::now();
    long long sum_unsorted = sum_above_threshold(data);
    auto t1 = std::chrono::steady_clock::now();

    std::sort(data.begin(), data.end());

    auto t2 = std::chrono::steady_clock::now();
    long long sum_sorted = sum_above_threshold(data);
    auto t3 = std::chrono::steady_clock::now();

    double unsorted_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double sorted_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

    std::cout << "non trie : " << unsorted_ms << " ms (sum=" << sum_unsorted << ")\n";
    std::cout << "trie     : " << sorted_ms << " ms (sum=" << sum_sorted << ")\n";
    std::cout << "ratio    : " << (unsorted_ms / sorted_ms) << "x plus lent (non trie vs trie)\n";
    return 0;
}
