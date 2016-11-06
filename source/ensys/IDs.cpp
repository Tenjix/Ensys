#include "IDs.h"

#include <algorithm>

namespace tenjix {

	namespace ensys {

		IDs::IDs(uint initial_pool_size = 100) : ids(1 + initial_pool_size, false), next_id(1) {}

		uint IDs::acquire() {
			uint id;
			if (not reusable_ids.empty()) {
				id = reusable_ids.back();
				reusable_ids.pop_back();
				ids[id] = true;
			} else {
				id = next_id++;
				uint size = ids.size();
				if (id > ids.size()) {
					ids.resize(id, false);
				}
				if (id == ids.size()) {
					ids.push_back(true);
				} else {
					ids[id] = true;
				}
			}
			return id;
		}

		void IDs::require(uint number_of_new_ids) {
			auto number_of_ids = count() + number_of_new_ids;
			ids.reserve(number_of_ids);
		}

		void IDs::release(uint id) {
			if (not exists(id)) return;
			ids[id] = false;
			reusable_ids.push_back(id);
		}

		bool IDs::exists(uint id) const {
			return id != 0 and id < ids.size() and ids[id];
		}

		uint IDs::count() const {
			return std::count(ids.begin(), ids.end(), true);
		}

		void IDs::clear() {
			next_id = 1;
			reusable_ids.clear();
			ids.resize(1);
			ids.shrink_to_fit();
		}

	}

}
