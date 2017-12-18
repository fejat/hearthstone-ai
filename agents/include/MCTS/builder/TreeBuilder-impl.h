#pragma once

#include <cstdlib>

#include "MCTS/builder/TreeBuilder.h"
#include "MCTS/board/BoardView-impl.h"
#include "MCTS/board/ActionParameterGetter-impl.h"
#include "MCTS/board/RandomGenerator-impl.h"
#include "MCTS/detail/BoardNodeMap-impl.h"

namespace mcts
{
	namespace builder
	{
		// Note: can only be called when current player is the viewer of 'board'
		inline TreeBuilder::SelectResult TreeBuilder::PerformSelect(
			TreeNode * node, board::Board const& board, 
			detail::BoardNodeMap & last_node_map, TreeUpdater * updater)
		{
			assert(node);

			board_ = &board;

			assert(node->GetAddon().consistency_checker.CheckBoard(board.CreateView()));
			selection_stage_.StartNewMainAction(node);

			TreeBuilder::SelectResult perform_result(ApplyAction(selection_stage_));
			assert(perform_result.result.type_ != Result::kResultInvalid);

			// we use mutable here, since we will throw it away after all
			auto & traversed_path = selection_stage_.GetMutableTraversedPath();

			// mark the last action as a redirect node
			traversed_path.back().ConstructRedirectNode();

			bool new_node_created = false;
			if (perform_result.result.type_ == Result::kResultNotDetermined) {
				perform_result.node = last_node_map.GetOrCreateNode(board, &new_node_created);
				assert(perform_result.node);
			}
			else {
				assert(perform_result.node == nullptr);

				if (perform_result.result.type_ == Result::kResultWin) {
					perform_result.node = mcts::selection::TreeNode::GetWinNode();
				}
				else {
					assert(perform_result.result.type_ == Result::kResultLoss);
					perform_result.node = mcts::selection::TreeNode::GetLossNode();
				}
				assert(perform_result.node != nullptr);
			}

			if (!new_node_created) {
				new_node_created = selection_stage_.HasNewNodeCreated();
			}

			assert(perform_result.change_to_simulation == false); // default value
			if (new_node_created) {
				perform_result.change_to_simulation = true;
			}
			else if (traversed_path.back().GetEdgeAddon()->GetChosenTimes() < StaticConfigs::kSwitchToSimulationUnderChosenTimes) {
				perform_result.change_to_simulation = true;
			}

			assert(updater);
			updater->PushBackNodes(traversed_path, perform_result.node);

			assert([&](builder::TreeBuilder::TreeNode* node) {
				if (perform_result.result.type_ != Result::kResultNotDetermined) return true;
				if (!node->GetActionType().IsValid()) return true;
				return node->GetActionType().GetType() == FlowControl::ActionType::kMainAction;
			}(perform_result.node));

			return perform_result;
		}
		
		// Never returns kResultInvalid. Will automatically retry if an invalid action is applied
		// Note: can only be called when current player is the viewer of 'board'
		inline Result TreeBuilder::PerformSimulate(board::Board const& board)
		{
			board_ = &board;

			Result result = simulation_stage_.CutoffCheck(*board_);
			assert(result.type_ != Result::kResultInvalid);
			if (result.type_ != Result::kResultNotDetermined) return result;

			return ApplyAction(simulation_stage_);
		}

		template <typename StageHandler>
		inline Result TreeBuilder::ApplyAction(StageHandler&& stage_handler)
		{
			constexpr bool is_simulation = std::is_same_v<
				std::decay_t<StageHandler>,
				simulation::Simulation>;

			auto current_state_view = board_->GetCurrentPlayerStateView();
			action_parameter_getter_.Initialize(current_state_view);
			int choices = action_parameter_getter_.GetAnalyzer().GetMainActionsCount();
			assert(choices > 0); // at least end-turn should be valid

			int choice = -1;
			Result result = Result::kResultInvalid;
			if constexpr (is_simulation) {
				choice = this->ChooseSimulateAction(
					FlowControl::ActionType(FlowControl::ActionType::kMainAction),
					FlowControl::ActionChoices(choices));
			}
			else {
				choice = this->ChooseSelectAction(
					FlowControl::ActionType(FlowControl::ActionType::kMainAction),
					FlowControl::ActionChoices(choices));
			}

			action_parameter_getter_.SetMainOpIndex(choice);
			result = board_->ApplyAction(random_generator_, action_parameter_getter_);
			assert(result.type_ != Result::kResultInvalid);

			statistic_.ApplyActionSucceeded(is_simulation);
			return result;
		}

		inline int TreeBuilder::ChooseSelectAction(FlowControl::ActionType action_type, FlowControl::ActionChoices const& choices)
		{
			assert(!choices.Empty());
			int choice = selection_stage_.ChooseAction(*board_, action_type, choices);
			assert(choice >= 0); // always return a valid choice
			return choice;
		}

		inline int TreeBuilder::ChooseSimulateAction(FlowControl::ActionType action_type, FlowControl::ActionChoices const& choices)
		{
			int choice = simulation_stage_.ChooseAction(
				*board_,
				action_parameter_getter_.GetAnalyzer(),
				action_type, choices);
			assert(choice >= 0);
			return choice;
		}
	}
}