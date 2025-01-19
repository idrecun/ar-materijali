#include "fol.h"

unsigned zero(const std::vector<unsigned>&) { return 0; }
unsigned one(const std::vector<unsigned>&)  { return 1; }
unsigned plus(const std::vector<unsigned>& args) { return (args[0] + args[1]) % 4; }
unsigned times(const std::vector<unsigned>& args) { return (args[0] * args[1]) % 4; }
bool even(const std::vector<unsigned>& args) { return args[0] % 2 == 0; }
bool equals(const std::vector<unsigned>& args) { return args[0] == args[1]; }

int main() {
    LStructure L;

    L.signature.functions["0"] = 0;
    L.signature.functions["1"] = 0;
    L.signature.functions["+"] = 2;
    L.signature.functions["*"] = 2;
    L.signature.relations["even"] = 1;
    L.signature.relations["="] = 2;

    L.domain = {0, 1, 2, 3};

    L.functions["0"] = zero;
    L.functions["1"] = one;
    L.functions["+"] = plus;
    L.functions["*"] = times;

    L.relations["even"] = even;
    L.relations["="] = equals;

    // Ex (even(x) & ~even(x))
    TermPtr x = ptr(Variable{"x"});
    FormulaPtr evenX = ptr(Atom{"even", {x}});
    FormulaPtr oddX = ptr(Not{evenX});
    FormulaPtr evenAndOddX = ptr(Binary{Binary::And, evenX, oddX});
    FormulaPtr existsEvenAndOddX = ptr(Quantifier{Quantifier::Exists, "x", evenAndOddX});

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
    TermPtr oneTerm = ptr(Function{"1", {}});
    TermPtr y = ptr(Variable{"y"});
    TermPtr plusTerm = ptr(Function{"+", {y, oneTerm}});
    FormulaPtr existsY = ptr(Quantifier{Quantifier::Exists, "y", evenAndOddX});
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
