#pragma once

#include <utilities/Types.h>

namespace tenjix {

	namespace ensys {

		class IDs {

			// the next id to be used (if there is no reusable id)
			uint next_id;

			// the list of reusable ids
			Lot<uint> reusable_ids;

			// the list of all ids (true = existing id, false = reusable id)
			Lot<bool> ids;

		public:

			static constexpr uint No_Id = 0;

			explicit IDs(uint initial_pool_size);

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

}
