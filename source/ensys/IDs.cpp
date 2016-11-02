#include "IDs.h"

#include <algorithm>

namespace tenjix {

	namespace ensys {

		IDs::IDs(uint pool_size) : initial_pool_size(pool_size), ids(pool_size), next_id(0) {}

		uint IDs::acquire() {
			uint id;
			if (not reusable_ids.empty()) {
				id = reusable_ids.back();
				reusable_ids.pop_back();
				ids[id] = true;
			} else {
				id = next_id++;
				ids[id] = true;
			}
			return id;
		}

		void IDs::require(uint number_of_new_ids) {
			auto number_of_ids = count() + number_of_new_ids;
			if (number_of_ids > ids.size()) {
				ids.resize(number_of_ids);
			}
		}

		void IDs::release(uint id) {
			ids[id] = false;
			reusable_ids.push_back(id);
		}

		bool IDs::exists(uint id) const {
			return id < ids.size() && ids[id];
		}

		uint IDs::count() const {
			return std::count(ids.begin(), ids.end(), true);
		}

		void IDs::clear() {
			next_id = 0;
			ids.clear();
			reusable_ids.clear();
			ids.resize(initial_pool_size);
		}

	}

}
