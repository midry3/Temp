#pragma once
#include <string>

namespace Game {
	struct SkillInfo {
		std::string name;
		std::string primaryAttribute;
		std::string secondaryAttribute;
		std::string directionFile;
		int mpCost = 10;
	};
}
