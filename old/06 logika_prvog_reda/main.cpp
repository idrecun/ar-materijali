#include <iostream>
#include "interpretation.h"
#include "formula.h"

unsigned zero(const std::vector<unsigned>&) { return 0; }
unsigned one(const std::vector<unsigned>&)  { return 1; }
unsigned plus(const std::vector<unsigned>& args) { return (args[0] + args[1]) % 4; }
unsigned times(const std::vector<unsigned>& args) { return (args[0] * args[1]) % 4; }
bool even(const std::vector<unsigned>& args) { return args[0] % 2 == 0; }
bool equals(const std::vector<unsigned>& args) { return args[0] == args[1]; }

int main() {
    LStructure L;

    L.signature.fun["0"] = 0;
    L.signature.fun["1"] = 0;
    L.signature.fun["+"] = 2;
    L.signature.fun["*"] = 2;
    L.signature.rel["even"] = 1;
    L.signature.rel["="] = 2;

    L.domain = {0, 1, 2, 3};

    L.functions["0"] = zero;
    L.functions["1"] = one;
    L.functions["+"] = plus;
    L.functions["*"] = times;

    L.relations["even"] = even;
    L.relations["="] = equals;

    // Ex (even(x) & ~even(x))
    TermPtr x = std::make_shared<Term>(VariableTerm{"x"});
    FormulaPtr evenX = std::make_shared<Formula>(AtomData{"even", {x}});
    FormulaPtr oddX = std::make_shared<Formula>(NotData{evenX});
    FormulaPtr evenAndOddX = std::make_shared<Formula>(BinaryData{BinaryData::And, evenX, oddX});
    FormulaPtr existsEvenAndOddX = std::make_shared<Formula>(QuantifierData{QuantifierData::Exists, "x", evenAndOddX});

    if(!checkSignature(existsEvenAndOddX, L.signature)) {
        std::cout << "Signature mismatch" << std::endl;
    }
    else {
        print(existsEvenAndOddX); std::cout << std::endl;

        Valuation val;
        std::cout << evaluate(existsEvenAndOddX, L, val) << std::endl;
    }

    // Ey (even(x) & ~even(x)) [x -> y + 1]
    // = Eu (even(y + 1) & ~even(y + 1))
    TermPtr oneTerm = std::make_shared<Term>(FunctionTerm{"1", {}});
    TermPtr y = std::make_shared<Term>(VariableTerm{"y"});
    TermPtr plusTerm = std::make_shared<Term>(FunctionTerm{"+", {y, oneTerm}});
    FormulaPtr existsY = std::make_shared<Formula>(QuantifierData{
                                                       QuantifierData::Exists,
                                                       "y",
                                                       evenAndOddX
                                                   });
    print(existsY); std::cout << std::endl;
    FormulaPtr sub = substitute(existsY, "x", plusTerm);

    if(!checkSignature(sub, L.signature)) {
        std::cout << "Signature mismatch" << std::endl;
    }
    else {
        print(sub); std::cout << std::endl;
    }

    return 0;
}
