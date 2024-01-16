Getting Started
===============

Each entity that wants to be a part of the conversation system needs a
:cpp:class:`Dialogue Component<Conversation::DialogueComponent>` attached to it.

If the entity only needs to take part in conversations started by other
entities, then there's nothing left to do. 

However, if you'd like the player to
be able to start a conversation with the entity, you'll need to give it at
least one :cpp:class:`Conversation Asset <Conversation::ConversationAsset>` on the entity.

