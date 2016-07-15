#pragma once

#include <iostream>
#include <unordered_set>

#include <ensys/Entity.h>

#include <utilities/Properties.h>
#include <utilities/Types.h>
#include <utilities/TypeFilter.h>

namespace ensys {

	class System {

		friend class World;
		friend class ObservableComponent;

		bool active;
		bool get_is_initialized() const;
		bool get_is_active() const;

	public:

		using Priority = unsigned char;

		// the systems priority (systems with higher priority get updated first, but the order of systems with same priority is undefined)
		const Priority priority;

		// constructs a new system with the given priority (systems with higher priority get updated first, but the order of systems with same priority is undefined)
		explicit System(Priority priority = 0);

		System(const System&) = delete;
		System(System&&) = delete;

		System& operator=(const System&) = delete;
		System& operator=(System&&) = delete;

		virtual ~System() noexcept;

		// the world this system exists in
		ReadonlyPointerProperty<World, World> world;

		ReadonlyByValueProperty<bool, System, &System::get_is_initialized> is_initialized;
		ReadonlyByValueProperty<bool, System, &System::get_is_active> is_active;

		// activates this entity, including it in system updates
		void activate();

		// deactivates this entity, excluding it from system updates
		void deactivate();

		// returns the world this system blongs to
		//const World& get_world() const;
		// returns the of entities controlled by this system
		const Entities& get_entities() const;
		// returns the systems component type filter
		const TypeFilter& get_filter() const;

		// returns the number of entities in this system
		uint get_number_of_entities() const;

		friend std::ostream& operator<<(std::ostream& output, const System& system);

	protected:

		// the systems component type filter
		TypeFilter filter;

		// updates the system
		// invoked by the world.update(delta time)
		// default implementation invokes system.update(entity, delta_time) for each entity in the system
		virtual void update(float delta_time);

	private:

		Entities suitable_entities;

		// checks whether the given entity should be added to or removed from the system and acts accordingly
		void check(const Entity& entity);

		// adds an entity to the system and invokes on_entity_added
		void add(const Entity& entity);
		// removes an entity from the system and invokes on_entity_added
		void remove(const Entity& entity);

		// removes all entities from the system and invokes on_entity_removed accordingly
		void remove_all_entities();

		// initializes the system
		// invoked after the system has been added to a world
		virtual void initialize() {}
		// updates an entity in the system
		// invoked for each entity be the default implementation of system.update(delta_time)
		virtual void update(Entity& entity, float delta_time) {};
		// clears the system
		// invoked before the system is removed from a world
		virtual void terminate() {}

		virtual void on_entity_added(const Entity& entity) {}
		virtual void on_entity_removed(const Entity& entity) {}
		virtual void on_entity_modified(const Entity& entity) {}

	};

	using Systems = std::vector<System*>;

}
