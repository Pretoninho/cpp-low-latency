// Règle de 5 : une classe qui possède une ressource (ici un buffer alloué
// sur le tas) doit définir ses 5 membres spéciaux, sinon le compilateur en
// génère des versions par défaut incorrectes (copie superficielle du
// pointeur -> double free / use-after-free).
//
// Compiler et exécuter :
//   g++ -std=c++20 -Wall -Wextra -o buffer 01_buffer_rule_of_five.cpp && ./buffer

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <utility>

class Buffer {
public:
    explicit Buffer(std::size_t size)
        : size_(size), data_(size ? new unsigned char[size] : nullptr) {
        std::cout << "ctor        (size=" << size_ << ")\n";
        std::fill(data_, data_ + size_, 0);
    }

    // 1. Destructeur : libère la ressource possédée.
    ~Buffer() {
        std::cout << "dtor        (size=" << size_ << ", data=" << (void*)data_ << ")\n";
        delete[] data_;
    }

    // 2. Constructeur de copie : copie profonde, chaque Buffer possède son
    // propre buffer indépendant.
    Buffer(const Buffer& other)
        : size_(other.size_), data_(other.size_ ? new unsigned char[other.size_] : nullptr) {
        std::cout << "copy ctor   (size=" << size_ << ")\n";
        std::memcpy(data_, other.data_, size_);
    }

    // 3. Affectation par copie : idiome copy-and-swap, à la fois exception-safe
    // et sans duplication de logique avec le constructeur de copie.
    Buffer& operator=(const Buffer& other) {
        std::cout << "copy assign (size=" << other.size_ << ")\n";
        Buffer tmp(other);
        swap(tmp);
        return *this;
    }

    // 4. Constructeur de move : vole les ressources de `other` et le laisse
    // dans un état valide mais vide (nullptr, taille 0).
    Buffer(Buffer&& other) noexcept
        : size_(std::exchange(other.size_, 0)), data_(std::exchange(other.data_, nullptr)) {
        std::cout << "move ctor   (size=" << size_ << ")\n";
    }

    // 5. Affectation par move : même idée, via swap pour rester
    // exception-safe et laisser `other` détruire l'ancien contenu de *this.
    Buffer& operator=(Buffer&& other) noexcept {
        std::cout << "move assign (size=" << other.size_ << ")\n";
        swap(other);
        return *this;
    }

    std::size_t size() const { return size_; }
    unsigned char* data() { return data_; }

    void swap(Buffer& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
    }

private:
    std::size_t size_;
    unsigned char* data_;
};

int main() {
    std::cout << "-- construction --\n";
    Buffer a(16);

    std::cout << "\n-- copie --\n";
    Buffer b = a; // copy ctor

    std::cout << "\n-- move --\n";
    Buffer c = std::move(a); // move ctor, `a` devient vide

    std::cout << "\n-- affectation par copie --\n";
    Buffer d(4);
    d = b; // copy assign

    std::cout << "\n-- affectation par move --\n";
    Buffer e(4);
    e = std::move(c); // move assign, `c` devient vide

    std::cout << "\n-- fin de portee, destructions --\n";
    return 0;
}
