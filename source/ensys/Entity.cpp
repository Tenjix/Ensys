#include "Entity.h"

#include <ensys/World.h>

#include <utilities/Logging.h>

namespace tenjix {

	namespace ensys {

		Entity::Entity(World& world, Entity::Id id) : id(id), world(world) {
			name.owner = this;
			tag.owner = this;
			is_active.owner = this;
			is_existing.owner = this;
		}

		Entity::Entity(const Entity& entity) : Entity(entity.world, entity.id) {}
		Entity::Entity(Entity&& entity) : Entity(entity.world, entity.id) {}

		Entity& Entity::activate() {
			world.activate_entity(*this);
			return *this;
		}

		Entity& Entity::deactivate() {
			world.deactivate_entity(*this);
			return *this;
		}

		void Entity::destroy() {
			world.destroy_entity(*this);
		}

		uint Entity::get_number_of_components() const {
			runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't determine number of components");
			auto& components = world.components[id];
			return components.size();
		}

		const Components Entity::get_components() const {
			runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't retrieve components");
			Components components;
			for (auto& pair : world.components[id]) {
				components.push_back(pair.second);
			}
			return components;
		}

		void Entity::remove_all_components() {
			runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't remove components");
			trace("removing all components from ", *this);
			auto& components = world.components[id];
			auto iterator = components.begin();
			uint n = 0;
			while (iterator != components.end()) {
				Type component_type = (iterator++)->first;
				remove(component_type);
				n++;
			}
			trace("removed ", n, " components from ", *this);
		}

		const Types Entity::get_component_types() const {
			runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't determine component types");
			Types types;
			for (auto& pair : world.components[id]) {
				types.insert(pair.first);
			}
			return types;
		}

		bool Entity::operator==(const Entity &entity) const {
			return id == entity.id && &world == &entity.world;
		}

		bool Entity::operator!=(const Entity &entity) const {
			return not operator==(entity);
		}

		std::ostream& operator<<(std::ostream& output, const Entity& entity) {
			output << "Entity #" << entity.id;
			String name = entity.name;
			if (not name.empty()) output << " \"" << name << "\"";
			return output;
		}

		/// properties

		const String& Entity::get_name() const {
			runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't get name");
			return world.attributes[id].name;
		}

		void Entity::set_name(Assignment<String> assignment) {
			runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't set name");
			assignment.to(world.attributes[id].name);
		}

		const String& Entity::get_tag() const {
			runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't get tag");
			return world.attributes[id].tag;
		}

		void Entity::set_tag(Assignment<String> assignment) {
			runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't set tag");
			assignment.to(world.attributes[id].tag);
		}

		bool Entity::get_is_active() const {
			return world.is_active(id);
		}

		bool Entity::get_is_existing() const {
			return world.is_existing(id);
		}

		/// template implementation details

		void Entity::add(Type component_type, const shared<Component>& component) {
			trace("adding ", component_type, " to ", *this);
			world.components[id].emplace(component_type, component);
			world.update_systems(*this);
		}

		void Entity::remove(Type component_type) {
			trace("removing ", component_type, " from ", *this);
			auto& components = world.components[id];
			auto iterator = components.find(component_type);
			runtime_assert(iterator != components.end(), *this, " doesn't have a component of type ", component_type, ", can't remove it");
			components.erase(iterator);
			world.update_systems(*this);
		}

		bool Entity::has(Type component_type) const {
			auto& components = world.components[id];
			auto iterator = components.find(component_type);
			return iterator != components.end();
		}

		shared<Component> Entity::get(Type component_type) const {
			auto& components = world.components[id];
			auto iterator = components.find(component_type);
			if (iterator == components.end()) return shared<Component>();
			return iterator->second;
		}

	}

}
