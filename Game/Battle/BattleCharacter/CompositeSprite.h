#pragma once
#include <Scene.h>
#include <Scene/Texture/AnimatedSprite2D.h>
#include <initializer_list>
#include <map>
#include <optional>
#include <vector>

namespace Game {
	struct PartInfo {
		int group = 0;
		Path path;
		Vector2D position;
		std::optional<bool> loop = std::nullopt;
		bool reverse = false;
	};

	class CompositeSprite : public Scene {
	protected:
		std::vector<SharedPtr<AnimatedSprite2D>> _allParts;
		std::map<int, std::vector<SharedPtr<AnimatedSprite2D>>> _partsByGroup;
		void _Load() override;
	public:
		void AddPart(const PartInfo& part);
		void AddParts(std::initializer_list<PartInfo> parts);
		template<typename... Parts>
		void AddParts(const PartInfo& first, const Parts&... rest) {
			AddPart(first);
			(AddPart(rest), ...);
		}
		[[nodiscard]] const std::vector<SharedPtr<AnimatedSprite2D>>& GetParts(int group) const;
		[[nodiscard]] const std::vector<SharedPtr<AnimatedSprite2D>>& GetAllParts() const {
			return _allParts;
		}
		[[nodiscard]] const std::vector<SharedPtr<AnimatedSprite2D>>& GetPartsByGroup(int group) const {
			return GetParts(group);
		}
	};
}
