#include <iostream>
#include "interpretation.h"
#include "formula.h"
#include "resolution.h"

int main() {
    // p(x, f(y))
    // p(a, f(g(b)))
    FormulaPtr atom1 = Atom("p", {VariableT("x"), FunctionT("f", {VariableT("y")})});
    FormulaPtr atom2 = Atom("p", {FunctionT("a", {}), FunctionT("f", {FunctionT("g", {FunctionT("b", {})})})});

    std::optional<Substitution> unifier = unify(atom1, atom2);

    if(unifier) {
        for(const auto& p : unifier.value()) {
            std::cout << p.first << " -> ";
            print(p.second);
            std::cout << "  ";
        }
        std::cout << std::endl;
    }
    else {
        std::cout << "Unification failed" << std::endl;
    }

    return 0;
}
