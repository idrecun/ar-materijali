#include "resolution.h"
#include <iostream>


TermPtr substitute(TermPtr term, const Substitution& s) {
    switch(term->type) {
    case Term::Variable: return s.find(term->variable) != s.end() ? s.at(term->variable) : term;
    case Term::Function:
        std::vector<TermPtr> subterms;
        for(const auto& subterm : term->function.args)
            subterms.push_back(substitute(subterm, s));
        return std::make_shared<Term>(FunctionTerm{term->function.symbol, subterms});
    }
}

FormulaPtr substitute(FormulaPtr formula, const Substitution& s) {
    switch(formula->type) {
    case Formula::True:
    case Formula::False:
        return formula;
    case Formula::Atom: {
        std::vector<TermPtr> subterms;
        for(const auto& subterm : formula->atom.args)
            subterms.push_back(substitute(subterm, s));
        return std::make_shared<Formula>(AtomData{formula->atom.symbol, subterms});
    }
    case Formula::Not:
        return std::make_shared<Formula>(NotData{substitute(formula->negation.subformula, s)});
    case Formula::Binary:
        return std::make_shared<Formula>(BinaryData{
                                             formula->binary.type,
                                             substitute(formula->binary.l, s),
                                             substitute(formula->binary.r, s)
                                         });
    case Formula::Quantifier: {
        Substitution reducedS(s);
        reducedS.erase(formula->quantifier.var);
        if(reducedS.empty())
            return formula;

        bool termsContainVariable = false;
        for(const auto& substitution : reducedS)
            if(containsVariable(substitution.second, formula->quantifier.var))
                termsContainVariable = true;

        if(termsContainVariable) {
            std::string uniqueVar = uniqueVariable(formula, formula); // treba uniqueVariable za skup termova
            TermPtr varTerm = std::make_shared<Term>(VariableTerm{uniqueVar});
            FormulaPtr subformula = substitute(formula->quantifier.subformula, formula->quantifier.var, varTerm);
            return std::make_shared<Formula>(QuantifierData{
                                                 formula->quantifier.type,
                                                 uniqueVar,
                                                 substitute(subformula, reducedS)
                                             });
        }
        return std::make_shared<Formula>(QuantifierData{
                                                  formula->quantifier.type,
                                                  formula->quantifier.var,
                                                  substitute(formula->quantifier.subformula, reducedS)
                                              });
    }
    }
}

// --- Unifikacija ---
// Koraci:
// 1. Faktorizacija (uklanjanje duplikata parova)
// 2. Tautologija   (uklanjanje parova oblika t -> t)
// 3. Orijentacija  (usmeravanje u promenljiva -> term)
// 4. Za parove oblika t1 -> t2 gde nijedan nije promenljiva
//    4.1 Dekompozicija  (ako su simbol i arnost isti, dodaju se parovi podtermova)
//    4.2 Kolizija       (ako su simboli ili arnosti razliciti)
// 5. Za parove oblika v -> t
//    5.1 Ciklus         (ako se v sadrzi u t)
//    5.2 Aplikacija     (primenjuje se [v->t] na sve parove)
// 6. Ako nije moguce primeniti nijedno pravilo, zaustavljamo se

bool factorization(TermPairs& pairs) {
    bool change = false;
    for(unsigned i = 0; i < pairs.size(); i++) {
        unsigned j = i + 1;
        while(j < pairs.size())
            if(equal(pairs[i].first, pairs[j].first) &&
               equal(pairs[i].second, pairs[j].second))
            {
                std::swap(pairs[j], pairs.back());
                pairs.pop_back();
                change = true;
            }
            else j++;
    }
    return change;
}

bool tautology(TermPairs& pairs) {
    bool change = false;
    unsigned i = 0;
    while(i < pairs.size()) {
        if(equal(pairs[i].first, pairs[i].second)) {
            std::swap(pairs[i], pairs.back());
            pairs.pop_back();
            change = true;
        }
        else i++;
    }
    return change;
}

