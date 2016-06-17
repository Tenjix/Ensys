#include "System.h"

#include <ensys/World.h>

#include <utilities/Logging.h>

using namespace std;

namespace ensys {

	System::System(Priority priority) : priority(priority) {
		trace("constructing system");
		is_initialized._property_owner(this);
		is_active._property_owner(this);
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
		auto iterator = find(suitable_entities.begin(), suitable_entities.end(), entity);
		if (iterator == suitable_entities.end()) {
			trace("adding ", entity, " to ", *this);
			suitable_entities.insert(entity);
			on_entity_added(entity);
		}
	}

	void System::remove(const Entity& entity) {
		auto iterator = find(suitable_entities.begin(), suitable_entities.end(), entity);
		if (iterator != suitable_entities.end()) {
			trace("removing ", entity, " from ", *this);
			suitable_entities.erase(iterator);
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

	ostream& operator<<(ostream& output, const System& system) {
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
