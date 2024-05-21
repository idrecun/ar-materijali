#include "formula.h"
#include <iostream>

TermPtr VariableT(std::string v) { return std::make_shared<Term>(VariableTerm{v}); }
TermPtr FunctionT(std::string s, const std::vector<TermPtr>& args)
{ return std::make_shared<Term>(FunctionTerm{s, args}); }

FormulaPtr False() { return std::make_shared<Formula>(FalseData{}); }
FormulaPtr True() { return std::make_shared<Formula>(TrueData{}); }
FormulaPtr Atom(std::string p, const std::vector<TermPtr>& args)
{ return std::make_shared<Formula>(AtomData{p, args}); }

FormulaPtr Not(FormulaPtr f) { return std::make_shared<Formula>(NotData{f}); }

FormulaPtr Binary(BinaryData::Type t, FormulaPtr l, FormulaPtr r)
{ return std::make_shared<Formula>(BinaryData{t, l, r}); }
FormulaPtr And(FormulaPtr l, FormulaPtr r) { return Binary(BinaryData::And, l, r); }
FormulaPtr Or(FormulaPtr l, FormulaPtr r) { return Binary(BinaryData::Or, l, r); }
FormulaPtr Impl(FormulaPtr l, FormulaPtr r) { return Binary(BinaryData::Impl, l, r); }
FormulaPtr Eql(FormulaPtr l, FormulaPtr r) { return Binary(BinaryData::Eql, l, r); }

FormulaPtr Quantifier(QuantifierData::Type t, std::string v, FormulaPtr f)
{ return std::make_shared<Formula>(QuantifierData{t, v, f}); }
FormulaPtr All(std::string v, FormulaPtr f) { return Quantifier(QuantifierData::All, v, f); }
FormulaPtr Exists(std::string v, FormulaPtr f) { return Quantifier(QuantifierData::Exists, v, f); }

bool evaluate(FormulaPtr formula, const LStructure& s, const Valuation& val) {
    switch(formula->type) {
    case Formula::True:  return true;
    case Formula::False: return false;
    case Formula::Atom:  {
        std::vector<unsigned> args;
        for(const auto& term : formula->atom.args)
            args.push_back(evaluate(term, s, val));
        return s.relations.at(formula->atom.symbol)(args);
    }
    case Formula::Not:   return !evaluate(formula->negation.subformula, s, val);
    case Formula::Binary: {
        bool lEval = evaluate(formula->binary.l, s, val);
        bool rEval = evaluate(formula->binary.r, s, val);
        switch(formula->binary.type) {
        case BinaryData::And:  return lEval && rEval;
        case BinaryData::Or:   return lEval || rEval;
        case BinaryData::Impl: return !lEval || rEval;
        case BinaryData::Eql:  return lEval == rEval;
        }
    } break;
    case Formula::Quantifier: {
        std::string var = formula->quantifier.var;
        FormulaPtr subformula = formula->quantifier.subformula;

        switch(formula->quantifier.type) {
        case QuantifierData::All: {
            Valuation valQuantified(val);
            for(const auto& value : s.domain) {
                valQuantified[var] = value;
                if(!evaluate(subformula, s, valQuantified))
                    return false;
            }
            return true;
        }
        case QuantifierData::Exists: {
            Valuation valQuantified(val);
            for(const auto& value : s.domain) {
                valQuantified[var] = value;
                if(evaluate(subformula, s, valQuantified))
                    return true;
            }
            return false;
        }
        }
    }
    }
}

unsigned evaluate(TermPtr term, const LStructure &s, const Valuation &val) {
    switch(term->type) {
    case Term::Variable: return val.at(term->variable);
    case Term::Function:
        std::vector<unsigned> args;
        for(const auto& subterm : term->function.args)
            args.push_back(evaluate(subterm, s, val));
        return s.functions.at(term->function.symbol)(args);
    }
}

bool checkSignature(TermPtr term, const Signature &s) {
    switch(term->type) {
    case Term::Variable: return true;
    case Term::Function:
        auto it = s.fun.find(term->function.symbol);
        if(it == s.fun.end() || it->second != term->function.args.size())
            return false;

        for(const auto& subterm : term->function.args)
            if(!checkSignature(subterm, s))
                return false;
        return true;
    }
}