bool orientation(TermPairs& pairs) {
    bool change = false;
    for(auto& vt : pairs)
        if(vt.first->type == Term::Function &&
           vt.second->type == Term::Variable)
        {
            std::swap(vt.first, vt.second);
            change = true;
        }
    return change;
}

bool decomposition(TermPairs& pairs, bool& collision) {
    bool change = false;
    unsigned i = 0;
    while(i < pairs.size())
        if(pairs[i].first->type == Term::Function &&
           pairs[i].second->type == Term::Function)
        {
            auto& t1 = pairs[i].first->function;
            auto& t2 = pairs[i].second->function;
            if(t1.symbol != t2.symbol ||
               t1.args.size() != t2.args.size())
            {
                collision = true;
                return false;
            }

            for(unsigned j = 0; j < t1.args.size(); j++)
                pairs.push_back({t1.args[j], t2.args[j]});

            std::swap(pairs[i], pairs.back());
            pairs.pop_back();
            change = true;
        }
        else i++;
    return change;
}

bool application(TermPairs& pairs, bool& cycle) {
    bool change = false;
    for(unsigned i = 0; i < pairs.size(); i++)
        if(pairs[i].first->type == Term::Variable) {
            auto& v = pairs[i].first->variable;
            auto& t = pairs[i].second;
            if(containsVariable(t, v)) {
                cycle = true;
                return false;
            }

            for(unsigned j = 0; j < pairs.size(); j++)
                if(j != i) {
                    if(containsVariable(pairs[j].first, v)) {
                        pairs[j].first = substitute(pairs[j].first, v, t);
                        change = true;
                    }
                    if(containsVariable(pairs[j].second, v)) {
                        pairs[j].second = substitute(pairs[j].second, v, t);
                        change = true;
                    }
                }
        }
    return change;
}

std::optional<Substitution> unify(const TermPairs &pairs) {
    TermPairs result(pairs);

    bool repeat = true;
    bool cycle = false;
    bool collision = false;
    while(repeat && !cycle && !collision) {
        repeat =   factorization(result)
                || tautology(result)
                || orientation(result)
                || decomposition(result, cycle)
                || application(result, collision);
    }

    if(cycle || collision)
        return {};

    Substitution s;
    for(const auto& vt : result)
        s[vt.first->variable] = vt.second;
    return s;
}

std::optional<Substitution> unify(FormulaPtr a1, FormulaPtr a2) {
    if(a1->type != Formula::Atom || a2->type != Formula::Atom)
        return {};

    if(a1->atom.symbol != a2->atom.symbol)
        return {};

    if(a1->atom.args.size() != a2->atom.args.size())
        return {};

    TermPairs pairs;
    for(unsigned i = 0; i < a1->atom.args.size(); i++)
        pairs.push_back({a1->atom.args[i], a2->atom.args[i]});

    return unify(pairs);
}


// --- Cas 9 ---
bool clauseContainsLiteral(const Clause& clause, FormulaPtr literal) {
    for(const auto& l : clause)
        if(equal(l, literal))
            return true;
    return false;
}

bool clauseContained(const CNF& cnf, const Clause& clause) {
    for(const auto& c : cnf) {
        bool contained = true;
        for(const auto& literal : c)
            if(!clauseContainsLiteral(clause, literal)) {
                contained = false;
                break;
            }
        if(contained)
            return true;
    }
    return false;
}

bool clauseTautology(const Clause& clause) {
    for(const auto& literal : clause) {
        FormulaPtr opposite;
        if(literal->type == Formula::Not)
            opposite = literal->negation.subformula;
        else
            opposite = Not(literal);

        if(clauseContainsLiteral(clause, opposite))
            return true;
    }
    return false;
}

