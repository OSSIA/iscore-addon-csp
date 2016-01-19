#include "TimeRelation.hpp"
#include "Scenario.hpp"
#include <Scenario/Process/ScenarioInterface.hpp>
#include <Scenario/Process/ScenarioModel.hpp>
#include <Scenario/Document/Constraint/ConstraintModel.hpp>
#include <CSP/Model/TimeNode.hpp>
#include <kiwi/kiwi.h>
#include <Scenario/Process/Algorithms/Accessors.hpp>

namespace CSP
{
TimeRelationModel::TimeRelationModel(
        ScenarioModel& cspScenario,
        const Id<Scenario::ConstraintModel>& constraintId):
    ConstraintHolder::ConstraintHolder(cspScenario.getSolver(), &cspScenario)
{
    this->setParent(&cspScenario);
    this->setObjectName("CSPTimeRelation");

    auto& scenario = *cspScenario.getScenario();
    auto& constraint = scenario.constraint(constraintId);

    m_iscoreMin = constraint.duration.minDuration();
    m_iscoreMax = constraint.duration.maxDuration();

    m_min.setValue(m_iscoreMin.msec());
    m_max.setValue(m_iscoreMax.msec());

    // weight
    //solver.addEditVariable(m_min, kiwi::strength::strong);
    //solver.addEditVariable(m_max, kiwi::strength::medium);

    auto& prevTimenodeModel = startTimeNode(constraint, scenario);
    auto& nextTimenodeModel = endTimeNode(constraint, scenario);

    //retrieve/create prev and next timenode
    auto prevCSPTimenode = cspScenario.insertTimenode(prevTimenodeModel.id());
    auto nextCSPTimenode = cspScenario.insertTimenode(nextTimenodeModel.id());

    // apply model constraints
    // 1 - min >= 0
    auto cMinSupZero = new kiwi::Constraint({m_min >= 0});\
    putConstraintInSolver(cMinSupZero);

    // 2 - min inferior to max
    auto cMinInfMax = new kiwi::Constraint({m_min <= m_max});\
    putConstraintInSolver(cMinInfMax);

    // 3 - date of end timenode inside min and max
    auto cNextDateMin = new kiwi::Constraint({nextCSPTimenode->getDate() >= (prevCSPTimenode->getDate() + m_min)});\
    putConstraintInSolver(cNextDateMin);

    auto cNextDateMax = new kiwi::Constraint({nextCSPTimenode->getDate() <= (prevCSPTimenode->getDate() + m_max)});\
    putConstraintInSolver(cNextDateMax);


    // if there are sub scenarios, store them
    for(auto& process : constraint.processes)
    {
        if(auto* scenar = dynamic_cast<Scenario::ScenarioModel*>(&process))
        {
            m_subScenarios.insert(scenar->id(), new ScenarioModel(*scenar, scenar));
        }
    }

    // watch over durations edits
    con(constraint.duration, &Scenario::ConstraintDurations::minDurationChanged,
        this, &TimeRelationModel::onMinDurationChanged);
    con(constraint.duration, &Scenario::ConstraintDurations::maxDurationChanged,
        this, &TimeRelationModel::onMaxDurationChanged);

    // watch over potential subscenarios
    constraint.processes.added.connect<TimeRelationModel, &TimeRelationModel::onProcessCreated>(this);
    constraint.processes.removed.connect<TimeRelationModel, &TimeRelationModel::onProcessRemoved>(this);
}

TimeRelationModel::~TimeRelationModel()
{
}

kiwi::Variable& TimeRelationModel::getMin()
{
    return m_min;
}

kiwi::Variable& TimeRelationModel::getMax()
{
    return m_max;
}

bool TimeRelationModel::minChanged() const
{
    return m_min.value() != m_iscoreMin.msec();
}

bool TimeRelationModel::maxChanged() const
{
    return m_max.value() != m_iscoreMax.msec();
}

void TimeRelationModel::onMinDurationChanged(const TimeValue& min)
{
    m_min.setValue(min.msec());
}

void TimeRelationModel::onMaxDurationChanged(const TimeValue& max)
{
    if(max.isInfinite())
    {
        //TODO : ??? remove constraints on max?
    }else
    {
        m_max.setValue(max.msec());
    }
}

void TimeRelationModel::onProcessCreated(const Process::ProcessModel& process)
{
    if(auto scenario = dynamic_cast<const Scenario::ScenarioModel*>(&process))
    {
        m_subScenarios.insert(scenario->id(), new ScenarioModel(*scenario, const_cast<Scenario::ScenarioModel*>(scenario)));
    }
}

void TimeRelationModel::onProcessRemoved(const Process::ProcessModel& process)
{
    if(auto scenario = dynamic_cast<const Scenario::ScenarioModel*>(&process))
    {
        delete(m_subScenarios.take(scenario->id()));
    }
}
}