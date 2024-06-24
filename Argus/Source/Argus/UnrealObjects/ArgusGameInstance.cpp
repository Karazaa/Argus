// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameInstance.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

UArgusStaticDatabase* UArgusGameInstance::s_staticDatabaseLoadedReference = nullptr;

const UArgusStaticDatabase* UArgusGameInstance::GetStaticDatabase()
{
	if (!s_staticDatabaseLoadedReference)
	{
		UE_LOG
		(
			ArgusStaticDataLog, Error, 
			TEXT("[%s] No %s has been assigned to yet! Accessing database berfore %s has been called."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(s_staticDatabaseLoadedReference),
			ARGUS_NAMEOF(UArgusGameInstance::Init)
		);
		return nullptr;
	}

	return s_staticDatabaseLoadedReference;
}

void UArgusGameInstance::Init()
{
	s_staticDatabaseLoadedReference = m_staticDatabase.Get();
}

void UArgusGameInstance::Shutdown()
{
	s_staticDatabaseLoadedReference = nullptr;
}