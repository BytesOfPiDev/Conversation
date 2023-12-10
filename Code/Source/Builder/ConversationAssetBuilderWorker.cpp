#include "Builder/ConversationAssetBuilderWorker.h"

#include "AssetBuilderSDK/AssetBuilderSDK.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Debug/Trace.h"
#include "AzCore/Script/ScriptAsset.h"
#include "AzCore/Serialization/Utils.h"

#include "AzCore/StringFunc/StringFunc.h"
#include "Conversation/ConversationAsset.h"

namespace ConversationEditor
{
    constexpr auto const CompileKey = "Compile Conversation";
    constexpr auto const CopyKey = "Copy Conversation Asset";

    ConversationAssetBuilderWorker::ConversationAssetBuilderWorker() = default;
    ConversationAssetBuilderWorker::~ConversationAssetBuilderWorker() = default;

    void ConversationAssetBuilderWorker::CreateJobs(
        AssetBuilderSDK::CreateJobsRequest const& request, AssetBuilderSDK::CreateJobsResponse& response)
    {
        if (m_isShuttingDown)
        {
            response.m_result = AssetBuilderSDK::CreateJobsResultCode::ShuttingDown;
            return;
        }
        // Retrieve the file's extension.
        AZStd::string ext;
        AzFramework::StringFunc::Path::GetExtension(request.m_sourceFile.c_str(), ext, false);
        // Ensure that it matches the extension we're looking for.

        AZStd::string fullPath;
        AzFramework::StringFunc::Path::ConstructFull(request.m_watchFolder.c_str(), request.m_sourceFile.c_str(), fullPath, false);
        AzFramework::StringFunc::Path::Normalize(fullPath);
        AZStd::string relPath = request.m_sourceFile;

        // Handle assets that have already been processed outside of this worker.
        if (AzFramework::StringFunc::Equal(ext.c_str(), Conversation::ConversationAsset::ProductExtension))
        {
            AZ_TracePrintf(
                AssetBuilderSDK::InfoWindow, "Handling extension: %s. \n", Conversation::ConversationAsset::ProductExtension); // NOLINT

            for (AssetBuilderSDK::PlatformInfo const& platformInfo : request.m_enabledPlatforms)
            {
                AssetBuilderSDK::JobDescriptor descriptor;
                descriptor.m_jobKey = CopyKey;
                descriptor.SetPlatformIdentifier(platformInfo.m_identifier.c_str());

                response.m_createJobOutputs.push_back(descriptor);
            }

            response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
            return;
        }

        // Extension not handled. Currently, there's no need to handle a source asset since the editor is taking care of that for now with
        // ConversationGraphCompiler.
        response.m_result = AssetBuilderSDK::CreateJobsResultCode::Failed;
    }

    void ConversationAssetBuilderWorker::ProcessJob(
        AssetBuilderSDK::ProcessJobRequest const& request, AssetBuilderSDK::ProcessJobResponse& response)
    {
        if (!AZ::StringFunc::Equal(request.m_jobDescription.m_jobKey, CopyKey))
        {
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            AZ_TracePrintf(
                AssetBuilderSDK::ErrorWindow, "Job failed. Unsupported job key: '%s'\n", request.m_jobDescription.m_jobKey.c_str());
        }
        // This is the most basic example of handling for cancellation requests.
        // If possible, you should listen for cancellation requests and then cancel processing work to facilitate faster shutdown of the
        // Asset Processor If you need to do more things such as signal a semaphore or other threading work, derive from the Job Cancel
        // Listener and reimplement Cancel()
        AssetBuilderSDK::JobCancelListener jobCancelListener(request.m_jobId);

        // Use AZ_TracePrintf to communicate job details. The logging system will automatically file the text under the appropriate log file
        // and category.
        AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "Starting Job.\n"); // NOLINT

        // The assets filename; e.g. "myasset.assetextension"
        AZStd::string fileName;
        AzFramework::StringFunc::Path::GetFullFileName(request.m_fullPath.c_str(), fileName);

