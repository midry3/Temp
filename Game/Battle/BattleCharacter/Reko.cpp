#include "Reko.h"

namespace Game {
	void Reko::_Load() {
		BattleCharacter::_Load();
		SetInfo({
			"レコ",
			{
				{ RekoAnimation::Idle, "chara/reko/idle.png" },
				/*
				{ RekoAnimation::Attack, "chara/reko/reko_attack.png" },
				{ RekoAnimation::Damage, "chara/reko/reko_damage.png" },
				{ RekoAnimation::Death, "chara/reko/reko_death.png" }
				*/
			}
			});
		SetStats({
			100, 50, 10, 5, 7
			});

		Initialize();
	}
}