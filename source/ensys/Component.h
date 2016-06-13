#pragma once

#include <memory>
#include <vector>

#include <ensys/Namespace.h>

#include <utilities/Logging.h>
#include <utilities/Standard.h>

namespace ensys {

	struct Component {

		Component() {
			trace("constructing component [", this, "]");
		}

		virtual	~Component() noexcept {
			trace("destructing component [", this, "]");
		}

		//Component& operator=(const Component&) = delete;
		//Component& operator=(Component&&) = delete;

	};

	using Components = std::vector<linked<Component>>;

}
