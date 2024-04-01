#include <iostream>
#include <memory>
#include <variant>
#include <map>
#include <set>
#include <optional>

using Valuation = std::map<std::string, bool>;
using AtomSet = std::set<std::string>;

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

// Helper functions

FormulaPtr ptr(Formula f) { return std::make_shared<Formula>(f); }

template<typename T>
bool is(FormulaPtr f) { return std::holds_alternative<T>(*f); }

template<typename T>
T as(FormulaPtr f) { return std::get<T>(*f); }

template<typename R, typename F, typename T, typename A, typename N, typename B, typename... Args>
R match(F visitF, T visitT, A visitA, N visitN, B visitB, FormulaPtr f, Args&&... args) {
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

template<typename R, typename A, typename O, typename I, typename E, typename... Args>
R match(A visitA, O visitO, I visitI, E visitE, Binary b, Args&&... args) {
    switch(b.type) {
        case Binary::And: return visitA(b.l, b.r, std::forward<Args>(args)...);
        case Binary::Or:  return visitO(b.l, b.r, std::forward<Args>(args)...);
        case Binary::Imp: return visitI(b.l, b.r, std::forward<Args>(args)...);
        case Binary::Eq:  return visitE(b.l, b.r, std::forward<Args>(args)...);
    }
    return R();
}

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
    match<void>(printF, printT, printA, printN, printB, f);
}

unsigned complexity(FormulaPtr);
unsigned complexityF() { return 0; }
unsigned complexityT() { return 0; }
unsigned complexityA(Atom) { return 0; }
unsigned complexityN(Not n) { return 1 + complexity(n.sub); }
unsigned complexityB(Binary b) { return 1 + complexity(b.l) + complexity(b.r); }
unsigned complexity(FormulaPtr f) {
    return match<unsigned>(complexityF, complexityT, complexityA, complexityN, complexityB, f);
}

bool eval(FormulaPtr, Valuation&);
bool evalF(Valuation&) { return false; }
bool evalT(Valuation&) { return true; }
bool evalA(Atom a, Valuation& v) { return v[a.name]; }
bool evalN(Not n, Valuation& v) { return !eval(n.sub, v); }
bool evalAnd(FormulaPtr l, FormulaPtr r, Valuation& v) { return eval(l, v) && eval(r, v); }
bool evalOr(FormulaPtr l, FormulaPtr r, Valuation& v) { return eval(l, v) || eval(r, v); }
bool evalImp(FormulaPtr l, FormulaPtr r, Valuation& v) { return !eval(l, v) || eval(r, v); }
bool evalEq(FormulaPtr l, FormulaPtr r, Valuation& v) { return eval(l, v) == eval(r, v); }
bool evalB(Binary b, Valuation& v) {
    return match<bool>(evalAnd, evalOr, evalImp, evalEq,
                       b, v);
}
bool eval(FormulaPtr f, Valuation& v) {
    return match<bool>(evalF, evalT, evalA, evalN, evalB,
                       f, v);
}

bool equal(FormulaPtr f, FormulaPtr g);
bool equalF(FormulaPtr g) { return is<False>(g); };
bool equalT(FormulaPtr g) { return is<True>(g);  };
bool equalA(Atom a, FormulaPtr g) { return is<Atom>(g) && a.name == as<Atom>(g).name; }
bool equalN(Not n, FormulaPtr g) { return is<Not>(g) && equal(n.sub, as<Not>(g).sub); }
bool equalB(Binary b, FormulaPtr g) {
    if(!is<Binary>(g))
        return false;
    Binary gb = as<Binary>(g);
    return b.type == gb.type && equal(b.l, gb.l) && equal(b.r, gb.r);
}
bool equal(FormulaPtr f, FormulaPtr g) {
    return match<bool>(equalF, equalT, equalA, equalN, equalB, f, g);
}

