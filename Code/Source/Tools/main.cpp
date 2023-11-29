#include "AzCore/Module/ModuleManagerBus.h"

#include "Tools/ConversationCanvasApplication.h"

auto main(int argc, char** argv) -> int
{
    const AZ::Debug::Trace tracer;
    AzQtComponents::AzQtApplication::InitializeDpiScaling();

    ConversationEditor::ConversationCanvasApplication app(&argc, &argv);

    if (app.LaunchLocalServer())
    {
        AZ::ComponentApplication::Descriptor appDesc;
        appDesc.m_enableScriptReflection = true;

        appDesc.m_modules.push_back(AZ::DynamicModuleDescriptor{ "LmbrCentral" });
        appDesc.m_modules.push_back(AZ::DynamicModuleDescriptor{ "ScriptCanvas" });

        AZ::ComponentApplication::StartupParameters startupParams{};

        app.Start(appDesc, startupParams);
        app.RunMainLoop();
        app.Stop();
    }

    return 0;
}
