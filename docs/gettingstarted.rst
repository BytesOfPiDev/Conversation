Getting Started
===============

This section goes over the basics of setting up entities for use in the conversation system.


Basic setup
-----------

Each entity that wants to be a part of the conversation system needs a
:cpp:class:`Dialogue Component<Conversation::DialogueComponent>` attached to it. It describes the
very basics of the entity to the conversation system, such as
a name and `Speaker Tag`. 

See :cpp:class:`Dialogue Component Config` for the available settings.

Once done, the entity is ready to take part in conversations! If the entity only participates in
conversations started by others, then you're done. However, if you'd like a conversation to be
initiated on the entity, then a few more steps are necessary.

Assigning dialogue
------------------

To be able to initiate a conversation on an entity, it will need some dialogue assigned to it, which
are stored in :cpp:class:`assets <Conversation::ConversationAsset>`. You will need to add a
:cpp:class:`Conversation Asset Ref <Conversation::ConversationAssetRefComponent>` to the entity.
Once added, you assign the desired asset to the component, allowing the conversation(s) represented
by the asset to be started by speaking to that entity.

