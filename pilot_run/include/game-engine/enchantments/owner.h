#pragma once

#include <list>
#include "enchantment.h"
#include "owner-item.h"
#include "game-engine/enchantments/types.h"

namespace GameEngine
{
	template <typename EnchantmentTarget>
	class EnchantmentsOwner
	{
	private:
		using Manager = typename EnchantmentTypes<EnchantmentTarget>::Manager;
		using ManagerManagedItem = typename EnchantmentTypes<EnchantmentTarget>::ManagerManagedItem;
		using OwnerItem = typename EnchantmentTypes<EnchantmentTarget>::OwnerItem;
		using OwnerContainer = typename EnchantmentTypes<EnchantmentTarget>::OwnerContainer;
		using Token = typename EnchantmentTypes<EnchantmentTarget>::OwnerToken;

	public:
		EnchantmentsOwner() {}
		EnchantmentsOwner(EnchantmentsOwner<EnchantmentTarget> const& rhs) = delete;
		EnchantmentsOwner(EnchantmentsOwner<EnchantmentTarget> && rhs) = delete;
		EnchantmentsOwner<EnchantmentTarget> & operator=(EnchantmentsOwner<EnchantmentTarget> const& rhs) = delete;
		EnchantmentsOwner<EnchantmentTarget> & operator=(EnchantmentsOwner<EnchantmentTarget> && rhs) = delete;

		bool IsEmpty() const;

		void RemoveOwnedEnchantments();

		void AddEnchantment(Manager & manager, ManagerManagedItem const& managed_item);
		void RemoveEnchantment(Token token);

	private:
		OwnerContainer enchantments;
	};
} // namespace GameEngine