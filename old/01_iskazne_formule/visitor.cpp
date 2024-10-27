/*
* Alternativni nacin za implementaciju funkcija nad formulom.
* Koristi tzv. "visitor" sablon.
*/

#include <iostream>
#include <memory>
#include <variant>
#include <map>

using Valuation = std::map<std::string, bool>;

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

template<typename R, typename... Args>
struct Visitor {
    virtual R visitF(Args&&...) = 0;
    virtual R visitT(Args&&...) = 0;
    virtual R visitA(Atom, Args&&...) = 0;
    virtual R visitN(Not, Args&&...) = 0;
    virtual R visitB(Binary, Args&&...) = 0;

    R visit(FormulaPtr f, Args&&... args) {
        if(std::holds_alternative<False>(*f))
            return visitF(std::forward<Args>(args)...);
        if(std::holds_alternative<True>(*f))
            return visitT(std::forward<Args>(args)...);
        if(std::holds_alternative<Atom>(*f))
            return visitA(std::get<Atom>(*f), std::forward<Args>(args)...);
        if(std::holds_alternative<Not>(*f))
            return visitN(std::get<Not>(*f), std::forward<Args>(args)...);
        if(std::holds_alternative<Binary>(*f))
            return visitB(std::get<Binary>(*f), std::forward<Args>(args)...);
        return R();
    }
};

struct Print : Visitor<void> {
    void visitF() { std::cout << "F"; }
    void visitT() { std::cout << "T"; }
    void visitA(Atom a) { std::cout << a.name; }
    void visitN(Not n) { std::cout << "~"; visit(n.sub); }
    void visitB(Binary b) { 
        std::cout << "(";
        visit(b.l);
        std::cout << sign(b.type);
        visit(b.r);
        std::cout << ")";
    }
};
void print(FormulaPtr f) {
    Print visitor;
    return visitor.visit(f);
}

struct Complexity : Visitor<unsigned> {
    unsigned visitF() { return 0; }
    unsigned visitT() { return 0; }
    unsigned visitA(Atom) { return 0; }
    unsigned visitN(Not n) { return 1 + visit(n.sub); }
    unsigned visitB(Binary b) { return 1 + visit(b.l) + visit(b.r); }
};
unsigned complexity(FormulaPtr f) {
    Complexity visitor;
    return visitor.visit(f);
}

struct Eval : Visitor<bool, Valuation&> {
    bool visitF(Valuation& v) { return true; }
    bool visitT(Valuation& v) { return false; }
    bool visitA(Atom a, Valuation& v) { return v[a.name]; }
    bool visitN(Not n, Valuation& v) { return !visit(n.sub, v); }
    bool visitB(Binary b, Valuation& v) {
        bool leval = visit(b.l, v);
        bool reval = visit(b.r, v);
        switch(b.type) {
            case Binary::And: return  leval && reval;
            case Binary::Or:  return  leval || reval;
            case Binary::Imp: return !leval || reval;
            case Binary::Eq:  return  leval == reval;
        }
        return false;
    }
};
bool eval(FormulaPtr f, Valuation& v) {
    Eval visitor;
    return visitor.visit(f, v);
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
