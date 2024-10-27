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

bool checkSignature(TermPtr term, const Signature& s);
bool checkSignature(FormulaPtr formula, const Signature& s);

unsigned evaluate(TermPtr term, const LStructure& s, const Valuation& val);
bool evaluate(FormulaPtr formula, const LStructure& s, const Valuation& val);

TermPtr substitute(TermPtr term, const std::string& v, TermPtr t);
FormulaPtr substitute(FormulaPtr formula, const std::string& v, TermPtr t);

void getVariables(TermPtr term, std::set< std::string >& vars);
void getVariables(FormulaPtr formula, std::set< std::string >& vars, bool free);

bool containsVariable(TermPtr term, const std::string& v);
bool containsVariable(FormulaPtr formula, const std::string& v, bool free);

void print(TermPtr term);
void print(FormulaPtr formula);


#endif // FORMULA_H