bool checkSignature(FormulaPtr formula, const Signature &s) {
    switch(formula->type) {
    case Formula::True:
    case Formula::False:
        return true;
    case Formula::Atom: {
        auto it = s.rel.find(formula->atom.symbol);
        if(it == s.rel.end() || it->second != formula->atom.args.size())
            return false;
        for(const auto& term : formula->atom.args)
            if(!checkSignature(term, s))
                return false;
        return true;
    }
    case Formula::Not:
        return checkSignature(formula->negation.subformula, s);
    case Formula::Binary:
        return checkSignature(formula->binary.l, s) && checkSignature(formula->binary.r, s);
    case Formula::Quantifier:
        return checkSignature(formula->quantifier.subformula, s);
    }
}

TermPtr substitute(TermPtr term, const std::string &v, TermPtr t) {
    switch(term->type) {
    case Term::Variable: return term->variable == v ? t : term;
    case Term::Function:
        std::vector<TermPtr> subterms;
        for(const auto& subterm : term->function.args)
            subterms.push_back(substitute(subterm, v, t));
        return std::make_shared<Term>(FunctionTerm{term->function.symbol, subterms});
    }
}

FormulaPtr substitute(FormulaPtr formula, const std::string &v, TermPtr t) {
    switch(formula->type) {
    case Formula::True:
    case Formula::False:
        return formula;
    case Formula::Atom: {
        std::vector<TermPtr> subterms;
        for(const auto& subterm : formula->atom.args)
            subterms.push_back(substitute(subterm, v, t));
        return std::make_shared<Formula>(AtomData{formula->atom.symbol, subterms});
    }
    case Formula::Not:
        return std::make_shared<Formula>(NotData{substitute(formula->negation.subformula, v, t)});
    case Formula::Binary:
        return std::make_shared<Formula>(BinaryData{
                                             formula->binary.type,
                                             substitute(formula->binary.l, v, t),
                                             substitute(formula->binary.r, v, t)
                                         });
    case Formula::Quantifier: {
        if(formula->quantifier.var == v)
            return formula;
        if(containsVariable(t, formula->quantifier.var)) {
            std::string uniqueVar = uniqueVariable(formula, t);
            TermPtr varTerm = std::make_shared<Term>(VariableTerm{uniqueVar});
            FormulaPtr subformula = substitute(formula->quantifier.subformula, formula->quantifier.var, varTerm);
            return std::make_shared<Formula>(QuantifierData{
                                                 formula->quantifier.type,
                                                 uniqueVar,
                                                 substitute(subformula, v, t)
                                             });
        }
        return std::make_shared<Formula>(QuantifierData{
                                                  formula->quantifier.type,
                                                  formula->quantifier.var,
                                                  substitute(formula->quantifier.subformula, v, t)
                                              });
    }
    }
}

bool containsVariable(TermPtr term, const std::string &v) {
    std::set<std::string> vars;
    getVariables(term, vars);
    return vars.find(v) != vars.end();
}

bool containsVariable(FormulaPtr formula, const std::string &v, bool free) {
    std::set<std::string> vars;
    getVariables(formula, vars, free);
    return vars.find(v) != vars.end();
}

void getVariables(TermPtr term, std::set<std::string> &vars) {
    switch(term->type) {
    case Term::Variable: vars.insert(term->variable); break;
    case Term::Function:
        for(const auto& subterm : term->function.args)
            getVariables(subterm, vars);
    }
}

void getVariables(FormulaPtr formula, std::set<std::string> &vars, bool free) {
    switch(formula->type) {
    case Formula::True:
    case Formula::False:
        break;
    case Formula::Atom:
        for(const auto& subterm : formula->atom.args)
            getVariables(subterm, vars);
        break;
    case Formula::Not:
        getVariables(formula->negation.subformula, vars, free);
        break;
    case Formula::Binary:
        getVariables(formula->binary.l, vars, free);
        getVariables(formula->binary.r, vars, free);
        break;
    case Formula::Quantifier:
        if(!free) {
            getVariables(formula->quantifier.subformula, vars, free);
            vars.insert(formula->quantifier.var);
        }
        else {
            bool isVarFree = vars.find(formula->quantifier.var) != vars.end();
            getVariables(formula->quantifier.subformula, vars, free);
            if(!isVarFree)
                vars.erase(formula->quantifier.var);
        }
    }
}

void print(TermPtr term) {
    switch(term->type) {
    case Term::Variable:
        std::cout << term->variable;
        break;
    case Term::Function:
        std::cout << term->function.symbol;
        if(!term->function.args.empty()) {
            std::cout << "(";
            print(term->function.args[0]);
            for(auto it = term->function.args.begin() + 1; it != term->function.args.end(); it++) {
                std::cout << ", ";
                print(*it);
            }
            std::cout << ")";
        }
    }
}

