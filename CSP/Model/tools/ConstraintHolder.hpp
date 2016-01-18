#pragma once

#include <QObject>
#include <QVector>
#include <CSP/Model/Scenario.hpp>
#include <kiwi/kiwi.h>

#define PUT_CONSTRAINT(constraintName, constraint) \
    kiwi::Constraint* constraintName = new kiwi::Constraint(constraint);\
    m_solver.addConstraint(*constraintName);\
    m_constraints.push_back(constraintName)

namespace CSP
{
class ConstraintHolder : public QObject
{
public:

    using QObject::QObject;
    ConstraintHolder(kiwi::Solver& solver, QObject* parent)
        :QObject::QObject{parent},
          m_solver(solver)
    {}

    virtual ~ConstraintHolder()
    {
        removeAllConstraints();
    }

    void removeAllConstraints()
    {
        for(auto constraint : m_constraints)
        {
           m_solver.removeConstraint(*constraint);
           delete constraint;
        }

        removeStays();
    }

    void addStay(kiwi::Constraint* stay)
    {
        m_solver.addConstraint(*stay);
        m_stays.push_back(stay);
    }

    void removeStays()
    {
        for(auto stay : m_stays)
        {
           m_solver.removeConstraint(*stay);
           delete stay;
        }

        m_stays.clear();//important
    }

protected:
    QVector<kiwi::Constraint*> m_constraints;
    QVector<kiwi::Constraint*> m_stays;

    kiwi::Solver& m_solver;
};
}
