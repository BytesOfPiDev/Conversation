#pragma once

#include "AzCore/Component/ComponentApplication.h"
#include "AzCore/UnitTest/UnitTest.h"
#include "AzTest/GemTestEnvironment.h"

namespace ConversationTest
{
    class ConversationTestEnvironment : public AZ::Test::GemTestEnvironment
    {
    public:
        ConversationTestEnvironment() = default;
        ~ConversationTestEnvironment() override = default;

        void AddGemsAndComponents() override;
        auto CreateApplicationInstance() -> AZ::ComponentApplication* override;

    private:
    };
} // namespace ConversationTest
