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
        if(is<True>(ls) || is<True>(rs))
            return ptr(True{});
        if(is<False>(ls))
            return rs;
        if(is<False>(rs))
            return ls;
        return ptr(Binary{Binary::Or, ls, rs});
    }
    if(b.type == Binary::Impl) {
        if(is<False>(ls) || is<True>(rs))
            return ptr(True{});
        if(is<True>(ls))
            return rs;
        if(is<False>(rs))
            return ptr(Not{ls});
        return ptr(Binary{Binary::Impl, ls, rs});
    }
    if(b.type == Binary::Eq) {
        if(is<True>(ls))
            return rs;
        if(is<True>(rs))
            return ls;
        if(is<False>(ls) && is<False>(rs))
            return ptr(True{});
        if(is<False>(ls))
            return ptr(Not{rs});
        if(is<False>(rs))
            return ptr(Not{ls});
        return ptr(Binary{Binary::Eq, ls, rs});
    }
    return FormulaPtr{};
}

FormulaPtr nnf(const FormulaPtr& f);

FormulaPtr nnfNot(const FormulaPtr& f) {
    if(is<Atom>(f))
        return ptr(Not{f});
    if(is<Not>(f))
        return nnf(as<Not>(f).subformula);
    auto b = as<Binary>(f);
    if(b.type == Binary::And)
        return ptr(Binary{Binary::Or, nnfNot(b.left), nnfNot(b.right)});
    if(b.type == Binary::Or)
        return ptr(Binary{Binary::And, nnfNot(b.left), nnfNot(b.right)});
    if(b.type == Binary::Impl)
        return ptr(Binary{Binary::And, nnf(b.left), nnfNot(b.right)});
    if(b.type == Binary::Eq)
        return ptr(Binary{
            Binary::Or,
            ptr(Binary{Binary::And, nnf(b.left), nnfNot(b.right)}),
            ptr(Binary{Binary::And, nnfNot(b.left), nnf(b.right)})
        });
}

FormulaPtr nnf(const FormulaPtr& f) {
   if(is<False>(f) || is<True>(f) || is<Atom>(f))
       return f;
   if(is<Not>(f))
       return nnfNot(as<Not>(f).subformula);
   auto b = as<Binary>(f);
   if(b.type == Binary::And)
       return ptr(Binary{Binary::And, nnf(b.left), nnf(b.right)});
   if(b.type == Binary::Or)
       return ptr(Binary{Binary::Or, nnf(b.left), nnf(b.right)});
   if(b.type == Binary::Impl)
       return ptr(Binary{Binary::Or, nnfNot(b.left), nnf(b.right)});
   if(b.type == Binary::Eq)
       return ptr(Binary{Binary::And,
                         ptr(Binary{Binary::Or, nnfNot(b.left), nnf(b.right)}),
                         ptr(Binary{Binary::Or, nnf(b.left), nnfNot(b.right)})
                  });
   return FormulaPtr{};
}

struct Literal {
    bool pos;
    std::string name;
};

using Clause = std::vector<Literal>;
using NormalForm = std::vector<Clause>;

template<typename List>
List concat(const List& l, const List& r) {
    List result;
    std::copy(begin(l), end(l), std::back_inserter(result));
    std::copy(begin(r), end(r), std::back_inserter(result));
    return result;
}

NormalForm cross(const NormalForm& l, const NormalForm& r) {
    NormalForm result;
    for(const auto& lc : l)
        for(const auto& rc : r)
            result.push_back(concat(lc, rc));
    return result;
}

NormalForm cnf(const FormulaPtr& f) {
    if(is<True>(f))
        return {};
    if(is<False>(f))
        return {{}};
    if(is<Atom>(f))
        return {{Literal{true, as<Atom>(f).name}}};
    if(is<Not>(f))
        return {{Literal{false, as<Atom>(as<Not>(f).subformula).name}}};
    auto b = as<Binary>(f);
    if(b.type == Binary::And)
        return concat(cnf(b.left), cnf(b.right));
    if(b.type == Binary::Or)
        return cross(cnf(b.left), cnf(b.right));
    return NormalForm{};
}

void print(const NormalForm& f) {
    for(const auto& clause : f) {
        std::cout << "[ ";
        for (const auto &literal: clause)
            std::cout << (literal.pos ? "" : "~") << literal.name << " ";
        std::cout << "]";
    }
    std::cout << std::endl;
}

// cetvrti cas
std::string tseitinRec(const FormulaPtr& f, int& subCount, NormalForm& cnf) {
    if(is<False>(f)) {
        std::string sub = "s" + std::to_string(++subCount);
        cnf.push_back({Literal{false, sub}});
        return sub;
    }
    if(is<True>(f)) {
        std::string sub = "s" + std::to_string(++subCount);
        cnf.push_back({Literal{true, sub}});
        return sub;
    }
    if(is<Atom>(f))
        return as<Atom>(f).name;
    if(is<Not>(f)) {
        std::string subformula = tseitinRec(as<Not>(f).subformula, subCount, cnf);
        std::string substitution = "s" + std::to_string(++subCount);
        cnf.push_back({Literal{false, subformula}, Literal{false, substitution}});
        cnf.push_back({Literal{true, subformula}, Literal{true, substitution}});
        return substitution;
    }
    Binary b = as<Binary>(f);
    std::string l = tseitinRec(b.left, subCount, cnf);
    std::string r = tseitinRec(b.right, subCount, cnf);
    std::string sub= "s" + std::to_string(++subCount);
    if(b.type == Binary::And) {
        cnf.push_back({Literal{false, sub}, Literal{true, l}});
        cnf.push_back({Literal{false, sub}, Literal{true, r}});
        cnf.push_back({Literal{true, sub}, Literal{false, l}, Literal{false, r}});
        return sub;
    }
    // isto za ostale slucajeve
}

NormalForm tseitin(const FormulaPtr& f) {
    NormalForm cnf;
    int subCount = 0;
    std::string sub = tseitinRec(f, subCount, cnf);
    cnf.push_back({Literal{true, sub}});
    return cnf;
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

    FormulaPtr T = ptr(True{});
    FormulaPtr F = ptr(False{});
    FormulaPtr pAndF = ptr(Binary{Binary::And, p, F});
    FormulaPtr FEqpAndF = ptr(Binary{Binary::Eq, F, pAndF});

    std::cout << "Formula: " << print(FEqpAndF) << std::endl;
    std::cout << "Simplified: " << print(simplify(FEqpAndF)) << std::endl;

    FormulaPtr notQ = ptr(Not{q});
    FormulaPtr pEqNotQ = ptr(Binary{Binary::Eq, p, notQ});
    FormulaPtr notFormula = ptr(Not{pEqNotQ});
    FormulaPtr nnfFormula = nnf(notFormula);
    NormalForm cnfFormula = cnf(nnfFormula);

    std::cout << "Formula: " << print(notFormula) << std::endl;
    std::cout << "NNF: " << print(nnfFormula) << std::endl;
    std::cout << "CNF: ";
    print(cnfFormula);

    return 0;
}












