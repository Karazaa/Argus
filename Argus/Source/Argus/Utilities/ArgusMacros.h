// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

#define ARGUS_NAMEOF(x) TEXT(#x)
#define ARGUS_FUNCNAME TEXT(__FUNCTION__)

#define ARGUS_COMPONENT_SHARED	uint16 GetOwningEntityId() const; \
								void DrawComponentDebug() const;
#define ARGUS_DYNAMIC_COMPONENT_SHARED void DrawComponentDebug() const {}

#define ARGUS_SYSTEM_ARGS_SHARED bool PopulateArguments(const ArgusEntity& entity); \
								 bool AreComponentsValidCheck(const WIDECHAR* functionName) const; \
								 ArgusEntity m_entity = ArgusEntity::k_emptyEntity;

#define ARGUS_OBSERVABLE(x, y) void Set_##y(x newValue);
#define ARGUS_OBSERVABLE_DECLARATION(x, y, z)	x y = z; \
												void Set_##y(x newValue);

#define ARGUS_FSTRING_TO_CHAR(fstring) (ANSICHAR*)StringCast<ANSICHAR, 256>(static_cast<const TCHAR*>(*fstring)).Get();

#if LOGTRACE_ENABLED 
#define ARGUS_TRACE(x) TRACE_CPUPROFILER_EVENT_SCOPE(x)
#else
#define ARGUS_TRACE(x)
#endif

#define SHOULD_ARGUS_MEMORY_TRACE 0
#define IS_PACKAGING_ARGUS 0

#if LOGTRACE_ENABLED && SHOULD_ARGUS_MEMORY_TRACE
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

// Code generation macro that allows Component Data to pass through UPROPERTY values to the associated data asset.
#define ARGUS_PROPERTY(...)

// Code generation macro that prevents Component Data from generating a UPROPERTY for this field, or prevents getting/error checking around System Args.
#define ARGUS_IGNORE()

// Code generation macro that allows getting a component but skips error checking around System Args.
#define ARGUS_GET_BUT_SKIP()

// Code generation macro that allows Component Data fields to swap out a uint ID field for an object pointer to the record type.
#define ARGUS_STATIC_DATA(x)

// Code Generation Macro that allows System Arguments to not retrieve a given component from the passed in entity since it will instead be provided from the singleton entity.
#define ARGUS_FROM_SINGLETON()

// Code Generation Macro that excludes certain component references from being considered in AreComponentsValidCheck.
#define ARGUS_NOT_REQUIRED()