#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression)
        :ast_(ParseFormulaAST(expression)) {}
    
    Value Evaluate(SheetInterface const& sheet) const override 
    {
        try {
            return ast_.Execute(sheet);
        }
        catch (const FormulaError& e) {
            return e;
        }
    }

    std::vector<Position> GetReferencedCells() const
    {
        auto& list = ast_.GetCells();
        std::vector<Position> cells_vector{ list.begin(), list.end() };
        std::sort(cells_vector.begin(), cells_vector.end());
        auto last = std::unique(cells_vector.begin(), cells_vector.end());
        cells_vector.erase(last, cells_vector.end());
        return cells_vector;
    }

    std::string GetExpression() const override {
        std::ostringstream string_buffer;
        ast_.PrintFormula(string_buffer);
        return string_buffer.str();
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...) {
        throw FormulaException("");
    }
}