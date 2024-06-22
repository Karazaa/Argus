// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameInstance.h"

UArgusStaticDatabase* UArgusGameInstance::s_staticDatabaseLoadedReference = nullptr;

UArgusStaticDatabase* UArgusGameInstance::GetStaticDatabase()
{
	// TODO JAMES: Definitely error on nullptr here.
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