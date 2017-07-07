#pragma once

#include "MCTS/board/BoardView.h"

namespace mcts
{
	namespace board
	{
		inline BoardView::BoardView(state::PlayerSide side, state::State const& state) :
			turn_(state.GetTurn()),
			self_hero_(), self_crystal_(), self_hero_power_(), self_weapon_(),
			self_minions_(), self_hand_(), self_deck_(),
			opponent_hero_(), opponent_crystal_(), opponent_hero_power_(),
			opponent_weapon_(), opponent_minions_(), opponent_hand_(), opponent_deck_()
		{
			{
				state::board::Player const& self_player = state.GetBoard().Get(side);
				FlowControl::ValidActionGetter valid_action_getter(state);

				state::CardRef hero_ref = self_player.GetHeroRef();
				assert(hero_ref.IsValid());
				self_hero_.Fill(state.GetCard(hero_ref),
					valid_action_getter.IsAttackable(hero_ref));

				self_crystal_.Fill(self_player);

				state::CardRef hero_power_ref = self_player.GetHeroPowerRef();
				assert(hero_power_ref.IsValid());
				self_hero_power_.Fill(state.GetCard(hero_power_ref));

				state::CardRef weapon_ref = self_player.GetWeaponRef();
				if (weapon_ref.IsValid()) self_weapon_.Fill(state.GetCard(self_player.GetWeaponRef()));
				else self_weapon_.Invalidate();

				self_player.minions_.ForEach([&](state::CardRef card_ref) {
					boardview::SelfMinion item;
					item.Fill(state.GetCard(card_ref),
						valid_action_getter.IsAttackable(card_ref));
					self_minions_.push_back(std::move(item));
					return true;
				});

				self_player.hand_.ForEach([&](state::CardRef card_ref) {
					boardview::SelfHandCard item;
					item.Fill(state.GetCard(card_ref));
					self_hand_.push_back(std::move(item));
					return true;
				});

				self_deck_.Fill(self_player.deck_);
			}

			{
				state::PlayerIdentifier opponent_side = state::PlayerIdentifier(side).Opposite();
				state::board::Player const& opponent_player = state.GetBoard().Get(opponent_side);

				state::CardRef hero_ref = opponent_player.GetHeroRef();
				assert(hero_ref.IsValid());
				opponent_hero_.Fill(state.GetCard(hero_ref));

				opponent_crystal_.Fill(opponent_player);

				state::CardRef hero_power_ref = opponent_player.GetHeroPowerRef();
				assert(hero_power_ref.IsValid());
				opponent_hero_power_.Fill(state.GetCard(hero_power_ref));

				state::CardRef weapon_ref = opponent_player.GetWeaponRef();
				if (weapon_ref.IsValid()) opponent_weapon_.Fill(state.GetCard(opponent_player.GetWeaponRef()));
				else opponent_weapon_.Invalidate();

				opponent_player.minions_.ForEach([&](state::CardRef card_ref) {
					boardview::Minion item;
					item.Fill(state.GetCard(card_ref));
					opponent_minions_.push_back(std::move(item));
					return true;
				});

				opponent_player.hand_.ForEach([&](state::CardRef card_ref) {
					boardview::SelfHandCard item;
					item.Fill(state.GetCard(card_ref));
					self_hand_.push_back(std::move(item));
					return true;
				});

				opponent_deck_.Fill(opponent_player.deck_);
			}
		}
	}
}