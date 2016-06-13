#pragma once

#include <vector>

#include <utilities/Types.h>

namespace ensys {

	class IDs {

		// the initial pool size
		uint initial_pool_size;

		// the next id to be used (if there is no reusable id)
		uint next_id = 1;

		// the list of reusable ids
		std::vector<uint> reusable_ids;

		// the list of all ids (true = existing id, false = reusable id)
		std::vector<bool> ids;

	public:

		static const uint no_id = 0;

		explicit IDs(uint pool_size);

		IDs(const IDs&) = delete;
		IDs(IDs&&) = delete;

		IDs& operator=(const IDs&) = delete;
		IDs& operator=(IDs&&) = delete;

		// acquires a new id
		uint acquire();

		// announces the number of required new ids
		void require(uint number_of_new_ids);

		// releases an id from the pool
		void release(uint id);

		// checks whether this id is existing
		bool exists(uint id) const;

		void clear();

		uint count() const;

	};

}
