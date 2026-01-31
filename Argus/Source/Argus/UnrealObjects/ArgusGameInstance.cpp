// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameInstance.h"
#include "ArgusActor.h"
#include "ArgusLogging.h"

UArgusStaticDatabase* UArgusGameInstance::s_staticDatabaseLoadedReference = nullptr;
UArgusGameInstance* UArgusGameInstance::s_gameInstance = nullptr;

UArgusStaticDatabase* UArgusGameInstance::GetStaticDatabase()
{
	ARGUS_RETURN_ON_NULL_POINTER(s_staticDatabaseLoadedReference, ArgusStaticDataLog);
	return s_staticDatabaseLoadedReference;
}

UArgusGameInstance* UArgusGameInstance::GetArgusGameInstance()
{
	return s_gameInstance;
}

void UArgusGameInstance::Init()
{
	s_staticDatabaseLoadedReference = m_staticDatabase.Get();
	s_gameInstance = this;
}

void UArgusGameInstance::Shutdown()
{
	s_staticDatabaseLoadedReference = nullptr;
	s_gameInstance = nullptr;
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

AArgusActor* UArgusGameInstance::GetArgusActorFromArgusEntity(ArgusEntity argusEntity) const
{
	const uint16 entityId = argusEntity.GetId();
	if (!m_argusEntityActorMap.Contains(entityId))
	{
		return nullptr;
	}

	return m_argusEntityActorMap[entityId].Get();
}
