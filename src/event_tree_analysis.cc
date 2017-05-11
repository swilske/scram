/*
 * Copyright (C) 2017 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/// @file event_tree_analysis.cc
/// Implementation of event tree analysis facilities.

#include "event_tree_analysis.h"

#include "expression/numerical.h"

namespace scram {
namespace core {

EventTreeAnalysis::EventTreeAnalysis(
    const mef::InitiatingEvent& initiating_event, const Settings& settings)
    : Analysis(settings), initiating_event_(initiating_event) {}

namespace {  // The model cloning functions.

std::unique_ptr<mef::Formula> Clone(const mef::Formula& formula) noexcept {
  auto new_formula = std::make_unique<mef::Formula>(formula.type());
  for (const mef::Formula::EventArg& arg : formula.event_args())
    new_formula->AddArgument(arg);
  for (const mef::FormulaPtr& arg : formula.formula_args())
    new_formula->AddArgument(Clone(*arg));
  return new_formula;
}

}  // namespace

void EventTreeAnalysis::Analyze() noexcept {
  assert(initiating_event_.event_tree());
  SequenceCollector collector{initiating_event_};
  CollectSequences(initiating_event_.event_tree()->initial_state(), &collector);
  for (auto& sequence : collector.sequences) {
    auto gate = std::make_unique<mef::Gate>("__" + sequence.first->name());
    std::vector<mef::FormulaPtr> gate_formulas;
    std::vector<mef::Expression*> arg_expressions;
    for (PathCollector& path_collector : sequence.second) {
      if (path_collector.formulas.size() == 1) {
        gate_formulas.push_back(core::Clone(*path_collector.formulas.front()));
      } else if (path_collector.formulas.size() > 1) {
        auto and_formula = std::make_unique<mef::Formula>(mef::kAnd);
        for (const mef::Formula* arg_formula : path_collector.formulas)
          and_formula->AddArgument(core::Clone(*arg_formula));
        gate_formulas.push_back(std::move(and_formula));
      }
      if (path_collector.expressions.size() == 1) {
        arg_expressions.push_back(path_collector.expressions.front());
      } else if (path_collector.expressions.size() > 1) {
        expressions_.push_back(
            std::make_unique<mef::Mul>(std::move(path_collector.expressions)));
        arg_expressions.push_back(expressions_.back().get());
      }
    }
    assert(gate_formulas.empty() || arg_expressions.empty());
    bool is_expression_only = !arg_expressions.empty();
    if (gate_formulas.size() == 1) {
      gate->formula(std::move(gate_formulas.front()));
    } else if (gate_formulas.size() > 1) {
      auto or_formula = std::make_unique<mef::Formula>(mef::kOr);
      for (mef::FormulaPtr& arg_formula : gate_formulas)
        or_formula->AddArgument(std::move(arg_formula));
      gate->formula(std::move(or_formula));
    } else if (!arg_expressions.empty()) {
      auto event =
          std::make_unique<mef::BasicEvent>("__" + sequence.first->name());
      if (arg_expressions.size() == 1) {
        event->expression(arg_expressions.front());
      } else if (arg_expressions.size() > 1) {
        expressions_.push_back(
            std::make_unique<mef::Add>(std::move(arg_expressions)));
        event->expression(expressions_.back().get());
      }
      gate->formula(std::make_unique<mef::Formula>(mef::kNull));
      gate->formula().AddArgument(event.get());
      basic_events_.push_back(std::move(event));
    } else {
      gate->formula(std::make_unique<mef::Formula>(mef::kNull));
      gate->formula().AddArgument(&mef::HouseEvent::kTrue);
    }
    sequences_.push_back(
        {*sequence.first, std::move(gate), is_expression_only});
  }
}

void EventTreeAnalysis::CollectSequences(const mef::Branch& initial_state,
                                         SequenceCollector* result) noexcept {
  struct Collector {
    class Visitor : public mef::InstructionVisitor {
     public:
      explicit Visitor(Collector* collector) : collector_(*collector) {}

      /// @todo
      void Visit(const mef::SetHouseEvent*) override {}

      void Visit(const mef::Link* link) override {
        is_linked_ = true;
        Collector continue_connector(collector_);
        continue_connector(&link->event_tree().initial_state());
      }

      void Visit(const mef::CollectFormula* collect_formula) override {
        collector_.path_collector_.formulas.push_back(
            &collect_formula->formula());
      }

      void Visit(const mef::CollectExpression* collect_expression) override {
        collector_.path_collector_.expressions.push_back(
            &collect_expression->expression());
      }

      bool is_linked() const { return is_linked_; }

     private:
      Collector& collector_;
      bool is_linked_ = false;  /// Indicate that sequences not be registered.
    };

    void operator()(const mef::Sequence* sequence) {
      Visitor visitor(this);
      for (const mef::Instruction* instruction : sequence->instructions())
        instruction->Accept(&visitor);
      if (!visitor.is_linked())
        result_->sequences[sequence].push_back(std::move(path_collector_));
    }

    void operator()(const mef::Fork* fork) const {
      for (const mef::Path& fork_path : fork->paths())
        Collector(*this)(&fork_path);  // NOLINT(runtime/explicit)
    }

    void operator()(const mef::Branch* branch) {
      Visitor visitor(this);
      for (const mef::Instruction* instruction : branch->instructions())
        instruction->Accept(&visitor);

      boost::apply_visitor(*this, branch->target());
    }

    SequenceCollector* result_;
    PathCollector path_collector_;
  };
  Collector{result}(&initial_state);  // NOLINT(whitespace/braces)
}

}  // namespace core
}  // namespace scram
