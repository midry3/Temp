#include "Plea.h"

namespace Game {
	void Plea::_Load() {
		BattleCharacter::_Load();
		SetInfo({
			"プレア",
			{
				{ PleaAnimation::Idle, "chara/plea/idle.png" },
				{ PleaAnimation::Attack, "chara/plea/attack_front.png" },
				/*
				{ PleaAnimation::Damage, "chara/plea/plea_damage.png" },
				{ PleaAnimation::Death, "chara/plea/plea_death.png" }
				*/
			}
			});
		SetStats({
			100, 50, 10, 5, 7
			});
		SetSkills({
			{ "爆破魔法", "special", "fire", "chara/plea/attack/fire.lua" },
			{ "Fire", "special", "fire", "battle/fire.lua" },
			{ "Ice", "special", "water", "battle/ice.lua" },
			{ "Heal", "special", "nature", "battle/heal.lua" }
			});

		Initialize();
	}
}