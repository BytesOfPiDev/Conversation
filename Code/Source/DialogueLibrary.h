#pragma once

#include <ScriptCanvas/Libraries/Libraries.h>

namespace AZ
{
    class ReflectContext;
    class ComponentDescriptor;
} // namespace AZ

namespace Conversation
{
    struct DialogueLibrary : public ScriptCanvas::Library::LibraryDefinition
    {
        AZ_RTTI(DialogueLibrary, "{24EF66F2-D701-495F-A40E-B67E8246855D}", ScriptCanvas::Library::LibraryDefinition);

    public:
        static void Reflect(AZ::ReflectContext*);
        static void InitNodeRegistry(ScriptCanvas::NodeRegistry& nodeRegistry);
        static AZStd::vector<AZ::ComponentDescriptor*> GetComponentDescriptors();
    };

} // namespace Conversation
