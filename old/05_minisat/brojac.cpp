#include <iostream>
#include <vector>
#include <map>

using namespace std;

using Clause = vector<int>;
using NormalForm = vector<Clause>;

int atomCount = 0;
NormalForm formula;

map<int, int> pi;
map<int, int> qi;

int p(int i) {
    if(pi.find(i) == end(pi))
        pi[i] = ++atomCount;
    return pi[i];
}

int q(int i) {
    if(qi.find(i) == end(qi))
        qi[i] = ++atomCount;
    return qi[i];
}

void clause(const Clause& c) {
    formula.push_back(c);
}

// R(Si, Sj)
void R(int i, int j) {
    clause({-q(j), -q(i)});
    clause({q(j), q(i)});
    clause({-p(j), -p(i), -q(i)});
    clause({-p(j), p(i), q(i)});
    clause({p(j), -p(i), q(i)});
    clause({p(j), p(i), -q(i)});
}

// ~J(Si, Sj)
void nJ(int i, int j) {
    clause({-p(i), -p(j), -q(i), -q(j)});
    clause({-p(i), -p(j), q(i), q(j)});
    clause({p(i), p(j), -q(i), -q(j)});
    clause({p(i), p(j), q(i), q(j)});
}

int main() {
    R(0, 1);
    R(1, 2);
    R(2, 3);
    R(3, 4);
    nJ(0, 4);

    cout << "p cnf " << atomCount << ' ' << formula.size() << endl;
    for(auto c : formula) {
        for(auto l : c)
            cout << l << ' ';
        cout << 0 << endl;
    }

    return 0;
}













