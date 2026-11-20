#include "Zon.h"

namespace Game {
	void Zon::_Load() {
		BattleCharacter::_Load();
		SetInfo({
			"ゾン",
			{
				{ ZonAnimation::Idle, "chara/zon/idle.png" },
				{ ZonAnimation::Attack, "chara/zon/gun_shot.png" },
				/*
				{ PleaAnimation::Damage, "chara/plea/plea_damage.png" },
				{ PleaAnimation::Death, "chara/plea/plea_death.png" },
				*/
				{ ZonAnimation::GUN_SHOT_PREPARE, "chara/zon/gun_prepare.png" },
				{ ZonAnimation::GUN_SHOT, "chara/zon/gun_shot.png" },
				{ ZonAnimation::Pick, "chara/zon/pick.png" }
			}
			});
		SetStats({
			100, 50, 10, 5, 7
			});
		SetSkills({
			{ "射撃", "special", "fire", "chara/zon/attack/gun_shot.lua" },
			{ "特製手りゅう弾", "special", "fire", "chara/zon/attack/bomb.lua" },
			{ "補給物資", "special", "water", "chara/zon/attack/supply.lua" },
			{ "Heal", "special", "nature", "battle/heal.lua" }
			});

		Initialize();
	}
}