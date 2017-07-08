#pragma once

#include <unordered_map>
#include <memory>

#include "MCTS/Types.h"
#include "MCTS/detail/TreeNodeBase.h"
#include "MCTS/selection/TreeNodeAddon.h"
#include "MCTS/board/ActionChoices.h"
#include "MCTS/selection/EdgeAddon.h"
#include "MCTS/selection/ChildNodeMap.h"

namespace mcts
{
	namespace selection
	{
		class TreeNode
		{
		public:
			class ChoiceIterator {
			public:
				ChoiceIterator(board::ActionChoices & choices, ChildNodeMap & children) :
					choices_(choices), children_(children),
					current_choice_(0), current_child_(nullptr)
				{}

				void Begin() { choices_.Begin(); }
				void StepNext() { choices_.StepNext(); }
				bool IsEnd() { return choices_.IsEnd(); }

				enum CheckResult {
					kForceSelectChoice,
					kNormalChoice,
					kInvalidChoice
				};
				CheckResult Check() {
					current_choice_ = choices_.Get();
					assert(current_choice_ >= 0);

					current_child_ = children_.Get(current_choice_);

					if (!current_child_) {
						return kForceSelectChoice; // not expanded before
					}

					if (!current_child_->GetNode()) {
						// Marked as invalid choice before
						return kInvalidChoice;
					}

					return kNormalChoice;
				}

				int GetChoice() const { return current_choice_; }
				mcts::selection::EdgeAddon const& GetAddon() const {
					return current_child_->GetEdgeAddon();
				}
				TreeNode* GetNode() const {
					return current_child_->GetNode();
				}

			private:
				board::ActionChoices & choices_;
				ChildNodeMap & children_;

				int current_choice_;
				ChildType * current_child_;
			};

			TreeNode() : 
				action_type_(ActionType::kInvalid),
				choices_type_(board::ActionChoices::kInvalid),
				children_(), addon_()
			{}

			// select among specific choices
			// if any of the choices does not exist, return the edge to expand it
			//    A new node will be allocated
			// if all of the specific exist, use select_callback to select one of them
			//    Call select_callback.ReportChoicesCount(int count)
			//        Include invalid choices
			//    Call select_callback.AddChoice(int choice, EdgeAddon, TreeNode* node) for each choices
			//        If an action is marked invalid before, a nullptr is passed to the 'node' parameter
			//    Call select_callback.SelectChoice() -> TreeNode to get result
			// Return -1 if all choices are invalid.
			template <typename SelectCallback>
			int Select(ActionType action_type, board::ActionChoices choices, SelectCallback && select_callback)
			{
				if (choices_type_ == board::ActionChoices::kInvalid) {
					choices_type_ = choices.GetType();
				}
				else {
					assert(choices_type_ == choices.GetType());
				}

				if (action_type_.IsValid()) {
					assert(action_type_ == action_type);
				}
				else {
					action_type_ = action_type;
				}

				ChoiceIterator choice_iterator(choices, children_);
				return select_callback.SelectChoice(
					ChoiceIterator(choices, children_)
				);
			}

			// @return  (pointer to child, is_just_expanded)
			struct FollowStatus {
				bool just_expanded;
				EdgeAddon & edge_addon;
				TreeNode & node;
			};
			FollowStatus FollowChoice(int choice)
			{
				ChildType* child = children_.Get(choice);
				bool just_expanded = false;
				if (!child) {
					child = children_.Create(choice);
					child->SetNode(new TreeNode());
					just_expanded = true;
				}

				TreeNode* child_node = child->GetNode();
				assert(child_node); // should only follow valid choices
				return { just_expanded, child->GetEdgeAddon(), *child_node };
			}

			EdgeAddon& MarkChoiceInvalid(int choice)
			{
				// the child node is not yet created
				// since we delay the node creation as late as possible
				ChildType* child = children_.Get(choice);
				if (child) {
					child->SetNode(nullptr); // mark invalid
				}
				else {
					child = children_.Create(choice); // create an invalid child node
				}
				return child->GetEdgeAddon();
			}

			EdgeAddon& MarkChoiceRedirect(int choice, TreeNode* node)
			{
				// the child node is not yet created
				// since we delay the node creation as late as possible
				ChildType* child = children_.Get(choice);
				if (!child) child = children_.Create(choice);
				assert(child);
				child->SetAsRedirectNode(node);
				return child->GetEdgeAddon();
			}

		public:
			ActionType GetActionType() const { return action_type_; }

		public:
			TreeNodeAddon const& GetAddon() const { return addon_; }
			TreeNodeAddon & GetAddon() { return addon_; }

		private:
			ActionType action_type_;
			board::ActionChoices::Type choices_type_; // TODO: debug only

			ChildNodeMap children_;
			TreeNodeAddon addon_;
		};
	}
}