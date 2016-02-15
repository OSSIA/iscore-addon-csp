#include "ExecutionPolicyFactory.hpp"

#include <CSP/Policies/ExecutionPolicy.hpp>

namespace CSP {

CSPCoherencyCheckerInterface*ExecutionPolicyFactory::make(Scenario::ScenarioModel& scenario,
                                                          const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements)
{
    return new CSP::ExecutionPolicy{scenario, positionnedElements};
}

}