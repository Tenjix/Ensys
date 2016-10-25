#pragma once

#include <unordered_map>

#include <ensys/System.h>

namespace ensys {

	class ObservableComponent : public Component {

		//std::unordered_map<System*, std::vector<Entity>> observers;
		std::unordered_map<System*, Entities> observers;

	public:

		void attach(System* observer, const Entity& entity) {
			observers[observer].insert(entity);
			//auto& entities = observers[observer];
			//auto iterator = std::find(entities.begin(), entities.end(), entity);
			//if (iterator != entities.end()) return;
			//entities.push_back(entity);
		}

		void detach(System* observer, const Entity& entity) {
			observers[observer].erase(entity);
			//auto& entities = observers[observer];
			//auto iterator = std::find(entities.begin(), entities.end(), entity);
			//if (iterator == entities.end()) return;
			//entities.erase(iterator);
		}

		void detach(System* observer) {
			observers.erase(observer);
		}

	protected:

		void notify() {
			for (auto& entry : observers) {
				auto system = entry.first;
				auto& entities = entry.second;
				for (auto& entity : entities) {
					if (entity.is_active) system->on_entity_modified(entity);
				}
			}
		}

	};

}