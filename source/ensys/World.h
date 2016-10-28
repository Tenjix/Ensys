#pragma once

#include <functional>
#include <set>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include <ensys/Entity.h>
#include <ensys/Component.h>
#include <ensys/System.h>
#include <ensys/Attributes.h>
#include <ensys/IDs.h>

#include <utilities/Assertions.h>
#include <utilities/Types.h>

namespace ensys {

	class World final {

		friend Entity;

		using MappedAttributes = std::unordered_map<Entity::Id, Attributes>;
		using MappedComponents = std::unordered_map<Entity::Id, std::unordered_map<Type, shared<Component>>>;
		using MappedPriorities = std::map<System::Priority, std::vector<System*>, std::greater<System::Priority>>;
		using MappedSystems = std::unordered_map<Type, std::unique_ptr<System>>;

		MappedAttributes attributes;
		MappedComponents components;
		MappedPriorities priorities;
		MappedSystems systems;

		Entities entities;

		IDs entity_ids;

		const String name;

		bool disable_system_checks = false;

	public:

		explicit World(String name = "World", uint initial_entity_pool_size = 1000);

		World(const World&) = delete;
		World(World&&) = delete;

		World& operator=(const World&) = delete;
		World& operator=(World&&) = delete;

		// updates the world
		void update(float delta_time);

		// clears the world by removing all systems and entities
		void clear();

		// creates and activates a new entity (accepts a function to execute before the entity gets activated)
		Entity create_entity(const String& name = "", const std::function<void(Entity)>& function = nullptr);
		// creates and activates multiple new entities (accepts a function to execute on each entity before it gets activated)
		Entities create_entities(const uint amount, const String& name = "", const std::function<void(Entity)>& function = nullptr);
		// creates and activates a new entity with the given components (accepts a function to execute before the entity gets activated)
		template <class... Components>
		Entity create_entity_with(const String& name = "", const std::function<void(Entity)>& function = nullptr);
		// creates and activates multiple new entities with given components (accepts a function to execute on each entity before it gets activated)
		template <class... Components>
		Entities create_entities_with(const uint number_of_entities, const String& name = "", const std::function<void(Entity)>& function = nullptr);
		//template <class... Components>
		//Entity create_entity_with_shared(const String& name = "", const std::function<void(Entity)>& function = nullptr);
		//template <class... Components>
		//Entities create_entities_with_shared(const uint number_of_entities, const String& name = "", const std::function<void(Entity)>& function = nullptr);

		// destroys an entity with its components
		void destroy_entity(Entity& entity);
		// destroys an entity with its components
		void destroy_entity(const Entity::Id& id);
		// destroys multiple entities at once
		void destroy_entities(const Entities& entities);

		// returns the entity with the given id
		Entity get_entity(const Entity::Id id) const;

		// finds and returns the first entity with the given name
		Entity find_entity(const String& name) const;
		// finds and returns all entities with the given name
		Entities find_entities(const String& name) const;
		// finds and returns all entities whose name starts with the given String
		Entities find_entities_beginning(const String& name) const;
		// finds and returns all entities whose name ends with the given String
		Entities find_entities_ending(const String& name) const;

		// finds and returns the first entity with the given tag
		Entity find_entity_tagged(const String& tag) const;
		// finds and returns all entities with the given tag
		Entities find_entities_tagged(const String& tag) const;
		// finds and returns all entities whose tag starts with the given String
		Entities find_entities_tagged_beginning(const String& tag) const;
		// finds and returns all entities whose tag ends with the given String
		Entities find_entities_tagged_ending(const String& tag) const;

		// activates an entity, including it in system updates
		void activate_entity(Entity& entity);
		// activates an entity, including it in system updates
		void activate_entity(const Entity::Id& id);

		// deactivates an entity, excluding it from system updates
		void deactivate_entity(Entity& entity);
		// deactivates an entity, excluding it from system updates
		void deactivate_entity(const Entity::Id& id);

		// checks whether a given entity is existing and active
		bool is_active(const Entity& entity) const;
		// checks whether a given entity is existing and active
		bool is_active(const Entity::Id& id) const;

		// checks whether a given entity is existing
		bool is_existing(const Entity& entity) const;
		// checks whether a entity with the given id is existing
		bool is_existing(const Entity::Id id) const;

