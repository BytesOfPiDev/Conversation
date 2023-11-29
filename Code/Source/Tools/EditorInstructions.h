#pragma once

#include "AzCore/Memory/Memory_fwd.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/RTTI/TypeInfoSimple.h"

namespace ConversationEditor
{
    class EditorInstructions
    {
    public:
        AZ_TYPE_INFO_WITH_NAME_DECL(EditorInstructions); // NOLINT
        AZ_CLASS_ALLOCATOR_DECL; // NOLINT
        AZ_DEFAULT_COPY_MOVE(EditorInstructions); // NOLINT

        static void Reflect(AZ::ReflectContext* context);

        EditorInstructions() = default;
        ~EditorInstructions() = default;

        [[nodiscard]] auto GetValue() const -> AZStd::string_view
        {
            return m_instructions;
        }

    private:
        AZStd::string m_instructions{};
    };
} // namespace ConversationEditor