void print(FormulaPtr formula) {
    switch(formula->type) {
    case Formula::True: std::cout << "T"; break;
    case Formula::False: std::cout << "F"; break;
    case Formula::Atom: {
        std::cout << formula->atom.symbol;
        if(!formula->atom.args.empty()) {
            std::cout << "(";
            print(formula->atom.args[0]);
            for(auto it = formula->atom.args.begin() + 1; it != formula->atom.args.end(); it++) {
                std::cout << ", ";
                print(*it);
            }
            std::cout << ")";
        }
    } break;
    case Formula::Not:
        std::cout << "~";
        print(formula->negation.subformula);
        break;
    case Formula::Binary:
        std::cout << "(";
        print(formula->binary.l);

        switch(formula->binary.type) {
        case BinaryData::And: std::cout << " & "; break;
        case BinaryData::Or:  std::cout << " | "; break;
        case BinaryData::Impl: std::cout << " -> "; break;
        case BinaryData::Eql:  std::cout << " <-> "; break;
        }

        print(formula->binary.r);
        std::cout << ")";
        break;
    case Formula::Quantifier:
        switch(formula->quantifier.type) {
        case QuantifierData::All: std::cout << "A"; break;
        case QuantifierData::Exists: std::cout << "E"; break;
        }
        std::cout << " " << formula->quantifier.var << " ";
        print(formula->quantifier.subformula);
        break;
    }
}


// --- Cas 7 ---
FormulaPtr simplifyNot(FormulaPtr formula) {
    FormulaPtr subformula = simplify(formula->negation.subformula);
    switch(subformula->type) {
    case Formula::Type::True:
        return std::make_shared<Formula>(FalseData{});
    case Formula::Type::False:
        return std::make_shared<Formula>(TrueData{});
    default:
        return std::make_shared<Formula>(NotData{subformula});
    }
}

FormulaPtr simplifyAnd(FormulaPtr formula) {
    FormulaPtr simplifiedL = simplify(formula->binary.l);
    FormulaPtr simplifiedR = simplify(formula->binary.r);

    if(simplifiedL->type == Formula::Type::False || simplifiedR->type == Formula::Type::False)
        return std::make_shared<Formula>(FalseData{});
    if(simplifiedL->type == Formula::Type::True)
        return simplifiedR;
    if(simplifiedR->type == Formula::Type::True)
        return simplifiedL;
    return std::make_shared<Formula>(BinaryData{BinaryData::And, simplifiedL, simplifiedR});
}

FormulaPtr simplifyOr(FormulaPtr formula) {
    FormulaPtr simplifiedL = simplify(formula->binary.l);
    FormulaPtr simplifiedR = simplify(formula->binary.r);

    if(simplifiedL->type == Formula::Type::True || simplifiedR->type == Formula::Type::True)
        return std::make_shared<Formula>(TrueData{});
    if(simplifiedL->type == Formula::Type::False)
        return simplifiedR;
    if(simplifiedR->type == Formula::Type::False)
        return simplifiedL;
    return std::make_shared<Formula>(BinaryData{BinaryData::Or, simplifiedL, simplifiedR});
}

FormulaPtr simplifyImpl(FormulaPtr formula) {
    FormulaPtr simplifiedL = simplify(formula->binary.l);
    FormulaPtr simplifiedR = simplify(formula->binary.r);

    if(simplifiedL->type == Formula::Type::False || simplifiedR->type == Formula::Type::True)
        return std::make_shared<Formula>(TrueData{});
    if(simplifiedL->type == Formula::Type::True)
        return simplifiedR;
    if(simplifiedR->type == Formula::Type::False)
        return simplify(std::make_shared<Formula>(NotData{simplifiedL}));
    return std::make_shared<Formula>(BinaryData{BinaryData::Impl, simplifiedL, simplifiedR});
}

FormulaPtr simplifyEql(FormulaPtr formula) {
    FormulaPtr simplifiedL = simplify(formula->binary.l);
    FormulaPtr simplifiedR = simplify(formula->binary.r);

    if(simplifiedL->type == Formula::Type::True)
        return simplifiedR;
    if(simplifiedR->type == Formula::Type::True)
        return simplifiedL;
    if(simplifiedL->type == Formula::Type::False)
        return simplify(std::make_shared<Formula>(NotData{simplifiedR}));
    if(simplifiedR->type == Formula::Type::False)
        return simplify(std::make_shared<Formula>(NotData{simplifiedL}));
    return std::make_shared<Formula>(BinaryData{BinaryData::Eql, simplifiedL, simplifiedR});
}

FormulaPtr simplifyQuantifier(FormulaPtr formula) {
    FormulaPtr subformula = simplify(formula->quantifier.subformula);
    if(!containsVariable(subformula, formula->quantifier.var, true))
        return subformula;
    return std::make_shared<Formula>(QuantifierData{
                                         formula->quantifier.type,
                                         formula->quantifier.var,
                                         subformula
                                     });
}

