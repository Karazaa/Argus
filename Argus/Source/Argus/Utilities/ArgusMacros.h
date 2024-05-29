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

// Macro used for code generation that allows for the creation of UPROPERTY macros on component data fields.
#define ARGUS_PROPERTY(...)
#define ARGUS_IGNORE()