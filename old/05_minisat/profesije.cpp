#include <iostream>
#include <vector>
#include <map>

using namespace std;

using Clause = vector<int>;
using CNF = vector<Clause>;

string imena = "SBCT";

int atomCount = 0;
map< pair<char, char>, int> xs;
map< pair<char, char>, int> ys;
CNF formula;

int lazy(char i, char j, map< pair<char, char>, int> &m) {
    if(m.find({i, j}) == end(m))
        m[make_pair(i, j)] = ++atomCount;
    return m[make_pair(i, j)];
}

int x(char i, char j) { return lazy(i, j, xs); }
int y(char i, char j) { return lazy(i, j, ys); }
void clause(const Clause& c) { formula.push_back(c); }

int main() {
    // 1. svako ima bar jednu od ovih profesija
    for(char prezime : imena) {
        clause({x(prezime, 'S'), x(prezime, 'B'), x(prezime, 'C'), x(prezime, 'T')});
        clause({y(prezime, 'S'), y(prezime, 'B'), y(prezime, 'C'), y(prezime, 'T')});
    }

    // 2. svako ima najvise jednu od ovih profesija
    for(char prezime : imena)
        for(char profesija1 : imena)
            for(char profesija2 : imena)
                if(profesija1 != profesija2) {
                    clause({-x(prezime, profesija1), -x(prezime, profesija2)});
                    clause({-y(prezime, profesija1), -y(prezime, profesija2)});
                }

    // 3. niko nema profesiju kao svoje prezime
    for(char prezime : imena) {
        clause({-x(prezime, prezime)});
        clause({-y(prezime, prezime)});
    }

    // 4. otac i sin nemaju istu profesiju
    for(char prezime : imena)
        for(char profesija : imena)
            clause({-x(prezime, profesija), -y(prezime, profesija)});

    // 5. otac baker i sin carpenter imaju istu profesiju
    for(char profesija : imena) {
        clause({-x('B', profesija), y('C', profesija)});
        clause({x('B', profesija), -y('C', profesija)});
    }

    // 6. smith-ov sin je pekar
    clause({y('S', 'B')});



    for(auto [par, index] : xs)
        cout << "c x(" << par.first << ", " << par.second << ") -> " << index << endl;

    for(auto [par, index] : ys)
        cout << "c y(" << par.first << ", " << par.second << ") -> " << index << endl;

    cout << "p cnf " << atomCount << ' ' << formula.size() << endl;
    for(auto c : formula) {
        for(auto l : c)
            cout << l << ' ';
        cout << 0 << endl;
    }

    return 0;
}
