#include <Builder/ConversationAssetBuilderWorker.h>

#include <algorithm>

#include <AzCore/IO/FileIO.h>
#include <AzCore/RTTI/TypeInfo.h>
#include <AzCore/Serialization/Utils.h>
#include <AzCore/std/smart_ptr/make_shared.h>

#include <AzFramework/IO/LocalFileIO.h>
#include <AzFramework/StringFunc/StringFunc.h>

#include <ConversationEditor/ConversationDocument.h>
#include <ConversationEditor/ConversationGraph.h>
#include <ConversationEditor/ConversationGraphContext.h>
#include <ConversationEditor/DataTypes.h>
#include <ConversationEditor/Nodes/Link.h>
#include <ConversationEditor/Nodes/RootNode.h>

namespace ConversationAssetBuilder
{
    static constexpr const char* const CONVERSATION_DOCUMENT_EXTENSION = Conversation::ConversationAsset::SOURCE_EXTENSION;
    static constexpr const char* const CONVERSATION_ASSET_EXTENSION = Conversation::ConversationAsset::PRODUCT_EXTENSION;
    static constexpr const char* const COMPILEKEY = "Compile Conversation";

    ConversationAssetBuilderWorker::ConversationAssetBuilderWorker() = default;
    ConversationAssetBuilderWorker::~ConversationAssetBuilderWorker() = default;

    void ConversationAssetBuilderWorker::CreateJobs(
        const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response)
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

        AssetBuilderSDK::SourceFileDependency sourceFileDependencyInfo;
        AZStd::string fullPath;
        AzFramework::StringFunc::Path::ConstructFull(request.m_watchFolder.c_str(), request.m_sourceFile.c_str(), fullPath, false);
        AzFramework::StringFunc::Path::Normalize(fullPath);
        AZStd::string relPath = request.m_sourceFile;

        if (AzFramework::StringFunc::Equal(ext.c_str(), CONVERSATION_ASSET_EXTENSION))
        {
            AzFramework::StringFunc::Path::ReplaceExtension(relPath, CONVERSATION_DOCUMENT_EXTENSION);
            // Declare and add the dependency on the *.dialoguedoc file:
            sourceFileDependencyInfo.m_sourceFileDependencyPath = relPath;
            response.m_sourceFileDependencyList.push_back(sourceFileDependencyInfo);

            // Since we're a source file, we also add a job to do the actual compilation (for each enabled platform)
            for (const AssetBuilderSDK::PlatformInfo& platformInfo : request.m_enabledPlatforms)
            {
                AssetBuilderSDK::JobDescriptor descriptor;
                descriptor.m_jobKey = COMPILEKEY;
                descriptor.SetPlatformIdentifier(platformInfo.m_identifier.c_str());

                AssetBuilderSDK::JobDependency jobDependency(
                    COMPILEKEY, platformInfo.m_identifier.c_str(), AssetBuilderSDK::JobDependencyType::Fingerprint,
                    sourceFileDependencyInfo);
                descriptor.m_jobDependencyList.push_back(jobDependency);

                // you can also place whatever parameters you want to save for later into this map:
                descriptor.m_jobParameters[AZ_CRC("hello", 0x3610a686)] = "World";
                response.m_createJobOutputs.push_back(descriptor);
            }

            response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
            return;
        }

        AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "Checking for: %s. \n", CONVERSATION_DOCUMENT_EXTENSION);

        // Create a job when for dialogue documents.
        if (AzFramework::StringFunc::Equal(ext.c_str(), CONVERSATION_DOCUMENT_EXTENSION))
        {
            // Create a job for each platform that is enabled/supported.
            for (const AssetBuilderSDK::PlatformInfo& platformInfo : request.m_enabledPlatforms)
            {
                // We create a simple job here which only contains the identifying job key and the platform to process the file on
                AssetBuilderSDK::JobDescriptor descriptor;
                descriptor.m_jobKey = COMPILEKEY;
                descriptor.SetPlatformIdentifier(platformInfo.m_identifier.c_str());

                // Note that there are additional parameters for the JobDescriptor which may be beneficial in your use case.
                // Notable ones include:
                //   * m_critical - a boolean that flags this job as one which must complete before the Editor will start up.
                //   * m_priority - an integer where larger values signify that the job should be processed with higher priority than those
                //   with lower values.
                // Please see the JobDescriptor for the full complement of configuration parameters.
                response.m_createJobOutputs.push_back(descriptor);
            }
            response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
            return;
        }
    }

    void ConversationAssetBuilderWorker::ProcessJob(
        const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response)
    {
        // This is the most basic example of handling for cancellation requests.
        // If possible, you should listen for cancellation requests and then cancel processing work to facilitate faster shutdown of the
        // Asset Processor If you need to do more things such as signal a semaphore or other threading work, derive from the Job Cancel
        // Listener and reimplement Cancel()
        AssetBuilderSDK::JobCancelListener jobCancelListener(request.m_jobId);

        // Use AZ_TracePrintf to communicate job details. The logging system will automatically file the text under the appropriate log file
        // and category.
        AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "Starting Job.\n");

        // The assets filename; e.g. "myasset.assetextension"
        AZStd::string fileName;
        AzFramework::StringFunc::Path::GetFullFileName(request.m_fullPath.c_str(), fileName);

        // The assets extension; e.g. "assetextension"
        AZStd::string ext;
        AzFramework::StringFunc::Path::GetExtension(request.m_sourceFile.c_str(), ext, false);
        // Perform work based on extension type
        if (AzFramework::StringFunc::Equal(ext.c_str(), CONVERSATION_DOCUMENT_EXTENSION))
        {
            if (AzFramework::StringFunc::Equal(request.m_jobDescription.m_jobKey.c_str(), COMPILEKEY))
            {
                // Change the extension to what will be present in the shipped product.
                AzFramework::StringFunc::Path::ReplaceExtension(fileName, "conversation");
            }
        }

        AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "Loading/deserializing the conversation document.\n");
        AZStd::shared_ptr<ConversationEditor::ConversationDocument> doc(
            AZ::Utils::LoadObjectFromFile<ConversationEditor::ConversationDocument>(request.m_fullPath.c_str()));

        if (doc)
        {
            AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "Successfully loaded/deserialized the conversation document.\n");
        }
        else
        {
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            AZ_TracePrintf(AssetBuilderSDK::ErrorWindow, "Job failed. Unable to deserialize.\n", request.m_fullPath.c_str());
            return;
        }

        AZStd::shared_ptr<ConversationEditor::ConversationGraph> graph;
        {
            GraphModel::GraphContextPtr graphContext = AZStd::make_shared<ConversationEditor::ConversationGraphContext>();

            if (doc->m_graphPtr)
            {
                doc->m_graphPtr->PostLoadSetup(graphContext); // REQUIRED AFTER DESERIALIZATION!
            }

            doc->m_graphPtr;
        }

        // All your work should happen inside the tempDirPath.
        // The Asset Processor will handle taking the completed files you specify in JobProduct.m_outputProducts from the temp directory
        // into the cache.
        AZStd::string destPath;
        AzFramework::StringFunc::Path::ConstructFull(request.m_tempDirPath.c_str(), fileName.c_str(), destPath, true);

        // Check if we are cancelled or shutting down before doing intensive processing on this source file
        if (jobCancelListener.IsCancelled())
        {
            AZ_TracePrintf(AssetBuilderSDK::WarningWindow, "Cancel was requested for job %s.\n", request.m_fullPath.c_str());
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

        AZStd::shared_ptr<Conversation::ConversationAsset> ConversationAssetData;

        if (doc->m_graphPtr)
        {
            ConversationAssetData = doc->m_graphPtr->MakeAsset();
        }
        else
        {
            // Instead of failing the build, create an empty asset.
            AZ_TracePrintf(
                AssetBuilderSDK::WarningWindow, "The conversation document's graph is null/empty. The built asset will be empty.\n");
            ConversationAssetData = AZStd::make_shared<Conversation::ConversationAsset>();
        }

        // Save the asset to the temp destination path.

        bool success = AZ::Utils::SaveObjectToFile<Conversation::ConversationAsset>(
            destPath.c_str(), AZ::DataStream::ST_JSON, ConversationAssetData.get());

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
        jobProduct.m_productSubID = 0;
        jobProduct.m_dependenciesHandled = true;

        // once you've filled up the details of the product in jobProduct, add it to the result list:
        response.m_outputProducts.push_back(jobProduct);
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;

        AZ_TracePrintf(
            AssetBuilderSDK::InfoWindow, "Job completed. Asset has %i starting IDs and %i dialogues.",
            ConversationAssetData->GetStartingIds().size(), ConversationAssetData->GetDialogues().size());
        return;
    }

    void ConversationAssetBuilderWorker::ShutDown()
    {
        m_isShuttingDown = true;
    }
} // namespace ConversationAssetBuilder