// f    = (p & q) -> ~r
// what = ~r
// with = r | p
// res  = (p & q) -> (r | p)
FormulaPtr substitute(FormulaPtr f, FormulaPtr what, FormulaPtr with);
FormulaPtr substituteF(FormulaPtr what, FormulaPtr with) { return ptr(False{}); }
FormulaPtr substituteT(FormulaPtr what, FormulaPtr with) { return ptr(True{}); }
FormulaPtr substituteA(Atom a, FormulaPtr what, FormulaPtr with) { return ptr(a); }
FormulaPtr substituteN(Not n, FormulaPtr what, FormulaPtr with) { return ptr(Not{ substitute(n.sub, what, with) }); }
FormulaPtr substituteB(Binary b, FormulaPtr what, FormulaPtr with) {
    return ptr(Binary{
                   b.type,
                   substitute(b.l, what, with),
                   substitute(b.r, what, with)
               });
}
FormulaPtr substitute(FormulaPtr f, FormulaPtr what, FormulaPtr with) {
    if(equal(f, what))
        return with;
    return match<FormulaPtr>(substituteF, substituteT, substituteA, substituteN, substituteB, f, what, with);
}

void getAtoms(FormulaPtr f, AtomSet& atoms);
void getAtomsF(AtomSet&) {}
void getAtomsT(AtomSet&) {}
void getAtomsA(Atom a, AtomSet& atoms) { atoms.insert(a.name); }
void getAtomsN(Not n, AtomSet& atoms) { getAtoms(n.sub, atoms); }
void getAtomsB(Binary b, AtomSet& atoms) { getAtoms(b.l, atoms); getAtoms(b.r, atoms); }
void getAtoms(FormulaPtr f, AtomSet& atoms) {
    match<void>(getAtomsF, getAtomsT, getAtomsA, getAtomsN, getAtomsB, f, atoms);
}

void print(Valuation& val) {
    for(auto& [atom, v] : val)
        std::cout << v << ' ';
}

// pqrs
// 0010
// 0011
// 0100

bool next(Valuation& val) {
    auto it = begin(val);
    while(it != end(val) && it->second) {
        it->second = false;
        it++;
    }

    if(it == end(val))
        return false;

    return it->second = true;
}

void table(FormulaPtr f) {
    AtomSet atoms;
    getAtoms(f, atoms);

    Valuation val;
    for(auto atom : atoms) {
        val[atom] = false;
        std::cout << atom << ' ';
    }
    std::cout << std::endl;

    do {
        print(val);
        std::cout << "| ";
        std::cout << eval(f, val) << std::endl;
    } while(next(val));
}

std::optional<Valuation> isSatisfiable(FormulaPtr f) {
    AtomSet atoms;
    getAtoms(f, atoms);

    Valuation val;
    for(auto atom : atoms)
        val[atom] = false;

    do {
        if(eval(f, val))
            return val;
    } while(next(val));
    return {};
}

