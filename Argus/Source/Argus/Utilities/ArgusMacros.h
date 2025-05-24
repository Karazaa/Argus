// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

#define ARGUS_NAMEOF(x) TEXT(#x)
#define ARGUS_FUNCNAME TEXT(__FUNCTION__)

#define ARGUS_COMPONENT_SHARED	uint16 GetOwningEntityId() const; \
								void DrawComponentDebug() const;
#define ARGUS_DYNAMIC_COMPONENT_SHARED void DrawComponentDebug() const {}

#define ARGUS_SYSTEM_ARGS_SHARED bool AreComponentsValidCheck(const WIDECHAR* functionName) const;

#define ARGUS_OBSERVABLE(x, y) void Set_##y(x newValue);
#define ARGUS_OBSERVABLE_DECLARATION(x, y, z)	x y = z; \
												void Set_##y(x newValue);

#define ARGUS_FSTRING_TO_CHAR(fstring) (ANSICHAR*)StringCast<ANSICHAR, 256>(static_cast<const TCHAR*>(*fstring)).Get();

#if LOGTRACE_ENABLED 
#define ARGUS_TRACE(x) TRACE_CPUPROFILER_EVENT_SCOPE(x)
#else
#define ARGUS_TRACE(x)
#endif

#if LOGTRACE_ENABLED 
LLM_DECLARE_TAG(ArgusActorPool);
LLM_DECLARE_TAG(ArgusAvoidanceSystems);
LLM_DECLARE_TAG(ArgusComponentData);
LLM_DECLARE_TAG(ArgusDetourQuery);
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