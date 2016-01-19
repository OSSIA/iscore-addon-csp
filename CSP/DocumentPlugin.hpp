#pragma once

#include <iscore/plugins/documentdelegate/plugin/DocumentDelegatePluginModel.hpp>
#include "Model/Scenario.hpp"

namespace iscore
{
class DocumentModel;
}

class OSSIABaseScenarioElement;

namespace CSP
{
class DocumentPlugin : public iscore::DocumentPluginModel
{
        Q_OBJECT
    public:
        DocumentPlugin(iscore::Document& doc, QObject* parent);

        void reload(iscore::DocumentModel& doc);

        ScenarioModel* getScenario() const;

    private:
        ScenarioModel* m_cspScenario;
};
}