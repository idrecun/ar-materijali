#ifndef FORMULA_H
#define FORMULA_H

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "interpretation.h"

struct Term;
using TermPtr = std::shared_ptr<Term>;

using VariableTerm = std::string;
struct FunctionTerm {
    std::string symbol;
    std::vector<TermPtr> args;
};

struct Term {
    enum Type {
        Variable,
        Function
    } type;

    union {
        VariableTerm variable;
        FunctionTerm function;
    };

    Term(VariableTerm t) : type(Variable), variable(t) {}
    Term(FunctionTerm t) : type(Function), function(t) {}
    ~Term() {
        switch(type) {
        case Variable: variable.~basic_string(); break;
        case Function: function.~FunctionTerm(); break;
        }
    }
};

TermPtr VariableT(std::string v);
TermPtr FunctionT(std::string s, const std::vector<TermPtr>& args);

struct Formula;
using FormulaPtr = std::shared_ptr<Formula>;

struct TrueData {};
struct FalseData {};
struct AtomData {
    std::string symbol;
    std::vector<TermPtr> args;
};
struct NotData { FormulaPtr subformula; };
struct BinaryData {
    enum Type {
        And,
        Or,
        Impl,
        Eql
    } type;

    FormulaPtr l, r;
};
struct QuantifierData {
    enum Type {
        All,
        Exists
    } type;

    std::string var;
    FormulaPtr subformula;
};

struct Formula {
    enum Type {
        True,
        False,
        Atom,
        Not,
        Binary,
        Quantifier
    } type;

    union {
        AtomData atom;
        NotData negation;
        BinaryData binary;
        QuantifierData quantifier;
    };

    Formula(TrueData)         : type(True) {}
    Formula(FalseData)        : type(False) {}
    Formula(AtomData f)       : type(Atom), atom(f) {}
    Formula(NotData f)        : type(Not), negation(f) {}
    Formula(BinaryData f)     : type(Binary), binary(f) {}
    Formula(QuantifierData f) : type(Quantifier), quantifier(f) {}
    ~Formula() {
        switch(type) {
        case Atom: atom.~AtomData(); break;
        case Not:  negation.~NotData(); break;
        case Binary: binary.~BinaryData(); break;
        case Quantifier: quantifier.~QuantifierData(); break;
        default: {}
        }
    }
};
FormulaPtr False();
FormulaPtr True();
FormulaPtr Atom(std::string p, const std::vector<TermPtr>& args);

FormulaPtr Not(FormulaPtr f);

FormulaPtr Binary(BinaryData::Type t, FormulaPtr l, FormulaPtr r);
FormulaPtr And(FormulaPtr l, FormulaPtr r);
FormulaPtr Or(FormulaPtr l, FormulaPtr r);
FormulaPtr Impl(FormulaPtr l, FormulaPtr r);
FormulaPtr Eql(FormulaPtr l, FormulaPtr r);

FormulaPtr Quantifier(QuantifierData::Type t, std::string v, FormulaPtr f);
FormulaPtr All(std::string v, FormulaPtr f);
FormulaPtr Exists(std::string v, FormulaPtr f);


// --- Cas 6 ---
bool checkSignature(TermPtr term, const Signature& s);
bool checkSignature(FormulaPtr formula, const Signature& s);

unsigned evaluate(TermPtr term, const LStructure& s, const Valuation& val);
bool evaluate(FormulaPtr formula, const LStructure& s, const Valuation& val);

TermPtr substitute(TermPtr term, const std::string& v, TermPtr t);
FormulaPtr substitute(FormulaPtr formula, const std::string& v, TermPtr t);

void getVariables(TermPtr term, std::set< std::string >& vars);
void getVariables(FormulaPtr formula, std::set< std::string >& vars, bool free = false);

bool containsVariable(TermPtr term, const std::string& v);
bool containsVariable(FormulaPtr formula, const std::string& v, bool free = false);

void print(TermPtr term);
void print(FormulaPtr formula);

template<typename T1, typename T2>
std::string uniqueVariable(T1 f, T2 g) {
    unsigned uniqueCounter = 0;
    std::string uniqueVar;
    do {
        uniqueVar = "u" + std::to_string(++uniqueCounter);
    } while(containsVariable(f, uniqueVar) || containsVariable(g, uniqueVar));
    return uniqueVar;
}

// --- Cas 7 ---
FormulaPtr simplify(FormulaPtr formula);
FormulaPtr nnf(FormulaPtr formula);
FormulaPtr pullQuantifiers(FormulaPtr formula);
FormulaPtr prenex(FormulaPtr formula);
FormulaPtr skolem(FormulaPtr formula, Signature& s);

// --- Cas 8 ---
bool equal(TermPtr t1, TermPtr t2);
bool equal(FormulaPtr f1, FormulaPtr f2);

#endif // FORMULA_H
