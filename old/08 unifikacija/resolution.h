#ifndef RESOLUTION_H
#define RESOLUTION_H

#include <map>
#include <string>
#include <optional>
#include "formula.h"

bool equal(TermPtr t1, TermPtr t2);

using Substitution = std::map<std::string, TermPtr>;
using TermPairs = std::vector< std::pair<TermPtr, TermPtr> >;

std::optional<Substitution> unify(const TermPairs& pairs);
std::optional<Substitution> unify(FormulaPtr a1, FormulaPtr a2);

#endif // RESOLUTION_H
