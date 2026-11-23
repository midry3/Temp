#include "Gou.h"

namespace Game {
	void Gou::_Load() {
		BattleCharacter::_Load();
		SetInfo({
			"業",
			"chara/gou/icon.png",
			{
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
				{ GouAnimation::Idle, "chara/gou/idle.png" },
				{ GouAnimation::Attack, "chara/gou/attack.png" },
				{ GouAnimation::AttackPrepare, "chara/gou/attack_prepare.png" },
				{ GouAnimation::AttackPrepare2, "chara/gou/attack_prepare2.png" },
				/*
				{ GouAnimation::Damage, "chara/gou/gou_damage.png" },
				{ GouAnimation::Death, "chara/gou/gou_death.png" }
				*/
			},
			CoreBackend::Drawing::Color(255, 100, 100)
			});
		SetSkills({
			{ "金棒スイング", "special", "fire", "chara/gou/attack/swing.lua", 20 },
			});

		Initialize();
	}
}