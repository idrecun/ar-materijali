#ifndef FOL_H
#define FOL_H

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <variant>
#include <vector>

// Definisemo strukture za opisivanje termova
// Term moze biti
// 1. Promenljiva (npr. x)
// 2. Funkcijski term (npr. f(x, y), f(g(x), h(y, z)), itd.)
//    - Funkcijski term se sastoji od funkcijskog simbola (npr. f) i argumenata koji su termovi

struct Variable;
struct Function;
using Term = std::variant<Variable, Function>;
using TermPtr = std::shared_ptr<Term>;

struct Variable { std::string name; };
struct Function {
    std::string symbol;
    std::vector<TermPtr> args;
};

// Definisemo strukture za opisivanje formula logike prvog reda
// Formula moze biti
// 1. Atomicna formula (npr. P(x, y), Q(f(x), g(y)), itd.)
//    - Atomicna formula se sastoji od relacijskog simbola (npr. P) i argumenata koji su termovi
// 2. Negacija potformule
// 3. Binarna formula
// 4. Kvantifikator (npr. (A x) F(x) gde je F potformula)

struct Atom;
struct Not;
struct Binary;
struct Quantifier;
using Formula = std::variant<Atom, Not, Binary, Quantifier>;
using FormulaPtr = std::shared_ptr<Formula>;

struct Atom {
    std::string symbol;
    std::vector<TermPtr> args;
};
struct Not { FormulaPtr subformula; };
struct Binary {
    enum Type { And, Or, Impl, Eq } type;
    FormulaPtr l, r;
};
struct Quantifier {
    enum Type { All, Exists } type;
    std::string var;
    FormulaPtr subformula;
};

// Pomocne funkcije

// ptr Pravi pokazivac na formulu/term
TermPtr ptr(const Term& term) { return std::make_shared<Term>(term); }
FormulaPtr ptr(const Formula& formula) { return std::make_shared<Formula>(formula); }

// is Proverava da li je formula/term odredjenog tipa
template<typename T> bool is(const TermPtr& term) { return std::holds_alternative<T>(*term); }
template<typename T> bool is(const FormulaPtr& formula) { return std::holds_alternative<T>(*formula); }

// as Pretvara formulu/term u odredjeni tip
template<typename T> T as(const TermPtr& term) { return std::get<T>(*term); }
template<typename T> T as(const FormulaPtr& formula) { return std::get<T>(*formula); }


// Definisemo strukture za opisivanje signature i interpretacije
// Signatura je skup funkcijskih i relacijskih simbola
// Svakom simbolu pridruzujemo arnost (broj argumenata)
// Npr. za simbol f arnosti 2, f(x, y) je ispravan funkcijski term
struct Signature {
    std::map<std::string, unsigned> functions;
    std::map<std::string, unsigned> relations;
};

// Interpretacija (ili L-struktura za signaturu L) se sastoji iz
// - Domena ili univerzuma (skupa elemenata, npr. skup celih brojeva)
// - Funkcija i relacija nad tim domenom koje odgovaraju simbolima iz signature
using Domain = std::set<unsigned>;
using DomainFunction = std::function<unsigned(const std::vector<unsigned>&)>;
using DomainRelation = std::function<bool(const std::vector<unsigned>&)>;

struct LStructure {
    Signature signature;
    Domain domain;
    std::map<std::string, DomainFunction> functions;
    std::map<std::string, DomainRelation> relations;
};

// Provera signature formule

bool checkSignature(const TermPtr& term, const Signature& s) {
    if(is<Variable>(term))
        return true;
    if(is<Function>(term)) {
        // Proveravamo da li je simbol funkcije u signaturi i da li ima odgovarajuci broj argumenata
        const auto& function = as<Function>(term);
        if(!s.functions.contains(function.symbol))
            return false;
        if(s.functions.at(function.symbol) != function.args.size())
            return false;
        // Rekurzivno proveravamo argumente funkcije
        for(const auto& arg : function.args)
            if(!checkSignature(arg, s))
                return false;
        return true;
    }
    return false;
}

bool checkSignature(const FormulaPtr& formula, const Signature& s) {
    if(is<Atom>(formula)) {
        const auto& atom = as<Atom>(formula);
        if(!s.relations.contains(atom.symbol))
            return false;
        for(const auto& arg : atom.args)
            if(!checkSignature(arg, s))
                return false;
        return true;
    }
    if(is<Not>(formula))
        return checkSignature(as<Not>(formula).subformula, s);
    if(is<Binary>(formula)) {
        const auto& binary = as<Binary>(formula);
        return checkSignature(binary.l, s) && checkSignature(binary.r, s);
    }
    if(is<Quantifier>(formula))
        return checkSignature(as<Quantifier>(formula).subformula, s);
    return false;
}

