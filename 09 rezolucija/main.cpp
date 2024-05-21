#include <iostream>
#include "interpretation.h"
#include "formula.h"
#include "resolution.h"

int main() {
    // {p(x)}
    // {~p(f(x))}
    Clause a1 = {Atom("p", {VariableT("x")})};
    Clause a2 = {Not(Atom("p", {FunctionT("f", {VariableT("y")})}))};
    CNF aCNF = {a1, a2};

    std::cout << (resolution(aCNF) ? "SAT" : "UNSAT") << std::endl;


    // {p(x), q(x)}
    // {~p(x), r(f(x), y)}
    // {~q(x), r(y, f(x))}
    // {~r(x, y)}
    Clause b1 = {Atom("p", {VariableT("x")}), Atom("q", {VariableT("x")})};
    Clause b2 = {Not(Atom("p", {VariableT("x")})), Atom("r", {FunctionT("f", {VariableT("x")}), VariableT("y")})};
    Clause b3 = {Not(Atom("q", {VariableT("x")})), Atom("r", {VariableT("y"), FunctionT("f", {VariableT("x")})})};
    Clause b4 = {Not(Atom("r", {VariableT("x"), VariableT("y")}))};
    CNF bCNF = {b1, b2, b3, b4};

    std::cout << (resolution(bCNF) ? "SAT" : "UNSAT") << std::endl;


    // {p(x), p(y)}
    // {~p(x), ~p(y)}
    Clause c1 = {Atom("p", {VariableT("x")}), Atom("p", {VariableT("y")})};
    Clause c2 = {Not(Atom("p", {VariableT("x")})), Not(Atom("p", {VariableT("y")}))};
    CNF cCNF = {c1, c2};

    std::cout << (resolution(cCNF) ? "SAT" : "UNSAT") << std::endl;

    return 0;
}
