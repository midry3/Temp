#include "CompositeSprite.h"

namespace Game {
	void CompositeSprite::_Load() {}

	void CompositeSprite::AddPart(const PartInfo& part) {
		const auto sprite = CreateScene<AnimatedSprite2D>();
		sprite->AddAnimation(0, LoadAsset<SpriteSheet>(part.path));
		sprite->SetPosition(part.position);
		sprite->Play(0, part.loop, part.reverse);
		_allParts.push_back(sprite);
		_partsByGroup[part.group].push_back(sprite);
	}

	void CompositeSprite::AddParts(std::initializer_list<PartInfo> parts) {
		for (const auto& part : parts) {
			AddPart(part);
		}
	}

	const std::vector<SharedPtr<AnimatedSprite2D>>& CompositeSprite::GetParts(int group) const {
		const auto it = _partsByGroup.find(group);
		static const std::vector<SharedPtr<AnimatedSprite2D>> empty;
		if (it == _partsByGroup.end()) return empty;
		return it->second;
	}
}
