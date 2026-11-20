#include "Gou.h"

namespace Game {
	void Gou::_Load() {
		BattleCharacter::_Load();
		SetInfo({
			"業",
			{
				{ GouAnimation::Idle, "chara/gou/idle.png" },
				{ GouAnimation::Attack, "chara/gou/attack.png" },
				{ GouAnimation::AttackPrepare, "chara/gou/attack_prepare.png" },
				{ GouAnimation::AttackPrepare2, "chara/gou/attack_prepare2.png" },
				/*
				{ GouAnimation::Damage, "chara/gou/gou_damage.png" },
				{ GouAnimation::Death, "chara/gou/gou_death.png" }
				*/
			}
			});
		SetStats({
			100, 50, 10, 5, 7
			});
		SetSkills({
			{ "金棒スイング", "special", "fire", "chara/gou/attack/swing.lua" },
			{ "Fire", "special", "fire", "battle/fire.lua" },
			{ "Ice", "special", "water", "battle/ice.lua" },
			{ "Heal", "special", "nature", "battle/heal.lua" }
			});

		Initialize();
	}
}