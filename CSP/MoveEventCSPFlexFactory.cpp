#include <CSP/MoveEventCSPFlexFactory.hpp>
#include <CSP/Policies/FlexDisplacementPolicy.hpp>
#include <Scenario/Commands/Scenario/Displacement/MoveEvent.hpp>

namespace CSP
{
Scenario::Command::SerializableMoveEvent* MoveEventCSPFlexFactory::make(
        Path<Scenario::ScenarioModel> &&scenarioPath,
        const Id<Scenario::EventModel> &eventId,
        const TimeValue &newDate,
        ExpandMode mode)
{
    return new Scenario::Command::MoveEvent<FlexDisplacementPolicy>(std::move(scenarioPath), eventId, newDate, mode);
}

Scenario::Command::SerializableMoveEvent* MoveEventCSPFlexFactory::make()
{
    return new Scenario::Command::MoveEvent<FlexDisplacementPolicy>();
}
}
