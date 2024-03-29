#pragma once

#include "common.h"
#include "formula.h"


#include <functional>
#include <forward_list>
#include <optional>
#include <unordered_set>

class Sheet;

struct PosHasher{
    size_t operator()(const Position& pos) const {
        return std::hash<int>{}(pos.col) * 13 + std::hash<int>{}(pos.row);
    }
};

class Cell : public CellInterface {
public:
    Cell(const SheetInterface& sheet);
    ~Cell();

    void Set(std::string text, Position pos = {0,0});
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    std::optional<double> GetCache() const;
 

private:
    //можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
    protected:
        Value value_ = "";
        std::string text_ = "";
    };

    class EmptyImpl : public Impl {
    public:

        Value GetValue() const override {
            return value_;
        }

        std::string GetText() const override {
            return text_;
        }
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string text) {
            text_ = text;
            if (text[0] == '\'') {
                text = text.substr(1);
            }
            value_ = text;
        }

        Value GetValue() const override {
            return value_;
        }

        std::string GetText() const override {
            return text_;
        }
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string_view expression, SheetInterface const& sheet):sheet_(sheet) {

            if (expression.empty() || expression[0] != '=') throw std::logic_error("");

            expression = expression.substr(1);
            formula_ = ParseFormula(std::string(expression));
            value_ = std::string(expression);

            text_ = "=" + formula_->GetExpression();
        }
        std::vector<Position> GetReferencedCells() const {
            return formula_->GetReferencedCells();
        }
        
        Value GetValue() const override {
            auto formula = formula_->Evaluate(sheet_);
            if (std::holds_alternative<double>(formula)) {
                return std::get<double>(formula);
            }
            return std::get<FormulaError>(formula);
        }

        std::string GetText() const override {
            return text_;
        }

    private:
        std::unique_ptr<FormulaInterface> formula_;
        SheetInterface const& sheet_;
    };

    std::unique_ptr<Impl> impl_;
    std::optional<double> cache_;
    std::vector<Position> backward_cell_;
    const SheetInterface& sheet_;
    // Добавьте поля и методы для связи с таблицей, проверки циклических 
    // зависимостей, графа зависимостей и т. д.
public:
    void CheckCircularDependency(FormulaImpl const& f_impl, Position const& pos) const;
};
