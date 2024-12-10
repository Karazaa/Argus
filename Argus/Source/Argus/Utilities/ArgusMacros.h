// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

#define ARGUS_NAMEOF(x) TEXT(#x)
#define ARGUS_FUNCNAME TEXT(__FUNCTION__)

#if LOGTRACE_ENABLED 
#define ARGUS_TRACE(x) TRACE_CPUPROFILER_EVENT_SCOPE(x)
#else
#define ARGUS_TRACE(x)
#endif

#if LOGTRACE_ENABLED 
LLM_DECLARE_TAG(ArgusActorPool);
LLM_DECLARE_TAG(ArgusAvoidanceSystems);
LLM_DECLARE_TAG(ArgusComponentData);
LLM_DECLARE_TAG(ArgusInputManager);
LLM_DECLARE_TAG(ArgusKDTree);
LLM_DECLARE_TAG(ArgusNavigationSystems);
LLM_DECLARE_TAG(ArgusStaticData);
LLM_DECLARE_TAG(ArgusTimerSystems);
#define ARGUS_MEMORY_TRACE(x) LLM_SCOPE_BYTAG(x)
#else
#define ARGUS_MEMORY_TRACE(x)
#endif

// Macro used for code generation that allows for the creation of UPROPERTY macros on component data fields.
#define ARGUS_PROPERTY(...)
#define ARGUS_IGNORE()
#define ARGUS_STATIC_DATA(x)