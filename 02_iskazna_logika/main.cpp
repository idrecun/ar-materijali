#include <iostream>
#include <variant>
#include <memory>
#include <map>
#include <set>

// Structures for defining a Formula
struct False;
struct True;
struct Atom;
struct Not;
struct Binary;

using Formula = std::variant<False, True, Atom, Not, Binary>;
using FormulaPtr = std::shared_ptr<Formula>;

struct False {};
struct True {};
struct Atom { std::string name; };
struct Not { FormulaPtr subformula; };
struct Binary {
    enum Type { And, Or, Impl, Eq } type;
    FormulaPtr left, right;
};

FormulaPtr ptr(const Formula& f) { return std::make_shared<Formula>(f); }

template<typename T>
bool is(const FormulaPtr& f) { return std::holds_alternative<T>(*f);}

template<typename T>
T as(const FormulaPtr& f) { return std::get<T>(*f); }

// Valuation
using Valuation = std::map<std::string, bool>;

// AtomSet
using AtomSet = std::set<std::string>;

int complexity(const FormulaPtr& f) {
    if(is<False>(f) || is<True>(f) || is<Atom>(f))
        return 0;
    if(is<Not>(f))
        return 1 + complexity(as<Not>(f).subformula);
    if(is<Binary>(f))
        return 1 + complexity(as<Binary>(f).left) + complexity(as<Binary>(f).right);
    return 0;
}

std::string print(const FormulaPtr& f) {
    if(is<False>(f)) return "F";
    if(is<True>(f))  return "T";
    if(is<Atom>(f))  return as<Atom>(f).name;
    if(is<Not>(f))   return "~" + print(as<Not>(f).subformula);
    if(is<Binary>(f)) {
        std::string sign;
        switch(as<Binary>(f).type) {
            case Binary::And:  sign = "&";   break;
            case Binary::Or:   sign = "|";   break;
            case Binary::Impl: sign = "->";  break;
            case Binary::Eq:   sign = "<->"; break;
        }
        return "(" + print(as<Binary>(f).left) + " " + sign + " " + print(as<Binary>(f).right) + ")";
    }
    return "";
}

bool evaluate(const FormulaPtr& f, Valuation& v) {
    if(is<False>(f))
        return false;
    if(is<True>(f))
        return true;
    if(is<Atom>(f))
        return v[as<Atom>(f).name];
    if(is<Not>(f))
        return !evaluate(as<Not>(f).subformula, v);
    if(is<Binary>(f)) {
        bool evalL = evaluate(as<Binary>(f).left, v);
        bool evalR = evaluate(as<Binary>(f).right, v);
        switch(as<Binary>(f).type) {
            case Binary::And:  return evalL && evalR;
            case Binary::Or:   return evalL || evalR;
            case Binary::Impl: return !evalL || evalR;
            case Binary::Eq:   return evalL == evalR;
        }
    }
    return false;
}

bool equal(const FormulaPtr& f, const FormulaPtr& g) {
    if(f->index() != g->index())
        return false;

    if(is<False>(f) || is<True>(f))
        return true;
    if(is<Atom>(f))
        return as<Atom>(f).name == as<Atom>(g).name;
    if(is<Not>(f))
        return equal(as<Not>(f).subformula, as<Not>(g).subformula);
    if(is<Binary>(f)) {
        return as<Binary>(f).type == as<Binary>(g).type &&
               equal(as<Binary>(f).left, as<Binary>(g).left) &&
               equal(as<Binary>(f).right, as<Binary>(g).right);
    }
    return false;
}

// Drugi cas

FormulaPtr substitute(const FormulaPtr& f, const FormulaPtr& what, const FormulaPtr& with) {
    if(equal(f, what))
        return with;
    if(is<False>(f) || is<True>(f) || is<Atom>(f))
        return f;
    if(is<Not>(f))
        return ptr(Not{substitute(as<Not>(f).subformula, what, with)});
    if(is<Binary>(f)) {
        auto binaryF = as<Binary>(f);
        return ptr(Binary{
            binaryF.type,
            substitute(binaryF.left, what, with),
            substitute(binaryF.right, what, with)
        });
    }
    return FormulaPtr{};
}

void getAtoms(const FormulaPtr& f, AtomSet& atoms) {
    if(is<Atom>(f))
        atoms.insert(as<Atom>(f).name);
    else if(is<Not>(f))
        getAtoms(as<Not>(f).subformula, atoms);
    else if(is<Binary>(f)) {
        getAtoms(as<Binary>(f).left, atoms);
        getAtoms(as<Binary>(f).right, atoms);
    }
}

bool next(Valuation& v) {
    auto it = begin(v);
    while(it != end(v) && it->second) {
        it->second = false;
        it++;
    }

    if(it == end(v))
        return false;

    return it->second = true;
}

void print(Valuation& v) {
    for(const auto& [atom, value] : v)
        std::cout << value << ' ';
}

void table(const FormulaPtr& f) {
    AtomSet atoms;
    getAtoms(f, atoms);

    Valuation v;
    for(const std::string& atom : atoms) {
        v[atom] = false;
        std::cout << atom << ' ';
    }
    std::cout << std::endl;

    do {
        print(v);
        std::cout << "| " << evaluate(f, v) << std::endl;
    } while(next(v));
}

std::optional<Valuation> isSatisfiable(const FormulaPtr& f) {
    AtomSet atoms;
    getAtoms(f, atoms);

    Valuation v;
    for(const auto& atom : atoms)
        v[atom] = false;

    do {
        if(evaluate(f, v))
            return v;
    } while(next(v));
    return {};
}

FormulaPtr simplify(const FormulaPtr& f) {
    if(is<False>(f) || is<True>(f) || is<Atom>(f))
        return f;
    if(is<Not>(f)) {
        FormulaPtr s = simplify(as<Not>(f).subformula);
        if(is<True>(s))
            return ptr(False{});
        if(is<False>(s))
            return ptr(True{});
        return ptr(Not{s});
    }
    auto b = as<Binary>(f);
    FormulaPtr ls = simplify(b.left);
    FormulaPtr rs = simplify(b.right);
    if(b.type == Binary::And) {
        if(is<False>(ls) || is<False>(rs))
            return ptr(False{});
        if(is<True>(ls))
            return rs;
        if(is<True>(rs))
            return ls;
        return ptr(Binary{Binary::And, ls, rs});
    }
    if(b.type == Binary::Or) {

    }
    if(b.type == Binary::Impl) {

    }
    if(b.type == Binary::Eq) {

    }
}

int main() {
    FormulaPtr p = ptr(Atom{"p"});
    FormulaPtr q = ptr(Atom{"q"});
    FormulaPtr pAndq = ptr(Binary{Binary::And, p, q});
    std::cout << complexity(pAndq) << std::endl;
    std::cout << print(pAndq) << std::endl;

    Valuation v = {{"p", true}, {"q", false}};
    std::cout << (evaluate(pAndq, v) ? "True" : "False") << std::endl;

    table(pAndq);

    auto satV = isSatisfiable(pAndq);
    if(satV) {
        std::cout << "SAT for valuation: ";
        print(satV.value());
        std::cout << std::endl;
    }
    else
        std::cout << "UNSAT" << std::endl;

    FormulaPtr unsat = ptr(False{});
    auto unsatV = isSatisfiable(unsat);
    if(unsatV) {
        std::cout << "SAT for valuation: ";
        print(unsatV.value());
        std::cout << std::endl;
    }
    else
        std::cout << "UNSAT" << std::endl;

    return 0;
}