FormulaPtr simplify(FormulaPtr);
FormulaPtr simplifyF() { return ptr(False{}); }
FormulaPtr simplifyT() { return ptr(True{}); }
FormulaPtr simplifyA(Atom a) { return ptr(a); }
FormulaPtr simplifyN(Not n) {
    FormulaPtr sub_simplified = simplify(n.sub);
    if(is<True>(sub_simplified))
        return ptr(False{});
    if(is<False>(sub_simplified))
        return ptr(True{});
    return ptr(Not{sub_simplified});
}
FormulaPtr simplifyAnd(FormulaPtr l, FormulaPtr r) {
    FormulaPtr l_simplified = simplify(l);
    FormulaPtr r_simplified = simplify(r);

    if(is<False>(l_simplified) || is<False>(r_simplified))
        return ptr(False{});
    if(is<True>(l_simplified))
        return r_simplified;
    if(is<True>(r_simplified))
        return l_simplified;

    return ptr(Binary{Binary::And, l_simplified, r_simplified});
}
FormulaPtr simplifyOr(FormulaPtr l, FormulaPtr r) {
    FormulaPtr l_simplified = simplify(l);
    FormulaPtr r_simplified = simplify(r);

    if(is<True>(l_simplified) || is<True>(r_simplified))
        return ptr(True{});
    if(is<False>(l_simplified))
        return r_simplified;
    if(is<False>(r_simplified))
        return l_simplified;

    return ptr(Binary{Binary::Or, l_simplified, r_simplified});
}
FormulaPtr simplifyImp(FormulaPtr l, FormulaPtr r) {
    FormulaPtr l_simplified = simplify(l);
    FormulaPtr r_simplified = simplify(r);

    if(is<False>(l_simplified) || is<True>(r_simplified))
        return ptr(True{});
    if(is<True>(l_simplified))
        return r_simplified;
    if(is<False>(r_simplified))
        return ptr(Not{l_simplified});

    return ptr(Binary{Binary::Imp, l_simplified, r_simplified});
}
FormulaPtr simplifyEq(FormulaPtr l, FormulaPtr r) {
    FormulaPtr l_simplified = simplify(l);
    FormulaPtr r_simplified = simplify(r);

    if(is<True>(l_simplified))
        return r_simplified;
    if(is<True>(r_simplified))
        return l_simplified;
    if(is<False>(l_simplified) && is<False>(r_simplified))
        return ptr(True{});
    if(is<False>(l_simplified))
        return ptr(Not{r_simplified});
    if(is<False>(r_simplified))
        return ptr(Not{l_simplified});

    return ptr(Binary{Binary::Eq, l_simplified, r_simplified});
}
FormulaPtr simplifyB(Binary b) {
    return match<FormulaPtr>(simplifyAnd, simplifyOr, simplifyImp, simplifyEq, b);
}
FormulaPtr simplify(FormulaPtr f) {
    return match<FormulaPtr>(simplifyF, simplifyT, simplifyA, simplifyN, simplifyB, f);
}

FormulaPtr nnf(FormulaPtr);
FormulaPtr nnfNot(FormulaPtr);
FormulaPtr nnfNotF() {}
FormulaPtr nnfNotT() {}
FormulaPtr nnfNotA(Atom a) { return ptr(Not{ptr(a)}); }
FormulaPtr nnfNotN(Not n) { return nnf(n.sub); }
FormulaPtr nnfNotAnd(FormulaPtr l, FormulaPtr r) {
    return ptr(Binary{Binary::Or, nnfNot(l), nnfNot(r)});
}
FormulaPtr nnfNotOr(FormulaPtr l, FormulaPtr r) {
    return ptr(Binary{Binary::And, nnfNot(l), nnfNot(r)});
}
FormulaPtr nnfNotImp(FormulaPtr l, FormulaPtr r) {
    return ptr(Binary{Binary::And, nnf(l), nnfNot(r)});
}
FormulaPtr nnfNotEq(FormulaPtr l, FormulaPtr r) {
    return ptr(Binary{
                   Binary::Or,
                   ptr(Binary{Binary::And, nnf(l), nnfNot(r)}),
                   ptr(Binary{Binary::And, nnfNot(l), nnf(r)})
               });
}
FormulaPtr nnfNotB(Binary b) { return match<FormulaPtr>(nnfNotAnd, nnfNotOr, nnfNotImp, nnfNotEq, b); }
FormulaPtr nnfNot(FormulaPtr f) { return match<FormulaPtr>(nnfNotF, nnfNotT, nnfNotA, nnfNotN, nnfNotB, f); }