// Evaluacija formule
// Prosledjujemo formulu, L-strukturu i valuaciju promenljivih
using Valuation = std::map<std::string, unsigned>;

unsigned evaluate(const TermPtr& term, const LStructure& s, const Valuation& val) {
    // Ako je promenljiva, vrednost je u valuaciji
    if(is<Variable>(term))
        return val.at(as<Variable>(term).name);
    // Ako je funkcija, evaluiramo argumente i primenjujemo funkciju iz interpretacije
    if(is<Function>(term)) {
        const auto& function = as<Function>(term);
        std::vector<unsigned> args;
        for(const auto& arg : function.args)
            args.push_back(evaluate(arg, s, val));
        return s.functions.at(function.symbol)(args);
    }
    return 0;
}

bool evaluate(const FormulaPtr& formula, const LStructure& s, const Valuation& val) {
    // Ako je atomicna formula, evaluiramo argumente i primenjujemo relaciju iz interpretacije
    if (is<Atom>(formula)) {
        const auto& atom = as<Atom>(formula);
        std::vector<unsigned> args;
        for(const auto& arg : atom.args)
            args.push_back(evaluate(arg, s, val));
        return s.relations.at(atom.symbol)(args);
    }
    if (is<Not>(formula))
        return !evaluate(as<Not>(formula).subformula, s, val);
    if (is<Binary>(formula)) {
        auto binary = as<Binary>(formula);
        bool lEval = evaluate(binary.l, s, val);
        bool rEval = evaluate(binary.r, s, val);
        switch(binary.type) {
            case Binary::And:  return lEval && rEval;
            case Binary::Or:   return lEval || rEval;
            case Binary::Impl: return !lEval || rEval;
            case Binary::Eq:  return lEval == rEval;
        }
    }
    if (is<Quantifier>(formula)) {
        auto qf = as<Quantifier>(formula);
        // Ako je univerzalni kvantifikator, provedemo evaluaciju za svaku vrednost iz domena
        if(qf.type == Quantifier::All) {
            // Pravimo kopiju valuacije (nije efikasno, ali nam je okej)
            Valuation valQuantified(val);
            // Za svaku vrednost domena postavljamo vrednost promenljiive i evaluiramo potformulu
            for(const auto& value : s.domain) {
                valQuantified[qf.var] = value;
                if(!evaluate(qf.subformula, s, valQuantified))
                    return false;
            }
            // Ako je svaka vrednost domena zadovoljila formulu, vracamo true
            return true;
        }
        // Ako je egzistencijalni kvantifikator, provedemo evaluaciju za svaku vrednost iz domena
        if(qf.type == Quantifier::Exists) {
            // Pravimo kopiju valuacije (nije efikasno, ali nam je okej)
            Valuation valQuantified(val);
            // Za svaku vrednost domena postavljamo vrednost promenljiive i evaluiramo potformulu
            for(const auto& value : s.domain) {
                valQuantified[qf.var] = value;
                if(evaluate(qf.subformula, s, valQuantified))
                    // Ako je neka vrednost domena zadovoljila formulu, vracamo true
                    return true;
            }
            return false;
        }
    }
    return false;
}

// Dohvatanje svih promenljivih koje se pojavljuju u formuli
// Postoje dve varijante provere koje zelimo da implementiramo
// 1. Dohvatanje svih promenljivih koje se pojavljuju u formuli
// 2. Dohvatanje svih *slobodnih* promenljivih koje se pojavljuju u formuli
// Pojavljivanje promenljive u formuli je slobodno ako se ne nalazi unutar kvantifikatora
// Npr. u formuli "Ex Q(x, y)" promenljiva y je slobodna, a promenljiva x nije
// Npr. u formuli "P(x) & Ex Q(x, y)" promenljive x i y su slobodne
//   - Pojavljivanje promenljive x u potformuli "Ex Q(x, y)" je vezano
//   - Pojavljivanje promenljive x u potformuli "P(x)" je slobodno
//   - Prema tome, promenljiva x je slobodna u formuli "P(x) & Ex Q(x, y)"
//   - Ovo znaci da vrednost formule zavisi od vrednosti promenljive x u valuaciji
//
// Ovo nam je neophodno prilikom supstitucije
void getVariables(const TermPtr& term, std::set<std::string>& vars) {
    if(is<Variable>(term)) {
        vars.insert(as<Variable>(term).name);
    }
    if(is<Function>(term)) {
        for(const auto& arg : as<Function>(term).args)
            getVariables(arg, vars);
    }
}

