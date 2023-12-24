#pragma once

#include <GraphModel/GraphModelBus.h>

namespace ConversationEditor
{
    class GraphControllerHandler
        : private GraphModelIntegration::GraphControllerNotificationBus::
              MultiHandler
    {
    public:
        GraphControllerHandler();
        ~GraphControllerHandler() override;

    private:
        //! A node has been added to the scene.
        void OnGraphModelNodeAdded(GraphModel::NodePtr /*node*/) override
        {
        }
        //! A node has been removed from the scene.
        void OnGraphModelNodeRemoved(GraphModel::NodePtr /*node*/) override
        {
        }
        //! Invoked prior to a node being removed from the scene.
        void PreOnGraphModelNodeRemoved(GraphModel::NodePtr /*node*/) override
        {
        }

        //! A connection has been added to the scene.
        void OnGraphModelConnectionAdded(
            GraphModel::ConnectionPtr /*connection*/) override
        {
        }
        //! A connection has been removed from the scene.
        void OnGraphModelConnectionRemoved(
            GraphModel::ConnectionPtr /*connection*/) override
        {
        }

        //! The specified node has been wrapped (embedded) onto the wrapperNode
        void OnGraphModelNodeWrapped(
            GraphModel::NodePtr /*wrapperNode*/,
            GraphModel::NodePtr /*node*/) override
        {
        }
        //! The specified node has been unwrapped (removed) from the wrapperNode
        void OnGraphModelNodeUnwrapped(
            GraphModel::NodePtr /*wrapperNode*/,
            GraphModel::NodePtr /*node*/) override
        {
        }

        //! Something in the graph has been modified
        //! \param node The node that was modified in the graph.  If this is
        //! nullptr, some metadata on the graph itself was modified
        void OnGraphModelGraphModified(GraphModel::NodePtr node) override;
    };
} // namespace ConversationEditor