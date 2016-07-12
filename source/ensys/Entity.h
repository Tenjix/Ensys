#pragma once

#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>

#include <ensys/Component.h>

#include <utilities/Assertions.h>
#include <utilities/Optional.h>
#include <utilities/Properties.h>
#include <utilities/Types.h>

namespace ensys {

	// represents an entity
	class Entity final {

		friend class World;

		String& get_name() const;
		void set_name(Assignment<String>);

		String& get_tag() const;
		void set_tag(Assignment<String>);

		bool get_is_active() const;
		bool get_is_existing() const;

	public:

		using Id = uint;
		using Ids = std::unordered_set<Id>;

		// the entities unique id
		const Id id;
		// the world this entity lives in
		World& world;

		Entity(const Entity&);
		Entity(Entity&&);

		Entity& operator=(const Entity&) = delete;
		Entity& operator=(Entity&&) = delete;

		// the entities name
		ByReferenceProperty<String, Entity, &Entity::get_name, &Entity::set_name> name;
		// the entities tag
		ByReferenceProperty<String, Entity, &Entity::get_tag, &Entity::set_tag> tag;
		// checks whether this entity is existing and active
		ReadonlyByValueProperty<bool, Entity, &Entity::get_is_active> is_active;
		// checks whether this entity is existing
		ReadonlyByValueProperty<bool, Entity, &Entity::get_is_existing> is_existing;

		// activates this entity, including it in system updates
		void activate();

		// deactivates this entity, excluding it from system updates
		void deactivate();

		// destroys this entity with all its components
		void destroy();

		template <class... ComponentTypes>
		void add_components();

		template <class... ComponentTypes>
		void add_shared_components(const Entity& other);

		template <class ComponentType>
		ComponentType& add(const shared<ComponentType>& component);

		template <class ComponentType, typename... Arguments>
		ComponentType& add(Arguments&&... args);

		template <class ComponentType>
		ComponentType& add_shared(const Entity& other);

		template <class... ComponentTypes>
		void remove_components();

		template <class ComponentType>
		void remove();

		template <class ComponentType>
		ComponentType& get() const;

		template <class ComponentType>
		Optional<ComponentType> has() const;

		template <class ComponentType>
		bool shares() const;

		const Types get_component_types() const;

		// returns the number of components owned by this entity
		uint get_number_of_components() const;

		// returns a collection of all components owned by this entity
		const Components get_components() const;

		// removes all components owned by this entity
		void remove_all_components();

		bool operator==(const Entity& other) const;
		bool operator!=(const Entity& other) const;

		friend std::ostream& operator<<(std::ostream& output, const Entity& entity);

	private:

		Entity(World& world, Id id);

		/// template implementation details
		void add(Type component_type, const shared<Component>& component);
		void remove(Type component_type);
		const shared<Component>& has(Type component_type) const;
		Component& get(Type component_type) const;

	};

	using Entities = std::unordered_set<Entity>;

	// adds several components based on the given types to this entity
	template <class... ComponentTypes>
	void Entity::add_components() {
		for_each_variadic(add<ComponentTypes>());
	}

	// adds several shared components based on the given types to this entity, shared by the given entity
	template <class... ComponentTypes>
	void Entity::add_shared_components(const Entity& other_entity) {
		for_each_variadic(add_shared<ComponentTypes>(other_entity));
	}

	// adds the given component to this entity
	template <class ComponentType>
	ComponentType& Entity::add(const shared<ComponentType>& component) {
		static_assert(std::is_base_of<Component, ComponentType>(), "given type is not a component, can't add it to entity");
		runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't add components");
		Type type = typeid(ComponentType);
		runtime_assert(not has(type), *this, " already contains a component of type ", type, ", can't add another");
		add(type, component);
		return *component;
	}

	// adds a component of the given type to this entity, constructed with the given arguments
	template <class ComponentType, typename... Arguments>
	ComponentType& Entity::add(Arguments&&... arguments) {
		static_assert(std::is_base_of<Component, ComponentType>(), "given type is not a component, can't add it to entity");
		runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't add components");
		Type type = typeid(ComponentType);
		runtime_assert(not has(type), *this, " already contains a component of type ", type, ", can't add another");
		shared<ComponentType> component = std::make_shared<ComponentType>(std::forward<Arguments>(arguments)...);
		add(type, component);
		return *component;
	}

	// adds a shared component of the given type to this entity, shared by the given entity
	template <class ComponentType>
	ComponentType& Entity::add_shared(const Entity& other_entity) {
		static_assert(std::is_base_of<Component, ComponentType>(), "given type is not a component, can't add it to entity");
		runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't add shared components");
		runtime_assert(other_entity.is_existing(), "there is no existing entity with id #", other_entity.id, " to share components");
		Type type = typeid(ComponentType);
		runtime_assert(not has(type), *this, " already contains a component of type ", type, ", can't add another");
		shared<ComponentType> component = std::static_pointer_cast<ComponentType>(other_entity.has(type));
		runtime_assert(component, other_entity, " doesn't have a component of type ", type, " to share with ", *this);
		add(type, component);
		return *component;
	}

	// removes several components from this entity, based on the given types
	template <class... ComponentTypes>
	void Entity::remove_components() {
		for_each_variadic(remove<ComponentTypes>());
	}

	// removes the component of the given type from this entity
	template <class ComponentType>
	void Entity::remove() {
		static_assert(std::is_base_of<Component, ComponentType>(), "given type is not a component, can't remove it from entity");
		runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't remove components");
		remove(typeid(ComponentType));
	}

	// checks whether this entity has a component of the given type
	template <class ComponentType>
	Optional<ComponentType> Entity::has() const {
		static_assert(std::is_base_of<Component, ComponentType>(), "given type is not a component, can't determine if entity has it");
		runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't determine components");
		return std::static_pointer_cast<ComponentType>(has(typeid(ComponentType)));
	}

	// checks whether this entity shares a component of the given type with other entities
	template <class ComponentType>
	bool Entity::shares() const {
		static_assert(std::is_base_of<Component, ComponentType>(), "given type is not a component, can't determine if entity has it");
		runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't determine shared components");
		const shared<Component>& component = has(typeid(ComponentType));
		return component and not component.unique();
	}

	// returns the component of the given type owned by this entity
	template <class ComponentType>
	ComponentType& Entity::get() const {
		static_assert(std::is_base_of<Component, ComponentType>(), "given type is not a component, can't retrieve it from entity");
		runtime_assert(is_existing(), "there is no existing entity with id #", id, " can't retreive components");
		return static_cast<ComponentType&>(get(typeid(ComponentType)));
	}

}

namespace std {

	template<>
	struct hash<es::Entity> {
		size_t operator()(const es::Entity& entity) const noexcept {
			size_t hash_prime = 92821;
			return hash_prime + hash<uint>()(entity.id);
		}
	};

}