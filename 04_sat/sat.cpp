#include <iostream>
#include <vector>
#include <map>
#include <optional>
#include <fstream>

using Atom = int;
using Literal = int;
using Clause = std::vector<Literal>;
using NormalForm = std::vector<Clause>;

struct PartialValuation {
    int atomCount;
    std::vector<Literal> stack;
    std::map<Atom, bool> value;

    Literal backtrack() {
        Literal last = 0;
        while(!stack.empty() && stack.back() != 0) {
            last = stack.back();
            value.erase(std::abs(last));
            stack.pop_back();
        }

        if(stack.empty())
            return 0;

        stack.pop_back();
        return last;
    }

    void push(Literal l, bool decide) {
        if(decide)
            stack.push_back(0);
        stack.push_back(l);
        value[std::abs(l)] = l > 0;
    }

    bool isConflict(const Clause& clause) {
        for(const auto& literal : clause) {
            Atom atom = std::abs(literal);
            if(value.find(atom) == value.end())
                return false;
            if(value[atom] == (literal > 0))
                return false;
        }
        return true;
    }

    bool hasConflict(const NormalForm& cnf) {
        for(const auto& clause : cnf)
            if(isConflict(clause))
                return true;
        return false;
    }

    Literal isUnitClause(const Clause& clause) {
        Literal unit = 0;
        for(const auto& literal : clause) {
            Atom atom = std::abs(literal);
            if(value.find(atom) == value.end()) {
                if(unit != 0)
                    return 0;
                unit = literal;
            }
            else if(value[atom] == (literal > 0))
                return 0;
        }
        return unit;
    }

    Literal unitClause(const NormalForm& cnf) {
        Literal literal;
        for(const auto& clause : cnf) {
            if((literal = isUnitClause(clause)) != 0)
                return literal;
        }
        return 0;
    }

    Literal nextLiteral() {
        for(int atom = 1; atom <= atomCount; atom++)
            if(value.find(atom) == end(value))
                return atom;
        return 0;
    }

    void print() {
        for(int i = 0; i < stack.size(); i++)
            if(stack[i] == 0)
                std::cout << "| ";
            else
                std::cout << stack[i] << ' ';
        std::cout << std::endl;
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
        } else if((l = valuation.unitClause(cnf)) != 0) {
            valuation.push(l, false);
        } else if((l = valuation.nextLiteral()) != 0) {
            valuation.push(l, true);
        } else {
            return valuation;
        }
    }
    return {};
}

NormalForm parse(std::istream& input, int& atomCount) {
    std::string buffer;
    do {
        input >> buffer;
        if(buffer == "c")
            input.ignore(1000, '\n');
    } while(buffer != "p");
    // procitaj "cnf"
    input >> buffer;

    int clauseCount;
    input >> atomCount >> clauseCount;

    NormalForm res;
    for(int i = 0; i < clauseCount; i++) {
        Clause clause;
        Literal literal;
        input >> literal;
        while(literal != 0) {
            clause.push_back(literal);
            input >> literal;
        }
        res.push_back(clause);
    }
    return res;
}

int main() {
    std::string filename = "/Users/idrecun/matf/ar/sat/formula.cnf";
    std::ifstream inputFile(filename);

    int atomCount;
    auto formula = parse(inputFile, atomCount);
    auto valuation = solve(formula, atomCount);
    if(valuation.has_value()) {
        std::cout << "SAT" << std::endl;
        valuation.value().print();
    } else {
        std::cout << "UNSAT" << std::endl;
    }
    return 0;
}
