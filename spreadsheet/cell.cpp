#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

// Реализуйте следующие методы
// Реализуйте следующие методы
Cell::Cell(const SheetInterface& sheet) :sheet_(sheet) {
	impl_ = std::make_unique<EmptyImpl>();
}

Cell::~Cell() {}

void Cell::Set(std::string text, Position pos) {
	if (text.size() == 0) impl_ = std::make_unique<EmptyImpl>();
	else if (text.size() > 1 && text[0] == '=') {
		try {
			auto formula = std::make_unique<FormulaImpl>(std::move(text), sheet_);
			if (!formula->GetReferencedCells().empty()) {

				backward_cell_ = std::move(formula->GetReferencedCells());
				for (auto cell : backward_cell_) {
					if (pos == cell) {
						throw  CircularDependencyException{ "Circular error" };
					}
					if (sheet_.GetCell(cell) == nullptr) {
						auto non_const_sheet = const_cast<SheetInterface*>(&sheet_);
						non_const_sheet->SetCell(cell, "");
					}
				}
				CheckCircularDependency(*formula, pos);
			}
			impl_ = std::move(formula);
		}
		catch (const FormulaError& e) {
			throw e;
		}
		auto value = impl_->GetValue();
		if (std::get_if<double>(&value)) {
			cache_ = std::get<double>(value);
		}


	}
	else impl_ = std::make_unique<TextImpl>(std::move(text));

}

void Cell::Clear() {
	impl_ = std::make_unique<EmptyImpl>();
}


Cell::Value Cell::GetValue() const {
	return impl_->GetValue();
}

std::optional<double> Cell::GetCache() const {
	return cache_;
}

std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position>  Cell::GetReferencedCells() const {

	return backward_cell_;
}


void AddRecursiveCell(Position const& pos, std::unordered_set<Position, PosHasher >& uniq_poses, SheetInterface const& sheet) {

	auto poses = sheet.GetCell(pos)->GetReferencedCells();
	if (poses.empty()) {
		return;
	}
	for (auto& pos : poses) {
		if (uniq_poses.count(pos)) {
			throw  CircularDependencyException{ "Circular error" };
		}
		uniq_poses.insert(pos);
		AddRecursiveCell(pos, uniq_poses, sheet);
	}

}
void Cell::CheckCircularDependency(FormulaImpl const& f_impl, Position const& pos) const
{
	std::unordered_set<Position, PosHasher > uniq_cells;
	uniq_cells.insert(pos);
	auto poses = f_impl.GetReferencedCells();
	for (auto& pos : poses) {
		uniq_cells.insert(pos);
		AddRecursiveCell(pos, uniq_cells, sheet_);
	}

}