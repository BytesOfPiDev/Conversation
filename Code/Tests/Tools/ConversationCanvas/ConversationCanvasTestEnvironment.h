#pragma once

#include <AzCore/UnitTest/UnitTest.h>

namespace ConversationCanvasTest
{
    class ConversationCanvasTestEnvironment : public UnitTest::TraceBusHook
    {
    public:
        ConversationCanvasTestEnvironment();
        ~ConversationCanvasTestEnvironment() override;
    };
} // namespace ConversationCanvasTest