        // The assets extension; e.g. "assetextension"
        AZStd::string ext;
        AzFramework::StringFunc::Path::GetExtension(request.m_sourceFile.c_str(), ext, false);

        // Perform work based on extension type
        if (AzFramework::StringFunc::Equal(ext.c_str(), Conversation::ConversationAsset::SourceExtension))
        {
            if (AzFramework::StringFunc::Equal(request.m_jobDescription.m_jobKey.c_str(), CompileKey))
            {
                // Change the extension to what will be present in the shipped product.
                AzFramework::StringFunc::Path::ReplaceExtension(fileName, Conversation::ConversationAsset::ProductExtension);
            }
        }

        AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "Loading/deserializing the conversation document.\n"); // NOLINT

        AZStd::unique_ptr<Conversation::ConversationAsset> conversationAsset{
            AZ::Utils::LoadObjectFromFile<Conversation::ConversationAsset>(request.m_fullPath.c_str())
        };

        if (conversationAsset)
        {
            AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "Successfully loaded/deserialized the conversation document.\n"); // NOLINT
        }
        else
        {
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            AZ_TracePrintf(AssetBuilderSDK::ErrorWindow, "Job failed. Unable to load/deserialize the conversation document.\n");
            return;
        }

        // All your work should happen inside the tempDirPath.
        // The Asset Processor will handle taking the completed files you specify in JobProduct.m_outputProducts from the temp directory
        // into the cache.
        AZStd::string destPath;
        AzFramework::StringFunc::Path::ConstructFull(request.m_tempDirPath.c_str(), fileName.c_str(), destPath, true);

        // Check if we are cancelled or shutting down before doing intensive processing on this source file
        if (jobCancelListener.IsCancelled())
        {
            AZ_TracePrintf(AssetBuilderSDK::WarningWindow, "Cancel was requested for job %s.\n", request.m_fullPath.c_str()); // NOLINT
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Cancelled;
            return;
        }
        if (m_isShuttingDown)
        {
            AZ_TracePrintf(
                AssetBuilderSDK::WarningWindow, "Cancelled job %s because shutdown was requested.\n", request.m_fullPath.c_str());
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Cancelled;
            return;
        }

        // Save the asset to the temp destination path.
        bool const success = AZ::Utils::SaveObjectToFile<Conversation::ConversationAsset>(
            destPath.c_str(), AZ::DataStream::ST_JSON, conversationAsset.get());

        if (!success)
        {
            AZ_TracePrintf(AssetBuilderSDK::ErrorWindow, "Job failed. Could not save to temporary folder.\n");
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        // Push all products successfully built into the JobProduct.m_outputProducts.
        // Filepaths can be absolute, or relative to your temporary directory.
        // The job request struct has the temp directory, so it will be properly reconstructed to an absolute path later.
        AssetBuilderSDK::JobProduct jobProduct(fileName);

        jobProduct.m_productAssetType = AZ::AzTypeInfo<Conversation::ConversationAsset>::Uuid();
        jobProduct.m_productSubID = Conversation::ConversationAsset::ProductAssetSubId;
        jobProduct.m_dependenciesHandled = true;

        // once you've filled up the details of the product in jobProduct, add it to the result list:
        response.m_outputProducts.push_back(jobProduct);
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;

        AZ_TracePrintf(
            AssetBuilderSDK::InfoWindow,
            "Job completed. Asset has %i starting IDs and %i dialogues.",
            conversationAsset->GetStartingIds().size(),
            conversationAsset->GetDialogues().size());
        return;
    }

    void ConversationAssetBuilderWorker::ShutDown()
    {
        m_isShuttingDown = true;
    }

    void ConversationAssetBuilderWorker::HandleCompileKey(AssetBuilderSDK::ProcessJobRequest const&, AssetBuilderSDK::ProcessJobResponse&)
    {
    }

    void ConversationAssetBuilderWorker::HandleCopyKey(AssetBuilderSDK::ProcessJobRequest const&, AssetBuilderSDK::ProcessJobResponse&)
    {
    }

} // namespace ConversationEditor
