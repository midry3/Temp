#pragma once
#include <Game/Board/BoardBase.h>

namespace Game {
	class CommonData {
	public:
		static SharedPtr<BoardBase> CurrentBoard;
		// 直近の戦闘で敵を全滅させた（倒した）かどうか。BoardBase::StartBattle で false に戻し、
		// BattleBase が Won 状態に入ったときに true にする。BoardBase が盤面復帰時に参照する。
		static bool LastBattleEnemyDefeated;
	};
}