		// returns the number of entities (including deactivated ones)
		uint get_number_of_entities() const;

		// returns all entities of this world
		const Entities get_entities() const;

		// removes all entities from this world
		void remove_all_entities();

		template <class SystemType, typename... Arguments>
		SystemType& add(Arguments&&... arguments);

		template <typename SystemType>
		void remove();

		template <class SystemType>
		bool has() const;

		template <class SystemType>
		SystemType& get() const;

		template <class SystemType>
		void activate();

		template <class SystemType>
		void deactivate();

		// returns the number of systems within the world
		uint get_number_of_systems() const;

		// returns all systems of this world
		const Systems get_systems() const;

		// removes all systems from this world
		void remove_all_systems();

		friend std::ostream& operator<<(std::ostream& output, const World& world);

	private:

		void update_systems(const Entity& entity);
		void update_system(System& system);

		Entity find_entity(const std::function<bool(const Attributes&)>& accepts) const;
		Entities find_entities(const std::function<bool(const Attributes&)>& accepts) const;

		/// template implementation details
		void add(Type system_type, System*const system);
		void remove(Type system_type);
		bool has(Type system_type) const;
		System& get(Type system_type) const;

	};

	// creates and activates a new entity with given components
	template <class... Components>
	Entity World::create_entity_with(const String& name, const std::function<void(Entity)>& function) {
		Entity entity = create_entity(name, [&function](Entity entity) {
			for_each_variadic(entity.add<Components>());
			if (function) function(entity);
		});
		return entity;
	}

	// creates and activates multiple new entities with given components and executes a given function on each entity
	template <class... Components>
	Entities World::create_entities_with(const uint number_of_entities, const String& name, const std::function<void(Entity)>& function) {
		return create_entities(number_of_entities, name, [&function](Entity entity) {
			for_each_variadic(entity.add<Components>());
			if (function) function(entity);
		});
	}

	// adds a system of the given type to this world, constructed with the given arguments
	template <class SystemType, typename... Arguments>
	SystemType& World::add(Arguments&&... arguments) {
		static_assert(std::is_base_of<System, SystemType>(), "given type is not a system, can't add it to a world");
		Type type = typeid(SystemType);
		runtime_assert(not has(type), "a system of type ", type, " already exists in this world, can't add another");
		SystemType* system = new SystemType(std::forward<Arguments>(arguments)...);
		add(type, system);
		return *system;
	}

	// removes the system of the given type from this world
	template <class SystemType>
	void World::remove() {
		static_assert(std::is_base_of<System, SystemType>(), "given type is not a system, can't remove it from world");
		Type type = typeid(SystemType);
		runtime_assert(has(type), "a system of type ", type, " doesn't exist in this world, can't remove it");
		remove(type);
	}

	// checks whether this world has a system of the given type
	template <class SystemType>
	bool World::has() const {
		static_assert(std::is_base_of<System, SystemType>(), "given type is not a system, can't determine its existence");
		return has(typeid(SystemType));
	}

	// returns the system of the given type owned by this world
	template <class SystemType>
	SystemType& World::get() const {
		static_assert(std::is_base_of<System, SystemType>(), "given type is not a system, can't retrieve it from world");
		Type type = typeid(SystemType);
		runtime_assert(has(type), "a system of type ", type, " doesn't exist in this world, can't retreive it");
		return static_cast<SystemType&>(get(type));
	}

	// activates a system, enabling updates
	template<class SystemType>
	inline void World::activate() {
		static_assert(std::is_base_of<System, SystemType>(), "given type is not a system, can't activate it");
		Type type = typeid(SystemType);
		runtime_assert(has(type), "a system of type ", type, " doesn't exist in this world, can't activate it");
		get(type).activate();
	}

	// deactivates a system, disabling updates
	template<class SystemType>
	inline void World::deactivate() {
		static_assert(std::is_base_of<System, SystemType>(), "given type is not a system, can't deactivate it");
		Type type = typeid(SystemType);
		runtime_assert(has(type), "a system of type ", type, " doesn't exist in this world, can't activate it");
		get(type).deactivate();
	}

}

#ifndef ENSYS_NO_NAMESPACE_ALIAS
namespace es = ensys;
#endif
