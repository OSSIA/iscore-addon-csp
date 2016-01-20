#include <CSP/FlexDisplacementPolicy.hpp>
#include <CSP/Model/Scenario.hpp>
#include <CSP/Model/TimeNode.hpp>
#include <CSP/Model/TimeRelation.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>
#define STAY_MINMAXFROMDATEONCREATION_STRENGTH kiwi::strength::required
#define STAY_MINMAXPREVTIMERELATION_STRENGTH kiwi::strength::strong
#define STAY_MINMAX_STRENGTH kiwi::strength::required
#define STAY_TNODE_STRENGTH kiwi::strength::medium
#define STAY_DRAGGED_TNODE_STRENGTH kiwi::strength::strong + 1.0 //not so sure that its working

namespace CSP
{
FlexDisplacementPolicy::FlexDisplacementPolicy(
        Scenario::ScenarioModel& scenario,
        const QVector<Id<Scenario::TimeNodeModel> >& draggedElements)
{
    if(ScenarioModel* cspScenario = scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        // add stays to all elements
        refreshStays(*cspScenario, draggedElements);

    }else
    {
        ISCORE_BREAKPOINT;
        throw std::runtime_error("No CSP scenario found for this model");
    }
}

void FlexDisplacementPolicy::computeDisplacement(
        Scenario::ScenarioModel& scenario,
        const QVector<Id<Scenario::TimeNodeModel>>& draggedElements,
        const TimeValue& deltaTime,
        Scenario::ElementsProperties& elementsProperties)
{
    // get the csp scenario

    if(ScenarioModel* cspScenario = scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        auto& solver = cspScenario->getSolver();

        // get the corresponding CSP elements and start editing vars
        for(auto curDraggedTimeNodeId : draggedElements)
        {
            auto curDraggedCspTimeNode = cspScenario->m_timeNodes[curDraggedTimeNodeId];

            // get the initial value
            TimeValue initialDate;
            if(elementsProperties.timenodes.contains(curDraggedTimeNodeId))
            {
                initialDate = elementsProperties.timenodes[curDraggedTimeNodeId].oldDate;
            }else
            {
                initialDate = *(curDraggedCspTimeNode->m_iscoreDate);
            }

            //weight
            solver.addEditVariable(curDraggedCspTimeNode->m_date, STAY_DRAGGED_TNODE_STRENGTH);

            // suggest their new values
            auto newDate = initialDate.msec() + deltaTime.msec();

            try
            {
                solver.suggestValue(curDraggedCspTimeNode->getDate(), newDate);
            }catch(...)
            {
                qDebug("lalalalalala j'ai même pas planté");
            }
        }

        // solve system
        solver.updateVariables();

        // release edit vars
        for(auto curDraggedTimeNodeId : draggedElements)
        {
            solver.removeEditVariable(cspScenario->m_timeNodes[curDraggedTimeNodeId]->m_date);
        }

        // look for changes // TODO : maybe find a more efficient way of doing that

        // - in timenodes :
        QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario->m_timeNodes);
        while (timeNodeIterator.hasNext())
        {
            timeNodeIterator.next();

            auto curTimeNodeId = timeNodeIterator.key();
            auto curCspTimenode = timeNodeIterator.value();

            // if updated TODO : if updated => curCspTimenode->dateChanged()
            if(true)
            {
                // if timenode NOT already in element properties, create new element properties and set the old date
                if(! elementsProperties.timenodes.contains(curTimeNodeId))
                {
                    elementsProperties.timenodes[curTimeNodeId] = Scenario::TimenodeProperties{};
                    elementsProperties.timenodes[curTimeNodeId].oldDate = *(curCspTimenode->m_iscoreDate);
                }

                // put the new date
                elementsProperties.timenodes[curTimeNodeId].newDate = TimeValue::fromMsecs(curCspTimenode->m_date.value());
            }
        }

        // - in time relations :
        QHashIterator<Id<Scenario::ConstraintModel>, TimeRelationModel*> timeRelationIterator(cspScenario->m_timeRelations);
        while(timeRelationIterator.hasNext())
        {
            timeRelationIterator.next();

            const auto& curTimeRelationId = timeRelationIterator.key();
            auto& curCspTimerelation = timeRelationIterator.value();
            const auto& curConstraint = scenario.constraint(curTimeRelationId);

            // if osef TODO : if updated
            if(true)
            {
                // if timenode NOT already in element properties, create new element properties and set the old values
                if(! elementsProperties.constraints.contains(curTimeRelationId))
                {
                    elementsProperties.constraints[curTimeRelationId] = Scenario::ConstraintProperties{};
                    elementsProperties.constraints[curTimeRelationId].oldMin = curCspTimerelation->m_iscoreMin;
                    elementsProperties.constraints[curTimeRelationId].oldMax = curCspTimerelation->m_iscoreMax;

                    // Save the constraint display data START ----------------
                    QByteArray arr;
                    Visitor<Reader<DataStream>> jr{&arr};
                    jr.readFrom(curConstraint);

                    // Save for each view model of this constraint
                    // the identifier of the rack that was displayed
                    QMap<Id<Scenario::ConstraintViewModel>, Id<Scenario::RackModel>> map;
                    for(const Scenario::ConstraintViewModel* vm : curConstraint.viewModels())
                    {
                        map[vm->id()] = vm->shownRack();
                    }

                    elementsProperties.constraints[curTimeRelationId].savedDisplay = {{iscore::IDocument::path(curConstraint), arr}, map};
                    // Save the constraint display data END ----------------
                }

                // put the new values
                /*
                qDebug() << "----       min : " << curCspTimerelation->m_min.value();
                qDebug() << "    min iscore : " << curCspTimerelation->m_iscoreMin->msec();

                qDebug() << "----       max : " << curCspTimerelation->m_max.value();
                qDebug() << "    max iscore : " << curCspTimerelation->m_iscoreMax->msec();
                */

                elementsProperties.constraints[curTimeRelationId].newMin = TimeValue::fromMsecs(curCspTimerelation->m_min.value());
                elementsProperties.constraints[curTimeRelationId].newMax = TimeValue::fromMsecs(curCspTimerelation->m_max.value());
            }
        }
    }else
    {
        std::runtime_error("No CSP scenario found for this model");
    }
}

