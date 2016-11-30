#pragma once

#include <assert.h>
#include "FlowControl/Dispatchers/Auras.h"
#include "FlowControl/Manipulators/Helpers/AuraHelper.h"
#include "FlowControl/Manipulate.h"

namespace FlowControl
{
	namespace Manipulators
	{
		namespace Helpers
		{
			struct AuraHelperDetail_AuraEnchantment
			{
				static constexpr EnchantmentTiers tier = kEnchantmentAura;

				template <typename T1>
				AuraHelperDetail_AuraEnchantment(T1&& apply_functor)
					: apply_functor(std::forward<T1>(apply_functor))
				{}
				
				state::Cards::Enchantments::ApplyFunctor apply_functor;
			};

			void AuraHelper::Update()
			{

				if (card_.GetAuraId() < 0) return; // no aura attached

				state::Cards::AuraAuxData & data = card_.GetMutableAuraAuxDataGetter().Get();

				std::unordered_set<state::CardRef> new_targets;
				Dispatchers::Auras::GetEligibles(card_.GetAuraId(), state_, card_ref_, card_, new_targets);

				for (auto it = data.applied_enchantments.begin(), it2 = data.applied_enchantments.end(); it != it2;)
				{
					auto new_target_it = new_targets.find(it->first);
					if (new_target_it != new_targets.end()) {
						// already applied
						new_targets.erase(new_target_it);
						++it;
					}
					else {
						// enchantments should be removed
						Manipulate(state_, flow_context_).Minion(it->first).Enchant().Remove<AuraHelperDetail_AuraEnchantment>(it->second);
						it = data.applied_enchantments.erase(it);
					}
				}

				for (auto new_target : new_targets) {
					// enchantments should be applied
					assert(data.applied_enchantments.find(new_target) == data.applied_enchantments.end());

					state::Cards::Enchantments::ApplyFunctor enchantment_functor;
					Dispatchers::Auras::CreateEnchantmentFor(card_.GetAuraId(), state_, new_target, enchantment_functor);

					auto enchant_identifier = Manipulate(state_, flow_context_).Minion(new_target).Enchant()
						.Add(AuraHelperDetail_AuraEnchantment(std::move(enchantment_functor)));
					data.applied_enchantments.insert(std::make_pair(new_target, std::move(enchant_identifier)));
				}
			}

		}
	}
}