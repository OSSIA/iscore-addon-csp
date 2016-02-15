#include <CSP/Model/Scenario.hpp>
#include <CSP/Model/TimeNode.hpp>
#include <CSP/Model/TimeRelation.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>

#include <CSP/DisplacementComputer.hpp>

namespace CSP {

void compute(
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
        initialDate = *(curDraggedCspTimeNode->getIscoreDate());
	    }

	    //weight
	    solver.addEditVariable(curDraggedCspTimeNode->getDate(),  kiwi::strength::strong + 1.0);

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
	    solver.removeEditVariable(cspScenario->m_timeNodes[curDraggedTimeNodeId]->getDate());
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
		    elementsProperties.timenodes[curTimeNodeId].oldDate = *(curCspTimenode->getIscoreDate());
		}

		// put the new date
		elementsProperties.timenodes[curTimeNodeId].newDate = TimeValue::fromMsecs(curCspTimenode->getDate().value());
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
		    elementsProperties.constraints[curTimeRelationId].oldMin = curCspTimerelation->getIscoreMin();
		    elementsProperties.constraints[curTimeRelationId].oldMax = curCspTimerelation->getIscoreMax();

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

		elementsProperties.constraints[curTimeRelationId].newMin = TimeValue::fromMsecs(curCspTimerelation->getMin().value());
        if(!curCspTimerelation->getIscoreMax().isInfinite())
            elementsProperties.constraints[curTimeRelationId].newMax = TimeValue::fromMsecs(curCspTimerelation->getMax().value());
	    }
	}
    }else
    {
	std::runtime_error("No CSP scenario found for this model");
    }
}

void computeMin(Scenario::ScenarioModel& scenario, const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements, Scenario::ElementsProperties& elementsProperties)
{
    if(ScenarioModel* cspScenario = scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        auto& solver = cspScenario->getSolver();
        QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario->m_timeNodes);
        auto startTnIt = timeNodeIterator;
        while (timeNodeIterator.hasNext())
        {
            timeNodeIterator.next();

            auto& curTimeNodeId = timeNodeIterator.key();
            auto& curCspTimeNode = timeNodeIterator.value();

            if(!positionnedElements.contains(curTimeNodeId))
            {
                solver.addEditVariable(curCspTimeNode->getDate(),  kiwi::strength::strong + 1.0);
                solver.suggestValue(curCspTimeNode->getDate(), 0);
            }
        }
        solver.updateVariables();
        timeNodeIterator = startTnIt;
        while(timeNodeIterator.hasNext())
        {
            timeNodeIterator.next();
            auto& curTimeNodeId = timeNodeIterator.key();
            auto& curCspTimeNode = timeNodeIterator.value();

            if(!positionnedElements.contains(curTimeNodeId))
                solver.removeEditVariable(curCspTimeNode->getDate());

            if(! elementsProperties.timenodes.contains(curTimeNodeId))
            {
                elementsProperties.timenodes[curTimeNodeId] = Scenario::TimenodeProperties{};
                elementsProperties.timenodes[curTimeNodeId].dateMin = curCspTimeNode->getDate().value();
            }
        }
    }
}

void computeMax(Scenario::ScenarioModel& scenario, const QVector<Id<Scenario::TimeNodeModel> >& positionnedElements, Scenario::ElementsProperties& elementsProperties)
{

    auto maxDate = scenario.duration().msec();
    if(ScenarioModel* cspScenario = scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        auto& solver = cspScenario->getSolver();
        QHashIterator<Id<Scenario::TimeNodeModel>, TimeNodeModel*> timeNodeIterator(cspScenario->m_timeNodes);
        auto startTnIt = timeNodeIterator;
        while (timeNodeIterator.hasNext())
        {
            timeNodeIterator.next();

            auto& curTimeNodeId = timeNodeIterator.key();
            auto& curCspTimeNode = timeNodeIterator.value();

            if(!positionnedElements.contains(curTimeNodeId))
            {
                solver.addEditVariable(curCspTimeNode->getDate(),  kiwi::strength::strong + 1.0);
                solver.suggestValue(curCspTimeNode->getDate(), maxDate);
            }
        }
        solver.updateVariables();
        timeNodeIterator = startTnIt;
        while(timeNodeIterator.hasNext())
        {
            timeNodeIterator.next();
            auto& curTimeNodeId = timeNodeIterator.key();
            auto& curCspTimeNode = timeNodeIterator.value();

            if(!positionnedElements.contains(curTimeNodeId))
                solver.removeEditVariable(curCspTimeNode->getDate());

            if(! elementsProperties.timenodes.contains(curTimeNodeId))
            {
                elementsProperties.timenodes[curTimeNodeId] = Scenario::TimenodeProperties{};
            }
            elementsProperties.timenodes[curTimeNodeId].dateMax = curCspTimeNode->getDate().value();
        }
    }
}

void updateConstraints(Scenario::ScenarioModel& scenario, Scenario::ElementsProperties& elementsProperties)
{
    if(ScenarioModel* cspScenario = scenario.findChild<ScenarioModel*>("CSPScenario", Qt::FindDirectChildrenOnly))
    {
        QHashIterator<Id<Scenario::ConstraintModel>, TimeRelationModel*> timeRelationIterator(cspScenario->m_timeRelations);
        while(timeRelationIterator.hasNext())
        {
            timeRelationIterator.next();
            const auto& curTimeRelationId = timeRelationIterator.key();

            auto& startTn = Scenario::startTimeNode(scenario.constraint(curTimeRelationId), scenario);
            auto& endTn = Scenario::endTimeNode(scenario.constraint(curTimeRelationId), scenario);

            if(! elementsProperties.constraints.contains(curTimeRelationId))
            {
                elementsProperties.constraints[curTimeRelationId] = Scenario::ConstraintProperties{};
                auto& iscore_cstr = scenario.constraint(curTimeRelationId);
                elementsProperties.constraints[curTimeRelationId].oldMin = iscore_cstr.duration.minDuration();
                elementsProperties.constraints[curTimeRelationId].oldMax = iscore_cstr.duration.maxDuration();
            }
            elementsProperties.constraints[curTimeRelationId].newMin.setMSecs(std::max(
                    elementsProperties.timenodes[endTn.id()].dateMin - elementsProperties.timenodes[startTn.id()].dateMax,
                    elementsProperties.constraints[curTimeRelationId].oldMin.msec()));


            elementsProperties.constraints[curTimeRelationId].newMax.setMSecs(std::min(
                    elementsProperties.timenodes[endTn.id()].dateMax - elementsProperties.timenodes[startTn.id()].dateMin,
                    elementsProperties.constraints[curTimeRelationId].oldMax.msec()));
        }
    }
}

} // namespace CSP