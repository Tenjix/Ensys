#pragma once

#include <memory>
#include <vector>

#include <utilities/Logging.h>
#include <utilities/Standard.h>

namespace ensys {

	class Component {

	public:

		Component() {
			trace("constructing component [", this, "]");
		}

		virtual	~Component() noexcept {
			trace("destructing component [", this, "]");
		}

	};

	using Components = std::vector<linked<Component>>;

}
