#include <iscore_addon_csp.hpp>
#include <CSP/ApplicationPlugin.hpp>
#include <CSP/MoveEventCSPFactory.hpp>
#include <CSP/MoveEventCSPFlexFactory.hpp>

#include <iscore/plugins/customfactory/FactorySetup.hpp>

iscore_addon_csp::iscore_addon_csp() :
    QObject {}
{
}

iscore_addon_csp::~iscore_addon_csp()
{

}

iscore::GUIApplicationContextPlugin* iscore_addon_csp::make_applicationPlugin(
        const iscore::ApplicationContext& app)
{
    return new CSP::ApplicationPlugin{app};
}

std::vector<std::unique_ptr<iscore::FactoryInterfaceBase>>
iscore_addon_csp::factories(
        const iscore::ApplicationContext& ctx,
        const iscore::AbstractFactoryKey& key) const
{
    return instantiate_factories<
            iscore::ApplicationContext,
    TL<
        FW<Scenario::Command::MoveEventFactoryInterface,
            CSP::MoveEventCSPFactory,
            CSP::MoveEventCSPFlexFactory>
      >
    >(ctx, key);
}

int32_t iscore_addon_csp::version() const
{
    return 1;
}

UuidKey<iscore::Plugin> iscore_addon_csp::key() const
{
    return "8953366a-947c-453c-9e8e-395e0e39be25";
}
