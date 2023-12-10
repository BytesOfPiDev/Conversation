set(FILES
    Include/Conversation/Components/DialogueComponentConfig.h
    Include/Conversation/DialogueData_incl.h

    Source/ConversationModuleInterface.h
    Source/ConversationSystemComponent.cpp
    Source/ConversationSystemComponent.h

    Source/ConversationAsset.cpp
    Source/DialogueComponent.cpp
    Source/DialogueData.cpp
    Source/DialogueData_incl.cpp

    Source/Components/DialogueComponentConfig.cpp
    Source/Components/ConversationAssetRefComponent.cpp
    Source/Components/ConversationAssetRefComponent.h

    Source/Conditions/ComparisonOp.cpp
    Source/Conditions/ComparisonOp.h
    Source/Conditions/ConditionFunction.cpp
    Source/Conditions/ConditionFunction.h

    Source/VirtualMachine/ConversationVM.cpp
    Source/VirtualMachine/ConversationVM.h
    Source/VirtualMachine/Value.cpp
    Source/VirtualMachine/Value.h

    Source/StateMachine/ConversationSM.cpp
    Source/StateMachine/ConversationSM.h
)
