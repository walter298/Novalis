#include "EventHandler.h"

#include "detail/reflection/ClassIteration.h"

void nv::EventHandler::operator()() {
	//cancelled all events queued to be cancelled
	detail::forEachDataMember([this]<typename ID>(std::vector<ID>& ids) {
		auto& events = getEvents<ID>();
		for (const auto& id : ids) {
			events.erase(id);
		}
		ids.clear();
		return detail::STAY_IN_LOOP;
	}, m_cancelledEventQueues);

	auto& events = std::get<Events<void>>(m_events);
	for (auto& [id, event] : events) {
		event();
	}
	auto& cancellableEvents = std::get<Events<bool>>(m_events);
	for (auto& [id, event] : cancellableEvents) {
		if (event()) {
			std::get<std::vector<CancellableEventID>>(m_cancelledEventQueues).push_back(id);
		}
	}
}