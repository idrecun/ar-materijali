#include <iostream>
#include <vector>
#include <map>
#include <optional>

using Atom = int;
using Literal = int;
using Clause = std::vector<Literal>;
using NormalForm = std::vector<Clause>;

struct PartialValuation {
    int atomCount;
    std::map<Atom, bool> value;
    std::vector<Literal> stack;

    void print() {
        for(auto x : stack)
            std::cout << x << ' ';
        std::cout << std::endl;
    }

    void push(Literal l, bool decide) {
        if(decide)
            stack.push_back(0);
        stack.push_back(l);
        value[std::abs(l)] = l > 0;
    }

    Literal backtrack() {
        Literal last = 0;
        while(!stack.empty() && stack.back() != 0) {
            last = stack.back();
            stack.pop_back();
            value.erase(std::abs(last));
        }

        if(stack.empty())
            return 0;

        stack.pop_back();
        return last;
    }

    bool isConflict(const Clause& clause) {
        for(Literal l : clause) {
            Atom atom = std::abs(l);
            if(value.find(atom) == end(value))
                return false;
            if(value[atom] == (l > 0))
                return false;
        }
        return true;
    }

    bool hasConflict(const NormalForm& cnf) {
        for(const Clause& c : cnf)
            if(isConflict(c))
                return true;
        return false;
    }

    Literal isUnitClause(const Clause& clause) {
        Literal unit = 0;
        for(Literal l : clause) {
            Atom atom = std::abs(l);
            if(value.find(atom) == end(value)) {
                if(unit != 0)
                    return 0;
                unit = l;
            }
            else if(value[atom] == (l > 0))
                return 0;
        }
        return unit;
    }

    Literal hasUnitLiteral(const NormalForm& cnf) {
        Literal l = 0;
        for(const Clause& c : cnf)
            if((l = isUnitClause(c)) != 0)
                return l;
        return l;
    }

    Literal nextLiteral() {
        for(int atom = 1; atom < atomCount; atom++)
            if(value.find(atom) == end(value))
                return atom;
        return 0;
    }
};

std::optional<PartialValuation> solve(NormalForm& cnf, int atomCount) {
    PartialValuation valuation;
    valuation.atomCount = atomCount;

    Literal l;
    while(true) {
        valuation.print();

        if(valuation.hasConflict(cnf)) {
            l = valuation.backtrack();
            if(l == 0)
                break;
            valuation.push(-l, false);
        }
        else if((l = valuation.hasUnitLiteral(cnf)) != 0)
            valuation.push(l, false);
        else if((l = valuation.nextLiteral()) != 0)
            valuation.push(l, true);
        else
            return valuation;
    }
    return {};
}

NormalForm parse(std::istream& fin, int& atomCount) {
    std::string buffer;
    do {
        fin >> buffer;
        if(buffer == "c")
            fin.ignore(10000, '\n');
    } while(buffer != "p");

    // procitaj "cnf"
    fin >> buffer;

    int clauseCount;
    fin >> atomCount >> clauseCount;

    NormalForm formula;
    for(int i = 0; i < clauseCount; i++) {
        Clause c;

        Literal l;
        fin >> l;
        while(l != 0) {
            c.push_back(l);
            fin >> l;
        }

        formula.push_back(c);
    }

    return formula;
}

int main() {
    int atomCount = 0;
    NormalForm formula = parse(std::cin, atomCount);

    auto valuation = solve(formula, atomCount);
    if(valuation)
        std::cout << "SAT" << std::endl;
    else
        std::cout << "UNSAT" << std::endl;

    return 0;
}