FormulaPtr simplify(FormulaPtr formula) {
    switch(formula->type) {
    case Formula::True:
    case Formula::False:
    case Formula::Atom:
        return formula;
    case Formula::Not:
        return simplifyNot(formula);
    case Formula::Binary:
        switch(formula->binary.type) {
        case BinaryData::And: return simplifyAnd(formula);
        case BinaryData::Or:  return simplifyOr(formula);
        case BinaryData::Impl: return simplifyImpl(formula);
        case BinaryData::Eql:  return simplifyEql(formula);
        }
    case Formula::Quantifier:
        return simplifyQuantifier(formula);
    }
}

FormulaPtr nnfNot(FormulaPtr formula) {
    switch(formula->type) {
    case Formula::Atom:
        return Not(formula);
    case Formula::Not:
        return nnf(formula->negation.subformula);
    case Formula::Binary: {
        FormulaPtr l = formula->binary.l;
        FormulaPtr r = formula->binary.r;
        switch(formula->binary.type) {
        case BinaryData::And:
            return Or(nnf(Not(l)), nnf(Not(r)));
        case BinaryData::Or:
            return And(nnf(Not(l)), nnf(Not(r)));
        case BinaryData::Impl:
            return And(nnf(l), nnf(Not(r)));
        case BinaryData::Eql:
            return Or(And(nnf(l), nnf(Not(r))), And(nnf(Not(l)), nnf(r)));
        }
    }
    case Formula::Quantifier:
        switch(formula->quantifier.type) {
        case QuantifierData::All: return Exists(formula->quantifier.var, nnf(Not(formula)));
        case QuantifierData::Exists: return All(formula->quantifier.var, nnf(Not(formula)));
        }

    default:
        return nullptr;
    }
}

FormulaPtr nnf(FormulaPtr formula) {
    switch(formula->type) {
    case Formula::Atom:
        return formula;
    case Formula::Binary: {
        FormulaPtr l = formula->binary.l;
        FormulaPtr r = formula->binary.r;
        switch(formula->binary.type) {
        case BinaryData::And:
        case BinaryData::Or:
            return Binary(formula->binary.type, nnf(l), nnf(r));
        case BinaryData::Impl:
            return Or(nnf(Not(l)), nnf(r));
        case BinaryData::Eql:
            return And(Or(nnf(Not(l)), nnf(r)), Or(nnf(Not(r)), nnf(l)));
        }
    }
    case Formula::Not:
        return nnfNot(formula->negation.subformula);
    case Formula::Quantifier:
        return Quantifier(formula->quantifier.type,
                          formula->quantifier.var,
                          nnf(formula->quantifier.subformula));

    default:
        return nullptr;
    }
}

FormulaPtr pullQuantifiers(FormulaPtr formula) {
    if(formula->type != Formula::Binary)
        return formula;

    BinaryData::Type type = formula->binary.type;
    FormulaPtr l = formula->binary.l;
    FormulaPtr r = formula->binary.r;

    if(type == BinaryData::And &&
       l->type == Formula::Quantifier &&
       l->quantifier.type == QuantifierData::All &&
       r->type == Formula::Quantifier &&
       r->quantifier.type == QuantifierData::All)
    {
        if(l->quantifier.var == r->quantifier.var)
            return All(l->quantifier.var, pullQuantifiers(And(l->quantifier.subformula, r->quantifier.subformula)));
        std::string uniqueVar = uniqueVariable(l, r);
        TermPtr varTerm = std::make_shared<Term>(VariableTerm{uniqueVar});
        return All(uniqueVar,
                   pullQuantifiers(And(substitute(l->quantifier.subformula, l->quantifier.var, varTerm),
                                       substitute(r->quantifier.subformula, r->quantifier.var, varTerm))));
    }

    if(type == BinaryData::Or &&
       l->type == Formula::Quantifier &&
       l->quantifier.type == QuantifierData::Exists &&
       r->type == Formula::Quantifier &&
       r->quantifier.type == QuantifierData::Exists)
    {
        if(l->quantifier.var == r->quantifier.var)
            return Exists(l->quantifier.var, pullQuantifiers(Or(l->quantifier.subformula, r->quantifier.subformula)));
        std::string uniqueVar = uniqueVariable(l, r);
        TermPtr varTerm = std::make_shared<Term>(VariableTerm{uniqueVar});
        return Exists(uniqueVar,
                      pullQuantifiers(Or(substitute(l->quantifier.subformula, l->quantifier.var, varTerm),
                                         substitute(r->quantifier.subformula, r->quantifier.var, varTerm))));
    }

    if(l->type == Formula::Quantifier) {
        if(!containsVariable(r, l->quantifier.var, true))
            return Quantifier(l->quantifier.type, l->quantifier.var,
                              pullQuantifiers(Binary(type,
                                                     l->quantifier.subformula,
                                                     r)));
        else {
            std::string uniqueVar = uniqueVariable(l->quantifier.subformula, r);
            TermPtr varTerm = std::make_shared<Term>(VariableTerm{uniqueVar});
            return Quantifier(l->quantifier.type, uniqueVar,
                              pullQuantifiers(Binary(type,
                                                     substitute(l->quantifier.subformula, l->quantifier.var, varTerm),
                                                     r)));
        }
    }

    if(r->type == Formula::Quantifier) {
        if(!containsVariable(l, r->quantifier.var, true))
            return Quantifier(r->quantifier.type, r->quantifier.var,
                              pullQuantifiers(Binary(type,
                                                     l,
                                                     r->quantifier.subformula)));
        else {
            std::string uniqueVar = uniqueVariable(l, r->quantifier.subformula);
            TermPtr varTerm = std::make_shared<Term>(VariableTerm{uniqueVar});
            return Quantifier(r->quantifier.type, uniqueVar,
                              pullQuantifiers(Binary(type,
                                                     l,
                                                     substitute(r->quantifier.subformula, r->quantifier.var, varTerm))));
        }
    }

    return formula;
}

