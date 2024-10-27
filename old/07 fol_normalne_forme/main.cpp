#include <iostream>
#include "interpretation.h"
#include "formula.h"

unsigned zero(const std::vector<unsigned>&) { return 0; }
unsigned one(const std::vector<unsigned>&)  { return 1; }
unsigned plus(const std::vector<unsigned>& args) { return (args[0] + args[1]) % 4; }
unsigned times(const std::vector<unsigned>& args) { return (args[0] * args[1]) % 4; }
bool even(const std::vector<unsigned>& args) { return args[0] % 2 == 0; }
bool equals(const std::vector<unsigned>& args) { return args[0] == args[1]; }

int main() {

    Signature s;
    s.rel["p"] = 1;
    s.rel["q"] = 2;

    // Ax (p(x) & T)
    TermPtr x = VariableT("x");
    TermPtr y = VariableT("y");
    FormulaPtr allPxAndT = All("x", And(Atom("p", {x}), True()));
    print(allPxAndT); std::cout << std::endl;

    FormulaPtr simplifyAllPx = simplify(allPxAndT);
    print(simplifyAllPx); std::cout << std::endl;

    // Ax p(x) | Ex Ey q(x, y)
    FormulaPtr f = Or(All("x", Atom("p", {x})), Exists("x", Exists("y", Atom("q", {x, y}))));
    print(f); std::cout << std::endl;

    FormulaPtr prenexf = prenex(nnf(simplify(f)));
    print(prenexf); std::cout << std::endl;

    FormulaPtr skolemf = skolem(prenexf, s);
    print(skolemf); std::cout << std::endl;

    return 0;
}
