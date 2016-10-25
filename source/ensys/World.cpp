#include "World.h"

#include <utilities/Logging.h>
#include <utilities/Strings.h>

using namespace std;

namespace ensys {

	World::World(String name, uint initial_entity_pool_size) : name(name), entity_ids(initial_entity_pool_size) {}

	void World::update(float delta_time) {
		for (auto& entry : priorities) {
			auto& systems = entry.second;
			for (System* system : systems) {
				if (system->is_active) system->update(delta_time);
			}
		}
	}

	void World::update_systems(const Entity& entity) {
		if (disable_system_checks) return;
		trace("update systems with ", entity);
		for (auto& entry : systems) {
			System& system = *entry.second;
			system.check(entity);
		}
	}

	void World::update_system(System& system) {
		if (disable_system_checks) return;
		trace("update system ", system);
		for (auto& entity : entities) {
			system.check(entity);
		}
	}

	void World::clear() {
		trace("clearing ", *this);
		remove_all_systems();
		remove_all_entities();
		components.clear();
		priorities.clear();
	}

	Entity World::create_entity(const String& name, const function<void(Entity)>& function) {
		trace("creating entity \"", name, "\" in ", *this);
		Entity::Id id = entity_ids.acquire();
		attributes.emplace(id, Attributes { false, name });
		Entity entity(*this, id);
		entities.insert(entity);
		if (function) {
			disable_system_checks = true;
			function(entity);
			disable_system_checks = false;
		}
		trace("created ", entity, " with ", entity.get_number_of_components(), " components");
		activate_entity(entity);
		return entity;
	}

	Entities World::create_entities(const uint amount, const String& name, const function<void(Entity)>& function) {
		entity_ids.require(amount);
		Entities created_entities;
		for (uint i = 0; i < amount; ++i) {
			Entity entity = name.empty() ? create_entity("", function) : create_entity(name + to_string(i), function);
			created_entities.insert(entity);
		}
		return created_entities;
	}

	void World::destroy_entity(Entity& entity) {
		runtime_assert(is_existing(entity), "there is no existing entity with id #", entity.id, " can't destroy");
		trace("destroying ", entity, " in ", *this);
		deactivate_entity(entity);
		entity.remove_all_components();
		entities.erase(entity);
		entity_ids.release(entity.id);
		attributes.erase(entity.id);
		components.erase(entity.id);
	}

	void World::destroy_entity(const Entity::Id & id) {
		Entity entity = get_entity(id);
		destroy_entity(entity);
	}

	void World::destroy_entities(const Entities& entities) {
		for (auto entity : entities) {
			if (is_existing(entity)) destroy_entity(entity);
		}
	}

	Entity World::get_entity(const Entity::Id id) const {
		return Entity(const_cast<World&>(*this), id);
	}

	Entity World::find_entity(const String& name) const {
		return find_entity([&name](const Attributes& attributes) { return name == attributes.name; });
	}

	Entities World::find_entities(const String& name) const {
		return find_entities([&name](const Attributes& attributes) { return name == attributes.name; });
	}

	Entities World::find_entities_beginning(const String& name) const {
		return find_entities([&name](const Attributes& attributes) { return begins_with(name, attributes.name); });
	}

	Entities World::find_entities_ending(const String& name) const {
		return find_entities([&name](const Attributes& attributes) { return ends_with(name, attributes.name); });
	}

	Entity World::find_entity_tagged(const String& tag) const {
		return find_entity([&tag](const Attributes& attributes) { return tag == attributes.tag; });
	}

	Entities World::find_entities_tagged(const String& tag) const {
		return find_entities([&tag](const Attributes& attributes) { return tag == attributes.tag; });
	}

	Entities World::find_entities_tagged_beginning(const String& tag) const {
		return find_entities([&tag](const Attributes& attributes) { return begins_with(tag, attributes.tag); });
	}

	Entities World::find_entities_tagged_ending(const String& tag) const {
		return find_entities([&tag](const Attributes& attributes) { return ends_with(tag, attributes.tag); });
	}