void getVariables(const FormulaPtr& formula, std::set<std::string>& vars, bool includeBound) {
    if (is<Atom>(formula)) {
        for(const auto& arg : as<Atom>(formula).args)
            getVariables(arg, vars);
    }
    if (is<Not>(formula))
        getVariables(as<Not>(formula).subformula, vars, includeBound);
    if (is<Binary>(formula)) {
        auto binary = as<Binary>(formula);
        getVariables(binary.l, vars, includeBound);
        getVariables(binary.r, vars, includeBound);
    }
    if (is<Quantifier>(formula)) {
        // Ako je kvantifikator, imamo dve varijante u zavisnosti od toga da li trazimo samo slobodne promenljive
        auto qf = as<Quantifier>(formula);
        // Ako trazimo sve promenljive, dodajemo promenljivu kvantifikatora u skup i rekurzivno trazimo promenljive u potformuli
        if (includeBound) {
            getVariables(qf.subformula, vars, includeBound);
            vars.insert(qf.var);
        }
        // Ako trazimo samo slobodne promenljive:
        // 1. Dohvatamo promenljive u potformuli u poseban skup
        // 2. Uklanjamo kvantifikovanu promenljivu iz tog skupa (jer je vezana)
        // 3. Dodajemo preostale promenljive u glavni skup promenljivih
        // Ovo mozemo optimizovati tako sto odmah dodamo sve u skup, a zatim uklonimo kvantifikovanu promenljivu
        // samo ako je nismo imali u skupu pre toga - ovo implementiramo
        else {
            bool varHasFreeOccurrence = vars.contains(qf.var);
            getVariables(qf.subformula, vars, includeBound);
            if (!varHasFreeOccurrence)
                vars.erase(qf.var);
        }
    }
}

bool containsVariable(const TermPtr& term, const std::string& var) {
    std::set<std::string> vars;
    getVariables(term, vars);
    return vars.contains(var);
}

bool containsVariable(const FormulaPtr& formula, const std::string& var, bool includeBound) {
    std::set<std::string> vars;
    getVariables(formula, vars, includeBound);
    return vars.contains(var);
}

// Odredjivanje promenljive koja se ne pojavljuje ni u termu ni u formuli
std::string uniqueVar(const FormulaPtr& formula, const TermPtr& term) {
    std::set<std::string> formulaVars;
    getVariables(formula, formulaVars, true);
    std::set<std::string> termVars;
    getVariables(term, termVars);

    // Generisemo u1, u2, u3, ... dok ne nadjemo promenljivu koja se nigde ne pojavljuje
    static unsigned uniqueCounter = 0;
    std::string var;
    do {
        var = "u" + std::to_string(++uniqueCounter);
    } while(formulaVars.contains(var) || termVars.contains(var));
    return var;
}

// Smena promenljive termom
// U slucaju kvantifikatora:
// 1. Ako je promenljiva kvantifikatora promenljiva koju zelimo da zamenimo, ne menjamo nista
//    - To je zato sto je promenljiva kvantifikatora vezana i ne zavisi od vrednosti promenljive u valuaciji
//    - Mozemo se uveriti u to time sto ako promenimo naziv vezane promenljive, znacenje formule se ne menja
//    - Npr. ako treba da primenimo smenu [x->f(y)] na formulu Ax P(x), rezultat treba da bude Ax P(x)
//    - Formule Ax P(x) i Az P(z) su ekvivalentne, a primena supstitucije [x->f(y)] na formulu Az P(z) nema efekta
// 2. Ako se promenljiva kvantifikatora pojavljuje u termu supstitucije, moramo da vrsimo preimenovanje
//    - Npr. ako bismo direktno primenili [x->f(y)] na formulu Ay P(x, y), rezultat bi bio Ay P(f(y), y)
//    - To je pogresno zato sto f(y) postaje vezano za kvantifikator Ay sto menja znacenje formule
//    - Pravilno bi bilo da prvo preimenujemo neku od ovih promenljivih, npr. Az P(x, z), i onda primenimo smenu
//    - Rezultat je onda Az P(f(y), z), sto je pravilno jer ne menja znacenje promenljive y (i samim tim formule)
// Ovo ce nam biti neophodno prilikom metode rezolucije