FormulaPtr prenex(FormulaPtr formula) {
    switch(formula->type) {
    case Formula::Binary:
        return pullQuantifiers(Binary(formula->binary.type,
                                      prenex(formula->binary.l),
                                      prenex(formula->binary.r)));
    case Formula::Quantifier:
        return Quantifier(formula->quantifier.type,
                          formula->quantifier.var,
                          prenex(formula->quantifier.subformula));
    default:
        return formula;
    }
}

FormulaPtr skolem(FormulaPtr formula, Signature& s, std::set<std::string>& vars) {
    if(formula->type != Formula::Quantifier)
        return formula;

    if(formula->quantifier.type == QuantifierData::All) {
        vars.insert(formula->quantifier.var);
        return All(formula->quantifier.var, skolem(formula->quantifier.subformula, s, vars));
    }

    std::string uniqueFun = s.getUniqueSymbol(vars.size());
    std::vector<TermPtr> args;
    for(const auto& var : vars)
        args.push_back(std::make_shared<Term>(VariableTerm{var}));
    TermPtr uniqueTerm = std::make_shared<Term>(FunctionTerm{uniqueFun, args});
    return skolem(substitute(formula->quantifier.subformula, formula->quantifier.var, uniqueTerm), s, vars);
}

FormulaPtr skolem(FormulaPtr formula, Signature& s) {
    std::set<std::string> vars;
    return skolem(formula, s, vars);
}

bool equal(TermPtr t1, TermPtr t2) {
    if(t1->type != t2->type)
        return false;

    switch(t1->type) {
    case Term::Variable:
        return t1->variable == t2->variable;
    case Term::Function:
        if(t1->function.symbol != t2->function.symbol ||
           t1->function.args.size() != t2->function.args.size())
            return false;
        for(unsigned i = 0; i < t1->function.args.size(); i++)
            if(!equal(t1->function.args[i], t2->function.args[i]))
                return false;
        return true;
    }
}

bool equal(FormulaPtr f1, FormulaPtr f2) {
    if(f1->type != f2->type)
        return false;

    switch(f1->type) {
    case Formula::True:
    case Formula::False:
        return true;
    case Formula::Atom: {
        if(f1->atom.symbol != f2->atom.symbol ||
           f1->atom.args.size() != f2->atom.args.size())
            return false;

        for(unsigned i = 0; i < f1->atom.args.size(); i++)
            if(!equal(f1->atom.args[i], f2->atom.args[i]))
                return false;
        return true;
    }
    case Formula::Not:
        return equal(f1->negation.subformula, f2->negation.subformula);
    case Formula::Binary: {
        if(f1->binary.type != f2->binary.type)
            return false;

        return equal(f1->binary.l, f2->binary.l) &&
               equal(f2->binary.l, f2->binary.r);
    }
    case Formula::Quantifier: {
        if(f1->quantifier.type != f2->quantifier.type ||
           f1->quantifier.var != f2->quantifier.var)
            return false;
        return equal(f1->quantifier.subformula, f2->quantifier.subformula);
    }
    }
}
