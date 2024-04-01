/*
* Implementira formulu pomocu std::variant
* Izbegavamo OOP, umesto cega koristimo algebarske tipove podataka
* Implementacija funkcija na ovaj nacin je nezgodna
* - jedno resenje u main.cpp
* - drugo resenje u visitor.cpp
*/
#include <iostream>
#include <memory>
#include <variant>

struct False; struct True; struct Atom; struct Not; struct Binary;

using Formula = std::variant<False, True, Atom, Not, Binary>;
using FormulaPtr = std::shared_ptr<Formula>;

struct False  { };
struct True   { };
struct Atom   { std::string name; };
struct Not    { FormulaPtr sub; };
struct Binary {
    enum Type {
        And, Or, Imp, Eq
    } type;

    FormulaPtr l, r;
};

FormulaPtr ptr(Formula f) { return std::make_shared<Formula>(f); }

std::string sign(Binary::Type type) {
    switch(type) {
        case Binary::And: return " & ";
        case Binary::Or:  return " | ";
        case Binary::Imp: return " -> ";
        case Binary::Eq:  return " <-> ";
    }
    return "";
}

void print(FormulaPtr);
void printF() { std::cout << "F"; }
void printT() { std::cout << "T"; }
void printA(Atom a) { std::cout << a.name; }
void printN(Not n) { std::cout << "~"; print(n.sub); }
void printB(Binary b) { 
    std::cout << "(";
    print(b.l);
    std::cout << sign(b.type);
    print(b.r);
    std::cout << ")";
}
void print(FormulaPtr f) {
    if(std::holds_alternative<False>(*f))
        printF();
    else if(std::holds_alternative<True>(*f))
        printT();
    else if(std::holds_alternative<Atom>(*f))
        printA(std::get<Atom>(*f));
    else if(std::holds_alternative<Not>(*f))
        printN(std::get<Not>(*f));
    else
        printB(std::get<Binary>(*f));
}

unsigned complexity(FormulaPtr);
unsigned complexityF() { return 0; }
unsigned complexityT() { return 0; }
unsigned complexityA(Atom) { return 0; }
unsigned complexityN(Not n) { return 1 + complexity(n.sub); }
unsigned complexityB(Binary b) { return 1 + complexity(b.l) + complexity(b.r); }

unsigned complexity(FormulaPtr f) {
    if(std::holds_alternative<False>(*f))
        return complexityF();
    else if(std::holds_alternative<True>(*f))
        return complexityT();
    else if(std::holds_alternative<Atom>(*f))
        return complexityA(std::get<Atom>(*f));
    else if(std::holds_alternative<Not>(*f))
        return complexityN(std::get<Not>(*f));
    else
        return complexityB(std::get<Binary>(*f));
}

int main() {
    FormulaPtr p = ptr(Atom{"p"});
    FormulaPtr q = ptr(Atom{"q"});
    FormulaPtr r = ptr(Atom{"r"});
    FormulaPtr ls = ptr(Binary{Binary::And, p, q});
    FormulaPtr rs = ptr(Not{r});
    FormulaPtr f = ptr(Binary{Binary::Imp, ls, rs});

    print(f);
    std::cout << std::endl;

    std::cout << complexity(f) << std::endl;

    return 0;
}