TermPtr substitute(const TermPtr& term, const std::string& var, const TermPtr& subterm) {
    // Ako je promenljiva, proveravamo da li je to promenljiva koju zelimo da zamenimo
    if(is<Variable>(term)) {
        if(as<Variable>(term).name == var)
            return subterm;
        return term;
    }
    // Ako je funkcija, rekurzivno smenjujemo pojavljivanja promenljive u argumentima
    if(is<Function>(term)) {
        auto function = as<Function>(term);
        std::vector<TermPtr> args;
        for(const auto& arg : function.args)
            args.push_back(substitute(arg, var, subterm));
        return ptr(Function{function.symbol, args});
    }
    return nullptr;
}

FormulaPtr substitute(const FormulaPtr& formula, const std::string& var, const TermPtr& term) {
    // Ako je atomicna formula, rekurzivno smenjujemo pojavljivanja promenljive u argumentima
    if(is<Atom>(formula)) {
        auto atom = as<Atom>(formula);
        std::vector<TermPtr> args;
        for(const auto& arg : atom.args)
            args.push_back(substitute(arg, var, term));
        return ptr(Atom{atom.symbol, args});
    }
    if(is<Not>(formula))
        return ptr(Not{substitute(as<Not>(formula).subformula, var, term)});
    if(is<Binary>(formula)) {
        auto binary = as<Binary>(formula);
        return ptr(Binary{binary.type, substitute(binary.l, var, term), substitute(binary.r, var, term)});
    }
    if(is<Quantifier>(formula)) {
        auto qf = as<Quantifier>(formula);
        if (qf.var == var)
            return formula;
        if (containsVariable(term, qf.var)) {
            std::string u = uniqueVar(formula, term);
            // U potformuli smenjujemo vezanu promenljivu novom promenljivom "u"
            FormulaPtr subformula = substitute(qf.subformula, qf.var, ptr(Variable{u}));
            // Primenjujemo smenu nad potformulom i vracamo formulu kvantifikovanu promenljivom "u"
            return ptr(Quantifier{qf.type, u, substitute(subformula, var, term)});
        }
        // Primenjujemo smenu nad potformulom i vracamo kvantifikovanu formulu
        return ptr(Quantifier{qf.type, qf.var, substitute(qf.subformula, var, term)});
    }
    return nullptr;
}

// Ispis formule

void print(const TermPtr& term) {
    if (is<Variable>(term))
        std::cout << as<Variable>(term).name;
    if (is<Function>(term)) {
        auto function = as<Function>(term);
        std::cout << function.symbol;
        if(!function.args.empty()) {
            std::cout << "(";
            print(function.args[0]);
            for (unsigned i = 1; i < function.args.size(); i++) {
                std::cout << ", ";
                print(function.args[i]);
            }
            std::cout << ")";
        }
    }
}

void print(const FormulaPtr& formula) {
    if(is<Atom>(formula)) {
        auto atom = as<Atom>(formula);
        std::cout << atom.symbol;
        if(!atom.args.empty()) {
            std::cout << "(";
            print(atom.args[0]);
            for (unsigned i = 1; i < atom.args.size(); i++) {
                std::cout << ", ";
                print(atom.args[i]);
            }
            std::cout << ")";
        }
    }
    if(is<Not>(formula)) {
        std::cout << "~";
        print(as<Not>(formula).subformula);
    }
    if(is<Binary>(formula)) {
        auto binary = as<Binary>(formula);
        std::cout << "(";
        print(binary.l);
        switch(binary.type) {
            case Binary::And:  std::cout << " & "; break;
            case Binary::Or:   std::cout << " | "; break;
            case Binary::Impl: std::cout << " -> "; break;
            case Binary::Eq:   std::cout << " <-> "; break;
        }
        print(binary.r);
        std::cout << ")";
    }
    if(is<Quantifier>(formula)) {
        auto qf = as<Quantifier>(formula);
        switch(qf.type) {
            case Quantifier::All: std::cout << "A"; break;
            case Quantifier::Exists: std::cout << "E"; break;
        }
        std::cout << qf.var << " ";
        print(qf.subformula);
    }
}

#endif //FOL_H
