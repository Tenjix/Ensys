#pragma once

#include <utilities/Logging.h>
#include <utilities/Standard.h>

namespace tenjix {

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

		using Components = Lot<linked<Component>>;

	}

}