	Entity World::find_entity(const std::function<bool(const Attributes&)>& accepts) const {
		for (auto& entry : attributes) {
			if (accepts(entry.second)) {
				auto id = entry.first;
				return get_entity(id);
			}
		}
		return get_entity(IDs::no_id);
	}

	Entities World::find_entities(const std::function<bool(const Attributes&)>& accepts) const {
		Entities entities;
		for (auto& entry : attributes) {
			if (accepts(entry.second)) {
				auto id = entry.first;
				entities.insert(get_entity(id));
			}
		}
		return entities;
	}

	void World::activate_entity(Entity& entity) {
		runtime_assert(is_existing(entity), "there is no existing entity with id #", entity.id, " can't activate");
		bool& active = attributes[entity.id].active;
		if (not active) {
			trace("activating ", entity, " in ", *this);
			active = true;
			update_systems(entity);
		}
	}

	void World::activate_entity(const Entity::Id & id) {
		Entity entity = get_entity(id);
		activate_entity(entity);
	}

	void World::deactivate_entity(Entity& entity) {
		runtime_assert(is_existing(entity), "there is no existing entity with id #", entity.id, " can't deactivate");
		bool& active = attributes[entity.id].active;
		if (active) {
			trace("deactivating ", entity, " in ", *this);
			active = false;
			update_systems(entity);
		}
	}

	void World::deactivate_entity(const Entity::Id & id) {
		Entity entity = get_entity(id);
		deactivate_entity(entity);
	}

	bool World::is_active(const Entity& entity) const {
		return is_existing(entity) && attributes.at(entity.id).active;
	}

	bool World::is_active(const Entity::Id & id) const {
		return is_existing(id) && attributes.at(id).active;
	}

	bool World::is_existing(const Entity& entity) const {
		return is_existing(entity.id);
	}

	bool World::is_existing(const Entity::Id id) const {
		return entity_ids.exists(id);
	}

	uint World::get_number_of_entities() const {
		return entities.size();
	}

	const Entities World::get_entities() const {
		return entities;
	}

	void World::remove_all_entities() {
		trace("removing all entities from ", *this);
		auto iterator = entities.begin();
		uint n = 0;
		while (iterator != entities.end()) {
			auto entity = *iterator++;
			destroy_entity(entity);
			n++;
		}
		trace("removed ", n, " entities from ", *this);
	}

	uint World::get_number_of_systems() const {
		return systems.size();
	}

	const Systems World::get_systems() const {
		Systems list(get_number_of_systems());
		for (auto& entry : systems) {
			System* system = entry.second.get();
			list.push_back(system);
		}
		return list;
	}

	void World::remove_all_systems() {
		trace("removing all systems from ", *this);
		auto iterator = systems.begin();
		uint n = 0;
		while (iterator != systems.end()) {
			Type system_type = (iterator++)->first;
			remove(system_type);
			n++;
		}
		trace("removed ", n, " systems from ", *this);
	}

	ostream& operator<<(ostream& output, const World& world) {
		output << "\"" << world.name << "\"";
		return output;
	}

	/// template implementation details

	void World::add(Type system_type, System*const system) {
		trace("adding ", system_type, " (", system->filter, ") to ", *this);
		system->world.pointer = this;
		priorities[system->priority].push_back(system);
		systems.emplace(system_type, system);
		system->initialize();
		update_system(*system);
		trace("added ", system->get_number_of_entities(), " entities to ", system_type);
		system->activate();
	}

	void World::remove(Type system_type) {
		trace("removing ", system_type, " from ", *this);
		auto iterator = systems.find(system_type);
		unique_ptr<System>& system = iterator->second;
		system->deactivate();
		auto number_of_entities = system->get_number_of_entities();
		system->remove_all_entities();
		trace("removed ", number_of_entities, " entities from ", system_type);
		system->terminate();
		Systems& list = priorities[system->priority];
		list.erase(find(list.begin(), list.end(), system.get()));
		systems.erase(iterator);
	}

	bool World::has(Type system_type) const {
		return (systems.find(system_type) != systems.end());
	}

	System& World::get(Type system_type) const {
		return *systems.at(system_type);
	}

}
