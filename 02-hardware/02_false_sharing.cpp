// False sharing : deux threads incrémentent chacun leur propre compteur
// (aucune donnée logiquement partagée), mais si les deux compteurs sont
// voisins dans la même ligne de cache de 64 octets, les cœurs se
// gênent mutuellement via le protocole de cohérence de cache.
//
// Compiler et exécuter (nécessite -pthread) :
//   g++ -std=c++20 -O2 -pthread -Wall -Wextra -o false_sharing 02_false_sharing.cpp && ./false_sharing

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

constexpr std::int64_t ITERATIONS = 100'000'000;

// alignas(64) sur la struct : force son adresse de départ à être alignée
// sur une ligne de cache, pour garantir de façon déterministe que `a` et
// `b` (8 octets chacun, contigus) tombent dans la MÊME ligne de 64 octets.
struct alignas(64) FalseSharing {
    std::atomic<std::int64_t> a{0};
    std::atomic<std::int64_t> b{0};
};

// Ici chaque compteur a son propre alignas(64) : chacun démarre sa PROPRE
// ligne de cache, donc plus aucune ligne n'est partagée entre les threads.
struct Padded {
    alignas(64) std::atomic<std::int64_t> a{0};
    alignas(64) std::atomic<std::int64_t> b{0};
};

template <typename T>
double run(T& counters) {
    auto t0 = std::chrono::steady_clock::now();
    std::thread t1([&] {
        for (std::int64_t i = 0; i < ITERATIONS; ++i) {
            counters.a.fetch_add(1, std::memory_order_relaxed);
        }
    });
    std::thread t2([&] {
        for (std::int64_t i = 0; i < ITERATIONS; ++i) {
            counters.b.fetch_add(1, std::memory_order_relaxed);
        }
    });
    t1.join();
    t2.join();
    auto t1_time = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(t1_time - t0).count();
}

int main() {
    FalseSharing fs;
    Padded p;

    std::cout << "adresses fs: a=" << &fs.a << " b=" << &fs.b
              << " (distance=" << (reinterpret_cast<char*>(&fs.b) - reinterpret_cast<char*>(&fs.a)) << " octets)\n";
    std::cout << "adresses p : a=" << &p.a << " b=" << &p.b
              << " (distance=" << (reinterpret_cast<char*>(&p.b) - reinterpret_cast<char*>(&p.a)) << " octets)\n\n";

    double fs_ms = run(fs);
    std::cout << "false sharing : " << fs_ms << " ms (a=" << fs.a.load() << ", b=" << fs.b.load() << ")\n";

    double p_ms = run(p);
    std::cout << "padded        : " << p_ms << " ms (a=" << p.a.load() << ", b=" << p.b.load() << ")\n";

    std::cout << "ratio         : " << (fs_ms / p_ms) << "x plus lent (false sharing vs padded)\n";
    return 0;
}
