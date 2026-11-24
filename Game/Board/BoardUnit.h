#pragma once
#include <Scene/Button/Button.h>
#include <Scene/Text/TextLabel.h>
#include <vector>
#include "BoardTypes.h"
#include "../Battle/BattleCharacter/BattleCharacter.h"

class Sprite2D;
class Texture;

namespace Game {
	class BoardUnit : public Button {
	private:
		static constexpr const char* HOVER_SE = "sound/one_shot/select_2.oga";

		BoardUnitData _data;
		SharedPtr<Sprite2D> _sprite;
		SharedPtr<TextLabel> _nameLabel;
		bool _intentHint = false, _isMouseEntered = false;
		double _hintTime = 0.0;
		std::vector<BoardOffset> _relativeMoveOffsets;
		// Board 単位で保持する永続キャラクター。ステータス変化はこの状態に蓄積され、
		// 戦闘開始時に戦闘側へ引き継がれ、戦闘終了時に書き戻される。
		SharedPtr<BattleCharacter> _character;

	protected:
		void _Load() override;
		void _Update(float dt) override;

	public:
		void SetUnit(const BoardUnitData& data, const SharedPtr<Texture>& texture, const Vector2D& tileSize);
		void SetBoardPosition(int x, int y) { _data.x = x; _data.y = y; }
		[[nodiscard]] const BoardUnitData& GetData() const { return _data; }
		[[nodiscard]] bool IsAlly() const { return _data.team == BoardTeam::Ally; }
		// タイル上のアイコン表示を切り替える（同じマスに重なる場合は代表だけを表示する）
		void SetIconVisible(bool visible);
		void SetIntentHint(bool hint);
		[[nodiscard]] bool HasIntentHint() const { return _intentHint; }
		void SetRelativeMoveOffsets(const std::vector<BoardOffset>& offsets) { _relativeMoveOffsets = offsets; }
		[[nodiscard]] const std::vector<BoardOffset>& GetRelativeMoveOffsets() const { return _relativeMoveOffsets; }
		void SetCharacter(const SharedPtr<BattleCharacter>& character) { _character = character; }
		[[nodiscard]] const SharedPtr<BattleCharacter>& GetCharacter() const { return _character; }
	};
}
