#pragma once

#include <kiwi/kiwi.h>
#include <QVector>
#include <iscore/tools/SettableIdentifier.hpp>
#include <Process/TimeValue.hpp>
#include <CSP/Model/tools/CSPConstraintHolder.hpp>

#include <CSP/Model/CSPScenario.hpp>


namespace Scenario
{
class ConstraintModel;
}

namespace CSP
{
class CSPDisplacementPolicy;
class CSPTimeRelation : public CSPConstraintHolder, public Nano::Observer
{
    friend class CSPDisplacementPolicy;
    friend class CSPFlexDisplacementPolicy;

public:
    CSPTimeRelation(CSPScenario& scenario, const Id<Scenario::ConstraintModel>& constraintId);

    CSPTimeRelation() = default;

    ~CSPTimeRelation();

    kiwi::Variable& getMin();

    kiwi::Variable& getMax();

    /**
     * @brief minChanged
     * call this function to check if csp min differ from iscore min
     * @return
     */
    bool minChanged() const;

    /**
     * @brief maxChanged
     * call this function to check if csp max differ from iscore max
     * @return
     */
    bool maxChanged() const;

private:
    kiwi::Variable m_min{"min"};
    TimeValue m_iscoreMin{};

    kiwi::Variable m_max{"max"};
    TimeValue m_iscoreMax{};

    kiwi::Constraint m_cstrRigidity{kiwi::Constraint(m_min == m_max)};// TODO ask JM if it is safe to do so

    //void onDefaultDurationChanged(const TimeValue& arg);
    void onMinDurationChanged(const TimeValue& min);
    void onMaxDurationChanged(const TimeValue& max);

    void onProcessCreated(const Process::ProcessModel& process);
    void onProcessRemoved(const Process::ProcessModel& process);

    QHash<Id<Process::ProcessModel>, CSPScenario*> m_subScenarios;
};
}
