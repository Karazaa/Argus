// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameInstance.h"
#include "ArgusActor.h"
#include "ArgusLogging.h"

UArgusGameInstance* UArgusGameInstance::s_gameInstance = nullptr;

UArgusGameInstance* UArgusGameInstance::GetArgusGameInstance()
{
	return s_gameInstance;
}

void UArgusGameInstance::Init()
{
	s_gameInstance = this;
}

void UArgusGameInstance::Shutdown()
{
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

	const TWeakObjectPtr<AArgusActor>* weakActorPointer = m_argusEntityActorMap.Find(entityId);
	if (!weakActorPointer)
	{
		return nullptr;
	}

	return weakActorPointer->Get();
}
