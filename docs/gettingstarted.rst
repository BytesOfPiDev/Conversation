Getting Started
===============

This section goes over the basics of setting up entities for use in the conversation system.


.. Basic setup:
Basic setup
-----------

All entities require a
:cpp:class:`Dialogue Component<Conversation::DialogueComponent>` to take part in dialogue.

.. Assigning Dialogue:
Assigning dialogue
------------------

Initiating a conversation on an entity requires that you assign dialogue to the entity using :cpp:class:`Conversation Asset Ref <Conversation::ConversationAssetRefComponent>`.

To start a conversation with an entity, it needs a
:cpp:class:`Conversation Asset Ref Component<Conversation::ConversationAssetRefComponent>` and at
least one :cpp:class:`asset<Conversation::ConversationAsset>`.

.. Note:: Only one asset ref allowed per component - this will change in the future

.. Note:: Once the initial version is stable, the possibility of using a singular database 
   for dialogue will be looked into - it would get rid of the need to use asset references.

Using the `ConversationCanvas` to create dialogue automatically creates a companion Lua script
when it compiles the graph. Attach the script to the same entity as the DialogueComponent; it
contains the code necessary for conditional branching.

.. Note:: The need to manually add the Lua file will change in the future.

.. Initiating a conversation:
Initiating a conversation
-------------------------

To start a conversation, call the following function:

.. doxygenfunction::  Conversation::DialogueComponentRequests::TryToStartConversation

The dialogue component will attempt to start the conversation by checking each dialogue node
tagged as a `starter` dialogue. 

At least one starter dialogue must pass their `availability` check, otherwise the conversation
will fail to start. The following notification goes out after the dialogue becomes active:

.. doxygenfunction::  Conversation::DialogueComponentNotifications::OnDialogue

.. Note:: Its not currently possible to determine which starter dialogue takes priority
   this will change in a future update.

Interacting with dialogue
------------

After any dialogue becomes active, the dialogue component performs an availability check on each
of the dialogue's responses. A notification goes out after each successful AC:

.. doxygenfunction::  Conversation::DialogueComponentNotifications::OnResponseAvailable

.. Note:: You must create the interface that displays the available responses to the player. An
   example setup comes with the gem. This includes excluding or auto-selecting options based on.
   context.

With this kind of response system, a dynamic UI for displaying responses works best.

You must select a response to every active dialogue. You may need to add special behavior to certain
sets of nodes. For example, you want to automatically choose the first response if the speaker
matches the active dialogue.