FormulaPtr nnfF() { return ptr(False{}); }
FormulaPtr nnfT() { return ptr(True{}); }
FormulaPtr nnfA(Atom a) { return ptr(a); }
FormulaPtr nnfN(Not n) { return nnfNot(n.sub); }
FormulaPtr nnfAnd(FormulaPtr l, FormulaPtr r) {
    return ptr(Binary{Binary::And, nnf(l), nnf(r)});
}
FormulaPtr nnfOr(FormulaPtr l, FormulaPtr r) {
    return ptr(Binary{Binary::Or, nnf(l), nnf(r)});
}
FormulaPtr nnfImp(FormulaPtr l, FormulaPtr r) {
    return ptr(Binary{Binary::Or, nnfNot(l), nnf(r)});
}
FormulaPtr nnfEq(FormulaPtr l, FormulaPtr r) {
    return ptr(Binary{
                   Binary::And,
                   ptr(Binary{Binary::Or, nnfNot(l), nnf(r)}),
                   ptr(Binary{Binary::Or, nnf(l), nnfNot(r)})
               });
}
FormulaPtr nnfB(Binary b) { return match<FormulaPtr>(nnfAnd, nnfOr, nnfImp, nnfEq, b); }
FormulaPtr nnf(FormulaPtr f) { return match<FormulaPtr>(nnfF, nnfT, nnfA, nnfN, nnfB, f); }

struct Literal {
    bool pos;
    std::string name;
};

using Clause = std::vector<Literal>;
using NormalForm = std::vector<Clause>;

template<typename List>
List concat(List a, List b) {
    List res;
    std::copy(begin(a), end(a), std::back_inserter(res));
    std::copy(begin(b), end(b), std::back_inserter(res));
    return res;
}

NormalForm cross(NormalForm a, NormalForm b) {
    NormalForm res;
    for(auto& ca : a)
        for(auto& cb : b)
            res.push_back(concat(ca, cb));
    return res;
}

NormalForm cnf(FormulaPtr);
NormalForm cnfF() { return {{}}; }
NormalForm cnfT() { return {}; }
NormalForm cnfA(Atom a) { return {{ Literal{true, a.name} }}; }
NormalForm cnfN(Not n) { return {{ Literal{false, as<Atom>(n.sub).name} }}; }
NormalForm cnfAnd(FormulaPtr l, FormulaPtr r) { return concat(cnf(l), cnf(r)); }
NormalForm cnfOr(FormulaPtr l, FormulaPtr r) { return cross(cnf(l), cnf(r)); }
NormalForm cnfImp(FormulaPtr, FormulaPtr) { throw std::runtime_error("Unreachable code reached."); }
NormalForm cnfEq(FormulaPtr, FormulaPtr) { throw std::runtime_error("Unreachable code reached."); }
NormalForm cnfB(Binary b) { return match<NormalForm>(cnfAnd, cnfOr, cnfImp, cnfEq, b); }
NormalForm cnf(FormulaPtr f) { return match<NormalForm>(cnfF, cnfT, cnfA, cnfN, cnfB, f); }

void print(NormalForm f) {
    std::cout << "{";
    for(auto& clause : f) {
        std::cout << "{ ";
        for(auto& literal : clause) {
            if(!literal.pos)
                std::cout << "~";
            std::cout << literal.name;
            std::cout << " ";
        }
        std::cout << "} ";
    }
    std::cout << "}";
}

// p & q -> ~r
// p & (q | F) -> (r | T)
int main() {
    FormulaPtr p = ptr(Atom{"p"});
    FormulaPtr q = ptr(Atom{"q"});
    FormulaPtr r = ptr(Atom{"r"});
    FormulaPtr ls = ptr(Binary{Binary::Or, p, q});
    FormulaPtr rs = ptr(Not{r});
    FormulaPtr f = ptr(Binary{Binary::Imp, ls, rs});
    FormulaPtr nf = ptr(Not{f});

    print(nf);
    std::cout << std::endl;
    print(nnf(nf));
    std::cout << std::endl;
    print(cnf(nnf(ptr(Not{nf}))));
    std::cout << std::endl;

    FormulaPtr T = ptr(True{});
    FormulaPtr F = ptr(False{});
    FormulaPtr q_or_F = ptr(Binary{Binary::Or, q, F});
    FormulaPtr p_and_qof = ptr(Binary{Binary::And, p, q_or_F});
    FormulaPtr r_or_T = ptr(Binary{Binary::Or, F, r});
    FormulaPtr g = ptr(Binary{Binary::Imp, p_and_qof, r_or_T});

    print(g);
    std::cout << std::endl;
    print(simplify(g));
    std::cout << std::endl;

    return 0;
}
