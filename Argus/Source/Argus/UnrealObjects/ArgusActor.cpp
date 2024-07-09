// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActor.h"
#include "ArgusGameInstance.h"
#include "ArgusStaticData.h"
#include "Engine/World.h"

AArgusActor::AArgusActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_LastDemotable;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(FName("RootSceneComponent")));
}

ArgusEntity AArgusActor::GetEntity() const
{
	return m_entity;
}

void AArgusActor::SetSelectionState(bool isSelected)
{
	if (isSelected == m_isSelected)
	{
		return;
	}

	m_isSelected = isSelected;

	if (m_isSelected)
	{
		OnSelected();
	}
	else
	{
		OnDeselected();
	}
}

void AArgusActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (!m_entityTemplate)
	{
		return;
	}

	m_entity = m_entityTemplate->MakeEntity();
	if (TransformComponent* transformComponent = m_entity.GetComponent<TransformComponent>())
	{
		transformComponent->m_transform = GetActorTransform();
	}

	if (const IdentityComponent* identityComponent = m_entity.GetComponent<IdentityComponent>())
	{
		if (const UFactionRecord* factionRecord = ArgusStaticData::GetRecord<UFactionRecord>(identityComponent->m_factionId))
		{
			OnPopulateFaction(factionRecord);
		}
	}

	const UWorld* world = GetWorld();
	if (!world)
	{
		return;
	}

	UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		return;
	}

	gameInstance->RegisterArgusEntityActor(this);
}

void AArgusActor::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);

	if (m_entity)
	{
		return;
	}

	const UWorld* world = GetWorld();
	if (!world)
	{
		return;
	}

	UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		return;
	}

	gameInstance->DeregisterArgusEntityActor(this);
}

void AArgusActor::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (TransformComponent* transformComponent = m_entity.GetComponent<TransformComponent>())
	{
		SetActorTransform(transformComponent->m_transform);
	}
}
