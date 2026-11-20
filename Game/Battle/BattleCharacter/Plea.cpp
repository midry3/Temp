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
				{ PleaAnimation::Chanting, "chara/plea/chanting.png" }
			}
			});
		SetStats({
			100, 50, 10, 5, 7
			});
		SetSkills({
			{ "爆破魔法", "special", "fire", "chara/plea/attack/fire.lua" },
			{ "流星魔法", "special", "fire", "chara/plea/attack/shooting_star.lua" },
			{ "二重詠唱", "special", "water", "chara/plea/attack/chanting.lua" },
			{ "Heal", "special", "nature", "battle/heal.lua" }
			});

		Initialize();
	}
}