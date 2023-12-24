#pragma once

#include "AzCore/Component/ComponentApplication.h"
#include "AzCore/UnitTest/UnitTest.h"
#include "AzTest/AzTest.h"
#include "AzTest/GemTestEnvironment.h"

namespace ConversationEditorTest
{
    class ConversationEditorTestEnvironment
        : public AZ::Test::GemTestEnvironment
    {
    public:
        AZ_DISABLE_COPY_MOVE(ConversationEditorTestEnvironment); // NOLINT

        ConversationEditorTestEnvironment() = default;
        ~ConversationEditorTestEnvironment() override = default;

        void AddGemsAndComponents() override;
        auto CreateApplicationInstance() -> AZ::ComponentApplication* override;
    };
} // namespace ConversationEditorTest
