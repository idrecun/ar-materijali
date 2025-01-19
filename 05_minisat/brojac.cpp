#include <iostream>
#include <vector>
#include <map>

using Clause = std::vector<int>;
using NormalForm = std::vector<Clause>;

int atomCount = 0;
NormalForm cnf;

std::map<int, int> pi;
std::map<int, int> qi;

int p(int i) {
    // daj mi redni broj literala koji predstavlja promenljivu pi
    if(pi.find(i) == pi.end())
        pi[i] = ++atomCount;
    return pi[i];
}

int q(int i) {
    if(qi.find(i) == qi.end())
        qi[i] = ++atomCount;
    return qi[i];
}

void clause(const Clause& c) {
    cnf.push_back(c);
}

void R(int i, int j) {
    clause({-q(j), -q(i)});
    clause({q(j), q(i)});
    clause({-p(j), p(i), q(i)});
    clause({-p(j), -p(i), -q(i)});
    clause({p(j), p(i), -q(i)});
    clause({p(j), -p(i), q(i)});
}

void nJ(int i, int j) {
    clause({p(i), p(j), q(i), q(j)});
    clause({p(i), p(j), -q(i), -q(j)});
    clause({-p(i), -p(j), q(i), q(j)});
    clause({-p(i), -p(j), -q(i), -q(j)});
}

int main() {
    R(1, 2);
    R(2, 3);
    R(3, 4);
    R(4, 5);
    nJ(1, 5);

    std::cout << "p cnf " << atomCount << " " << cnf.size() << std::endl;
    for(const auto& c : cnf) {
        for(const auto& l : c)
            std::cout << l << " ";
        std::cout << 0 << std::endl;
    }

    return 0;
}





