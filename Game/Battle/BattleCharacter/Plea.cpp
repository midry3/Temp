#include "Plea.h"

namespace Game {
	void Plea::_Load() {
		BattleCharacter::_Load();
		SetInfo({
			"プレア",
			{
				{ PleaAnimation::Idle, "chara/plea/idle.png" },
				/*
				{ PleaAnimation::Attack, "chara/plea/plea_attack.png" },
				{ PleaAnimation::Damage, "chara/plea/plea_damage.png" },
				{ PleaAnimation::Death, "chara/plea/plea_death.png" }
				*/
			}
			});
		SetStats({
			100, 50, 10, 5, 7
			});

		Initialize();
	}
}