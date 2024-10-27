#include "resolution.h"
#include <iostream>

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
    for(auto& tv : pairs)
        if(tv.first->type == Term::Function &&
           tv.second->type == Term::Variable)
        {
            std::swap(tv.first, tv.second);
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

            if(t1.symbol != t2.symbol || t1.args.size() != t2.args.size()) {
                collision = true;
                return true;
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
                return true;
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
    while(repeat && !cycle && !collision)
        repeat =   factorization(result)
                || tautology(result)
                || orientation(result)
                || decomposition(result, collision)
                || application(result, cycle);

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
