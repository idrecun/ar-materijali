#include "formula.h"
#include <iostream>

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
            unsigned uniqueCounter = 0;
            std::string uniqueVar;
            do {
                uniqueVar = "u" + std::to_string(++uniqueCounter);
            } while(containsVariable(formula, uniqueVar, false) || containsVariable(t, uniqueVar));

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
