#include "AzTest/AzTest.h"
#include "Tools/ConversationCanvas/ConversationCanvasTestEnvironment.h"

namespace ConversationCanvasTest
{
    class ConversationCanvasTests : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
        }

        void TearDown() override
        {
        }
    };

    TEST_F(ConversationCanvasTests, SANITY_CHECK)
    {
    }

}; // namespace ConversationCanvasTest

AZ_UNIT_TEST_HOOK(new ConversationCanvasTest::ConversationCanvasTestEnvironment)
