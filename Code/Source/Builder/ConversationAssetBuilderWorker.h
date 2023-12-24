#pragma once

#include <AzCore/RTTI/RTTI.h>

#include <AssetBuilderSDK/AssetBuilderBusses.h>
#include <AssetBuilderSDK/AssetBuilderSDK.h>

namespace ConversationEditor
{
    class ConversationAssetBuilderWorker
        : public AssetBuilderSDK::AssetBuilderCommandBus::
              Handler // this will deliver you the "shut down!" message on
                      // another thread.
    {
    public:
        AZ_RTTI( // NOLINT
            ConversationAssetBuilderWorker,
            "{248EB0F8-BCAD-43E4-9355-FD6B5DE34DE2}",
            AssetBuilderSDK::AssetBuilderCommandBus::Handler);

        AZ_DISABLE_COPY_MOVE(ConversationAssetBuilderWorker); // NOLINT

        ConversationAssetBuilderWorker();
        ~ConversationAssetBuilderWorker() override;

        //! Asset Builder Callback Functions
        void CreateJobs(
            AssetBuilderSDK::CreateJobsRequest const& request,
            AssetBuilderSDK::CreateJobsResponse& response);
        void ProcessJob(
            AssetBuilderSDK::ProcessJobRequest const& request,
            AssetBuilderSDK::ProcessJobResponse& response);

        void HandleCompileKey(
            AssetBuilderSDK::ProcessJobRequest const& request,
            AssetBuilderSDK::ProcessJobResponse& response);
        void HandleCopyKey(
            AssetBuilderSDK::ProcessJobRequest const& request,
            AssetBuilderSDK::ProcessJobResponse& response);

        //////////////////////////////////////////////////////////////////////////
        //! AssetBuilderSDK::AssetBuilderCommandBus interface
        void ShutDown() override; // if you get this you must fail all existing
                                  // jobs and return.
        //////////////////////////////////////////////////////////////////////////

    private:
        bool m_isShuttingDown = false;
    };
} // namespace ConversationEditor
