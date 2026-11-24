#pragma once
#include <Game/Board/BoardBase.h>

namespace Game {
	class CommonData {
	public:
		static SharedPtr<BoardBase> CurrentBoard;
		// 直近の戦闘で敵を全滅させた（倒した）かどうか。BoardBase::StartBattle で false に戻し、
		// BattleBase が Won 状態に入ったときに true にする。BoardBase が盤面復帰時に参照する。
		static bool LastBattleEnemyDefeated;
		// 直近の戦闘で参加した味方が全滅したかどうか。BoardBase::StartBattle で false に戻し、
		// BattleBase が Lost 状態に入ったときに true にする。BoardBase が盤面復帰時に参照する。
		static bool LastBattleAlliesDefeated;
		// 直近の戦闘に参加していない味方が盤面に残っているかどうか。
		// BoardBase::StartBattle で参加者を確定した後に設定する。
		// 敗北時に true なら Board へ戻ってステージを継続し、false ならステージ選択へ移行する。
		static bool LastBattleCanContinue;
	};
}