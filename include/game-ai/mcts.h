#ifndef GAME_AI_MCTS_H
#define GAME_AI_MCTS_H

#include <random>
#include <list>
#include <map>
#include <unordered_set>
#include "game-engine/game-engine.h"
#include "game-ai/tree.h"
#include "game-ai/board-node-map.h"
#include "game-ai/start-board.h"

class MCTS
{
public:
	MCTS();
	~MCTS();
	MCTS(const MCTS&) = delete;
	MCTS &operator=(const MCTS&) = delete;
	MCTS(MCTS&&) = delete;
	MCTS &operator=(MCTS&&) = delete;

public:
	void Initialize(unsigned int rand_seed, StartBoard && start_board);
	void Iterate();

public: // TODO: to be private?
	StartBoard start_board;
	GameEngine::Board current_iteration_root_node_board;
	Tree tree;
	BoardNodeMap board_node_map;
	bool tree_root_node_initialized;

private:
	void GenerateRootNodeBoard();
	void SelectAndExpand(TreeNode* & node, GameEngine::Board & board);
	bool Simulate(GameEngine::Board &board);
	void BackPropagate(bool is_win);

	// return true if the stage uses next-move-getter to get next moves
	bool UseNextMoveGetter(TreeNode * node);

	int GetRandom();
	TreeNode* CreateChildNode(TreeNode* const node, GameEngine::Move const& next_move, GameEngine::Board & next_board);
	TreeNode* CreateRedirectNode(TreeNode * parent, GameEngine::Move const& move, TreeNode * target_node);
	bool ExpandNewNode(TreeNode * & node, GameEngine::Board & board);
	bool ExpandNodeWithDeterministicNextMoves(TreeNode * & node, GameEngine::Board & board);
	bool ExpandNodeWithSingleRandomNextMove(TreeNode * & node, GameEngine::Board & board);
	bool ExpandNodeWithMultipleRandomNextMoves(TreeNode * & node, GameEngine::Board & board);

private: // for internal use
	TreeNode *allocated_node;

	std::vector<TreeNode*> traversed_nodes;
};

#endif
