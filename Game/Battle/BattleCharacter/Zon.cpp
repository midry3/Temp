#include "Zon.h"

namespace Game {
	void Zon::_Load() {
		BattleCharacter::_Load();
		SetInfo({
			"ゾン",
			"chara/zon/icon.png",
			{
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
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
			},
			CoreBackend::Drawing::Color(100, 50, 100)
			});
		SetStats({
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			});
		SetSkills({
			{ "射撃", "physical", "fire", "chara/zon/attack/gun_shot.lua", 20, SkillTargetScope::Single, SkillTargetSide::Enemy, "敵一体に銃弾を放つ。" },
			{ "特製手りゅう弾", "physical", "fire", "chara/zon/attack/bomb.lua", 40, SkillTargetScope::All, SkillTargetSide::Enemy, "特製の手りゅう弾を放って敵全体にダメージを与える。" },
			{ "補給物資", "special", "water", "chara/zon/attack/supply.lua", 20, SkillTargetScope::Single, SkillTargetSide::Ally, "補給物資を配布して味方のHP、MPを回復させる。" },
			//{ "Heal", "special", "nature", "battle/heal.lua", 20 }
			});

		Initialize();
	}
}