void FlexDisplacementPolicy::refreshStays(
        ScenarioModel& cspScenario,
        const QVector<Id<Scenario::TimeNodeModel> >& draggedElements)
{
    auto& scenario = *cspScenario.getScenario();
    // time relations stays
    QHashIterator<Id<Scenario::ConstraintModel>, TimeRelationModel*> timeRelationIterator(cspScenario.m_timeRelations);
    while(timeRelationIterator.hasNext())
    {
        timeRelationIterator.next();

        auto& curTimeRelationId = timeRelationIterator.key();
        auto& curTimeRelation = timeRelationIterator.value();

        auto& curConstraint = scenario.constraint(curTimeRelationId);
        auto initialDefault = curConstraint.duration.defaultDuration().msec();

        // - remove old stays
        curTimeRelation->removeStays();

        //ad new stays
        // - if constraint preceed dragged element
        auto& endTimeNodeId = endTimeNode(curConstraint, scenario).id();
        auto endTimenode = cspScenario.m_timeNodes[endTimeNodeId];

        auto distanceFromMinToDate = initialDefault - curTimeRelation->m_iscoreMin.msec();
        auto distanceFromMinToMax = curTimeRelation->m_iscoreMax.msec() - curTimeRelation->m_iscoreMin.msec();

        auto& startTimeNodeId = startTimeNode(curConstraint, scenario).id();
        auto startTimenode = cspScenario.m_timeNodes[startTimeNodeId];

        // ensure than [min - max] interval stays the same
        curTimeRelation->addStay(new kiwi::Constraint(curTimeRelation->m_max == curTimeRelation->m_min + distanceFromMinToMax,
                                                     kiwi::strength::required));
        // Try to keep min and max around default duration
        curTimeRelation->addStay(new kiwi::Constraint(curTimeRelation->m_min  == endTimenode->m_date - startTimenode->m_date - distanceFromMinToDate,
                                                      kiwi::strength::weak));
    }

    //time node stays
    // - in timenodes :
    QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario.m_timeNodes);
    while (timeNodeIterator.hasNext())
    {
        timeNodeIterator.next();

        auto& curTimeNodeId = timeNodeIterator.key();
        auto& curCspTimeNode = timeNodeIterator.value();

        // try to stay on initial value
        auto initialDate = cspScenario.getScenario()->timeNode(curTimeNodeId).date();

        // - remove old stays
        curCspTimeNode->removeStays();

        // - add new stays
        curCspTimeNode->addStay(new kiwi::Constraint(curCspTimeNode->m_date == initialDate.msec(), kiwi::strength::medium));
    }
}
}