// prosledjujemo indekse, a ne reference zbog toga sto
// se prilikom izvrsavanja funkcije cnf moze menjati (radi se push_back)
// pa se u slucaju realokacije svi iteratori i sve reference na elemente
// vektora invalidiraju :(
bool groupLiterals(CNF& cnf, unsigned c) {
    bool change = false;
    for(unsigned i = 0; i < cnf[c].size(); i++) {
        for(unsigned j = i + 1; j < cnf[c].size(); j++) {
            std::optional<Substitution> s;
            if(cnf[c][i]->type == Formula::Not &&
               cnf[c][j]->type == Formula::Not)
                s = unify(cnf[c][i]->negation.subformula, cnf[c][j]->negation.subformula);
            else
                s = unify(cnf[c][i], cnf[c][j]);

            if(s) {
                Clause groupedClause = cnf[c];
                std::swap(groupedClause[j], groupedClause.back());
                groupedClause.pop_back();

                for(auto& literal : groupedClause)
                    literal = substitute(literal, s.value());

                if(!clauseTautology(groupedClause) && !clauseContained(cnf, groupedClause)) {
                    change = true;
                    cnf.push_back(groupedClause);
                }
            }
        }
    }
    return change;
}

bool grouping(CNF& cnf, unsigned& i) {
    bool change = false;
    while(i < cnf.size())
        if(groupLiterals(cnf, i++))
            change = true;
    return change;
}

bool containsVariable(const Clause& clause, const std::string& v) {
    for(const auto& literal : clause)
        if(containsVariable(literal, v))
            return true;
    return false;
}

void renameVariables(Clause& c1, Clause& c2) {
    std::set<std::string> vars;
    for(const auto& literal : c1)
        getVariables(literal, vars);
    for(const auto& v : vars) {
        std::string renamed = uniqueVariable(c1, c2);
        for(auto& literal : c2)
            literal = substitute(literal, v, VariableT(renamed));
    }
}

// prosledjujemo indekse, a ne reference zbog toga sto
// se prilikom izvrsavanja funkcije cnf moze menjati (radi se push_back)
// pa se u slucaju realokacije svi iteratori i sve reference na elemente
// vektora invalidiraju :(
bool resolveClauses(CNF& cnf, unsigned c1, unsigned c2) {
    renameVariables(cnf[c1], cnf[c2]);
    bool change = false;
    for(unsigned i = 0; i < cnf[c1].size(); i++)
        for(unsigned j = 0; j < cnf[c2].size(); j++) {
            std::optional<Substitution> s;
            if(cnf[c1][i]->type == Formula::Not && cnf[c2][j]->type == Formula::Atom)
                s = unify(cnf[c1][i]->negation.subformula, cnf[c2][j]);
            else if(cnf[c1][i]->type == Formula::Atom && cnf[c2][j]->type == Formula::Not)
                s = unify(cnf[c1][i], cnf[c2][j]->negation.subformula);

            if(s) {
                Clause r1 = cnf[c1], r2 = cnf[c2];
                std::swap(r1[i], r1.back());
                r1.pop_back();
                std::swap(r2[j], r2.back());
                r2.pop_back();

                Clause r;
                for(const auto& literal : r1)
                    r.push_back(substitute(literal, s.value()));
                for(const auto& literal : r2)
                    r.push_back(substitute(literal, s.value()));

                if(!clauseTautology(r) && !clauseContained(cnf, r)) {
                    change = true;
                    cnf.push_back(r);
                }
            }
        }
    return change;
}

bool resolution(CNF &cnf, unsigned& i) {
    bool change = false;
    while(i < cnf.size()) {
        for(unsigned j = 0; j < i; j++)
            if(resolveClauses(cnf, j, i))
                change = true;
        i++;
    }
    return change;
}

bool resolution(const CNF& cnf) {
    CNF clauses = cnf;
    unsigned lastGroupingIndex = 0;
    unsigned lastResolutionIndex = 0;
    unsigned lastPrintIndex = 0;
    while(grouping(clauses, lastGroupingIndex) || resolution(clauses, lastResolutionIndex)) {
        while(lastPrintIndex < clauses.size()) {
            std::cout << "{ ";
            for(const auto& literal : clauses[lastPrintIndex]) {
                print(literal);
                std::cout << " ";
            }
            std::cout << "}" << std::endl;
            lastPrintIndex++;
        }
        for(const auto& clause : clauses)
            if(clause.empty())
                return false;
    }
    return true;
}
