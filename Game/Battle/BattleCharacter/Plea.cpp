#include "Plea.h"

namespace Game {
	void Plea::_Load() {
		BattleCharacter::_Load();
		SetInfo({
			"プレア",
			"chara/plea/icon.png",
			{
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
			DEFAULT_MAX_HP, DEFAULT_MAX_MP, DEFAULT_ATK, DEFAULT_DEF, DEFAULT_MDEF, DEFAULT_SPEED
			},
			{
				{ PleaAnimation::Idle, "chara/plea/idle.png" },
				{ PleaAnimation::Attack, "chara/plea/attack_front.png" },
				/*
				{ PleaAnimation::Damage, "chara/plea/plea_damage.png" },
				{ PleaAnimation::Death, "chara/plea/plea_death.png" }
				*/
				{ PleaAnimation::Chanting, "chara/plea/chanting.png" }
			},
			CoreBackend::Drawing::Color(80, 50, 150)
			});
		SetSkills({
			{ "爆破魔法", "special", "fire", "chara/plea/attack/fire.lua", 50, SkillTargetScope::Single, SkillTargetSide::Enemy, "炎の魔法により敵一体にダメージを与える。"},
			{ "流星魔法", "special", "", "chara/plea/attack/shooting_star.lua", 100, SkillTargetScope::All, SkillTargetSide::Enemy, "星の破片を放って敵全体にダメージを与える。" },
			{ "二重詠唱", "special", "", "chara/plea/attack/chanting.lua", 10, SkillTargetScope::Single, SkillTargetSide::Self, "詠唱により攻撃力を 2 倍にする。" },
			//{ "転送魔法", "special", "", "chara/plea/attack/teleport.lua", 20, SkillTargetScope::Single, SkillTargetSide::Ally }
			});

		Initialize();
	}
}