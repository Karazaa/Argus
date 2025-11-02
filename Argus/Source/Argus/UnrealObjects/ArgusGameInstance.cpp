// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameInstance.h"
#include "ArgusActor.h"
#include "ArgusLogging.h"

UArgusStaticDatabase* UArgusGameInstance::s_staticDatabaseLoadedReference = nullptr;

UArgusStaticDatabase* UArgusGameInstance::GetStaticDatabase()
{
	ARGUS_RETURN_ON_NULL_POINTER(s_staticDatabaseLoadedReference, ArgusStaticDataLog);
	return s_staticDatabaseLoadedReference;
}

void UArgusGameInstance::Init()
{
	s_staticDatabaseLoadedReference = m_staticDatabase.Get();
}

void UArgusGameInstance::Shutdown()
{
	s_staticDatabaseLoadedReference = nullptr;
	m_argusEntityActorMap.Empty();
}

void UArgusGameInstance::RegisterArgusEntityActor(const TWeakObjectPtr<AArgusActor> argusActor)
{
	if (!argusActor.IsValid())
	{
		return;
	}

	const uint16 entityId = argusActor->GetEntity().GetId();
	if (m_argusEntityActorMap.Contains(entityId))
	{
		return;
	}

	m_argusEntityActorMap.Emplace(entityId, argusActor);
}

void UArgusGameInstance::DeregisterArgusEntityActor(const TWeakObjectPtr<AArgusActor> argusActor)
{
	if (!argusActor.IsValid())
	{
		return;
	}

	const uint16 entityId = argusActor->GetEntity().GetId();
	if (!m_argusEntityActorMap.Contains(entityId))
	{
		return;
	}

	m_argusEntityActorMap.Remove(entityId);
}

AArgusActor* UArgusGameInstance::GetArgusActorFromArgusEntity(const ArgusEntity& argusEntity) const
{
	const uint16 entityId = argusEntity.GetId();
	if (!m_argusEntityActorMap.Contains(entityId))
	{
		return nullptr;
	}

	return m_argusEntityActorMap[entityId].Get();
}

TArray<uint16, FDefaultAllocator > UArgusGameInstance::GetAllRegisteredArgusEntityIds() const
{
	TArray<uint16, FDefaultAllocator > keys;
	m_argusEntityActorMap.GetKeys(keys);
	return keys;
}
