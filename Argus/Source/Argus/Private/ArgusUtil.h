// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

#define ARGUS_NAMEOF(x) TEXT(#x)
#define ARGUS_FUNCNAME TEXT(__FUNCTION__)

// Macro used for code generation that allows for the creation of UPROPERTY macros on component data fields.
#define ARGUS_PROPERTY(...)

DECLARE_LOG_CATEGORY_EXTERN(ArgusGameLog, Display, All);

namespace ArgusUtil
{

};
