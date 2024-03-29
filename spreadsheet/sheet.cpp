#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit([&](const auto& x) { output << x; }, value);
    return output;
}

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    Position size = { pos.row + 1, pos.col + 1};
    if (cells_.size() < static_cast<std::size_t>(size.row))
    {
        cells_.resize(size.row);
    }
    if (cells_[pos.row].size() < static_cast<std::size_t>(size.col))
    {
        cells_[pos.row].resize(size.col);
    }
    auto cell = std::make_unique<Cell>(*this);
    cell->Set(text, pos);

    cells_[pos.row][pos.col] = std::move(cell);
    
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    if (cells_.size() <= static_cast<std::size_t>(pos.row) || cells_[pos.row].size() <= static_cast<std::size_t>(pos.col)) {
        return nullptr;
    }
    auto& cell = cells_[pos.row][pos.col];
    if (cell.get() != nullptr) { 
        return cell.get();
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    if (static_cast<int>(cells_.size()) <= pos.row ||  static_cast<int>(cells_[pos.row].size()) <= pos.col ) {
        return;
    }
    cells_[pos.row][pos.col].reset();
}

Size Sheet::GetPrintableSize() const {


    int max_col = -1;
    int max_row = -1;

    for (int row = 0; row < static_cast<int>(cells_.size()); ++row) {
        for (int col = 0; col < static_cast<int>(cells_[row].size()); ++col) {
            if (auto p = GetCell({ row, col }); p != nullptr && !p->GetText().empty()) {
                max_col = std::max(max_col, col);
                max_row = std::max(max_row, row);
            }
        }
    }
    Size size{ std::max(max_row + 1, 0), std::max(max_col + 1, 0) };
    return size;
}

void Sheet::PrintValues(std::ostream& output) const {
    auto size = GetPrintableSize();

    for (auto i = 0; i < size.rows; i++) {
        for (auto j = 0; j < size.cols; j++)
        {

            auto cell = GetCell({ i,j });
            if (cell) {
                output << cell->GetValue();
            }
            if (j != size.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    auto size = GetPrintableSize();

    for (auto i = 0; i < size.rows; i++) {
        for (auto j = 0; j < size.cols; j++)
        {

            auto cell = GetCell({ i,j });
            if (cell) {
                output << cell->GetText();
            }
            if (j != size.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}