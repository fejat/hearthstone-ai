#pragma once

#include "MCTS/Config.h"
#include "MCTS/simulation/Tree.h"
#include "MCTS/policy/Simulation.h"

namespace mcts
{
	namespace simulation
	{
		class Simulation
		{
		public:
			Simulation() : tree_() {}

			void StartNewAction() {
				tree_.Clear();
			}

			int ChooseAction(board::Board const& board, ActionType action_type, board::ActionChoicesGetter const& action_choices_getter) {
				// TODO: support different types of action choices
				board::ActionChoices action_choices = action_choices_getter();
				if (action_choices.Empty()) return -1;
				int choices = action_choices.Size();

				tree_.FillChoices(choices);

				int choice = Simulate(board, action_type, choices);
				if (choice < 0) return -1;

				tree_.ApplyChoice(choice);
				return choice;
			}

			void ReportInvalidAction() {
				tree_.ReportInvalidChoice();
			}

			void RestartAction() {
				// Use a white-list-tree to record all viable (sub-)actions
				tree_.Restart();
			}

		private:
			int Simulate(board::Board const& board, ActionType action_type, int choices) const
			{
				size_t valid_choices = tree_.GetWhiteListCount();
				if (valid_choices <= 0) return -1;

				if (action_type.IsChosenRandomly()) {
					int rnd = StaticConfigs::SimulationPhaseRandomActionPolicy::GetRandom((int)valid_choices);
					return (int)tree_.GetWhiteListItem((size_t)rnd);
				}

				int choice = StaticConfigs::SimulationPhaseSelectActionPolicy::GetChoice(
					policy::simulation::ChoiceGetter(tree_), board
				);

				return choice;
			}

		private:
			Tree tree_;
		};
	}
}