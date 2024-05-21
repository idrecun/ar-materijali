#ifndef RESOLUTION_H
#define RESOLUTION_H

#include <map>
#include <string>
#include <optional>
#include "formula.h"

using Substitution = std::map<std::string, TermPtr>;

TermPtr substitute(TermPtr term, const Substitution& s);
FormulaPtr substitute(FormulaPtr formula, const Substitution& s);

using TermPairs = std::vector< std::pair<TermPtr, TermPtr> >;

std::optional<Substitution> unify(const TermPairs& pairs);
std::optional<Substitution> unify(FormulaPtr a1, FormulaPtr a2);

// --- Cas 9 ---
using Clause = std::vector<FormulaPtr>;
using CNF = std::vector<Clause>;

bool resolution(const CNF& cnf);

#endif // RESOLUTION_H
