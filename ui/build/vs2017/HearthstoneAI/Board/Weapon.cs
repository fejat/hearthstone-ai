﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.Serialization;

namespace HearthstoneAI.Board
{
    [DataContract]
    class Weapon
    {
        [DataMember]
        public bool equipped;

        [DataMember]
        public int entity_id;

        [DataMember]
        public string card_id = "";

        [DataMember]
        public int attack;

        [DataMember]
        public int durability;

        [DataMember]
        public int damage;

        [DataMember]
        public Enchantments enchantments = new Enchantments();

        public bool Parse(State.Game game, State.ReadOnlyEntity player)
        {
            bool ret = true;

            this.equipped = false;

            int controller = player.GetTagOrDefault(State.GameTag.CONTROLLER, -1);
            if (controller < 0) return false;

            foreach (var entity in game.Entities.Items)
            {
                if (entity.Value.GetTagOrDefault(State.GameTag.CONTROLLER, controller - 1) != controller) continue;

                if (!entity.Value.HasTag(State.GameTag.ZONE)) continue;
                if (entity.Value.GetTag(State.GameTag.ZONE) != (int)State.TAG_ZONE.PLAY) continue;

                if (!entity.Value.HasTag(State.GameTag.CARDTYPE)) continue;
                var card_type = (State.TAG_CARDTYPE)entity.Value.GetTag(State.GameTag.CARDTYPE);

                if (card_type != State.TAG_CARDTYPE.WEAPON) continue;

                if (this.equipped)
                {
                    // equip two weapons!
                    return false;
                }

                this.entity_id = entity.Value.Id;
                this.card_id = entity.Value.CardId;
                this.attack = entity.Value.GetTagOrDefault(State.GameTag.ATK, 0);
                this.durability = entity.Value.GetTagOrDefault(State.GameTag.DURABILITY, 0);
                this.damage = entity.Value.GetTagOrDefault(State.GameTag.DAMAGE, 0);
                ret = this.enchantments.Parse(game, entity.Value) && ret;

                this.equipped = true;
            }

            return ret;
        }

        public override bool Equals(object obj)
        {
            Weapon rhs = obj as Weapon;
            if (rhs == null) return false;
            if (!this.entity_id.Equals(rhs.entity_id)) return false;
            if (!this.equipped.Equals(rhs.equipped)) return false;
            if (!this.card_id.Equals(rhs.card_id)) return false;
            if (!this.enchantments.Equals(rhs.enchantments)) return false;
            return true;
        }

        public override int GetHashCode()
        {
            return new
            {
                this.entity_id,
                this.equipped,
                this.card_id,
                this.enchantments
            }.GetHashCode();
        }
    }
}
