Getting Started
===============

This section goes over the basics of setting up entities for use in the conversation system.


.. Basic setup:
Basic setup
-----------

All entities require a
:cpp:class:`Dialogue Component<Conversation::DialogueComponent>` to participate in dialogue.

Initiating a conversation on an entity requires that you assign dialogue to the entity.

.. Assigning Dialogue:
Assigning dialogue
------------------

Add a :cpp:class:`Conversation Asset Ref <Conversation::ConversationAssetRefComponent>` to
allow conversations to be initiated on the associated entity. Once added, assign it a
:cpp:class:`assets <Conversation::ConversationAsset>`.

.. Note:: Support for multiple asset references per entity is not yet implemented, so only
   use one asset reference for now.

.. Initiating a conversation:
Initiating a conversation
-------------------------

   To initiate a conversation, call the following function:

   .. doxygenfunction::  Conversation::DialogueComponentRequests::TryToStartConversation

