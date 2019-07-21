#include "System.h"

#include <ensys/World.h>

#include <utilities/Logging.h>

namespace tenjix {

	namespace ensys {

		System::System(Priority priority) : priority(priority) {
			trace("constructing system");
			is_initialized.owner = this;
			is_active.owner = this;
		};

		System::~System() noexcept {
			trace("destructing system");
		}

		void System::update(float delta_time) {
			auto iterator = suitable_entities.begin();
			while (iterator != suitable_entities.end()) {
				Entity entity = *iterator++;
				update(entity, delta_time);
			}
		}

		void System::check(const Entity& entity) {
			trace(*this, " check ", entity);
			if (not entity.is_active) {
				remove(entity);
				return;
			}
			if (filter.accepts(entity.get_component_types())) {
				add(entity);
			} else {
				remove(entity);
			}
		}

		void System::add(const Entity& entity) {
			if (suitable_entities.insert(entity).second) {
				trace("adding ", entity, " to ", *this);
				on_entity_added(entity);
			}
		}

		void System::remove(const Entity& entity) {
			if (suitable_entities.erase(entity)) {
				trace("removing ", entity, " from ", *this);
				on_entity_removed(entity);
			}
		}

		void System::activate() {
			active = true;
		}

		void System::deactivate() {
			active = false;
		}

		const Entities& System::get_entities() const {
			return suitable_entities;
		}

		const TypeFilter& System::get_filter() const {
			return filter;
		}

		uint System::get_number_of_entities() const {
			return suitable_entities.size();
		}

		void System::remove_all_entities() {
			auto iterator = suitable_entities.begin();
			while (iterator != suitable_entities.end()) {
				Entity entity = *iterator++;
				trace("removing ", entity, " from ", *this);
				suitable_entities.erase(iterator);
				on_entity_removed(entity);
			}
		}

		std::ostream& operator<<(std::ostream& output, const System& system) {
			return (output << Type(typeid(system)));
		}

		/// properties

		bool System::get_is_initialized() const {
			return world != nullptr;
		}

		bool System::get_is_active() const {
			return is_initialized and active;
		}

	}

}
