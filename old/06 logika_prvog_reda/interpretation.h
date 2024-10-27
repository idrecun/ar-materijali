#ifndef INTERPRETATION_H
#define INTERPRETATION_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <any>
#include <functional>

struct Signature {
    std::map<std::string, unsigned> rel;
    std::map<std::string, unsigned> fun;
};

using Domain = std::set<unsigned>;
using Valuation = std::map<std::string, unsigned>;

using Function = std::function<unsigned(const std::vector<unsigned>&)>;
using Relation = std::function<bool(const std::vector<unsigned>&)>;

struct LStructure {
    Signature signature;
    Domain domain;
    std::map<std::string, Function> functions;
    std::map<std::string, Relation> relations;
};

#endif // INTERPRETATION_H
