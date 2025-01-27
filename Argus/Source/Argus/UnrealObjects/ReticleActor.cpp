// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ReticleActor.h"
#include "ArgusEntity.h"
#include "Components/DecalComponent.h"

AReticleActor::AReticleActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_LastDemotable;

	m_decalComponent = CreateDefaultSubobject<UDecalComponent>(FName("DecalComponent"));
	SetRootComponent(m_decalComponent);
	DisableReticle();
}

void AReticleActor::BeginPlay()
{
	Super::BeginPlay();
}

void AReticleActor::Tick(float deltaTime)
{
	if (!m_decalComponent)
	{
		return;
	}

	ArgusEntity singletonEntity = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId);
	if (!singletonEntity)
	{
		return;
	}

	const ReticleComponent* reticleComponent = singletonEntity.GetComponent<ReticleComponent>();
	if (!reticleComponent)
	{
		return;
	}

	if (reticleComponent->m_abilityRecordId == 0u && !m_decalComponent->bHiddenInGame)
	{
		DisableReticle();
	}
	else if (reticleComponent->m_abilityRecordId != 0u && m_decalComponent->bHiddenInGame)
	{
		EnableReticle(reticleComponent->m_abilityRecordId);
	}
}

void AReticleActor::EnableReticle(uint32 abilityRecordId)
{
	if (!m_decalComponent)
	{
		return;
	}
}

void AReticleActor::DisableReticle()
{
	if (!m_decalComponent)
	{
		return;
	}

	m_decalComponent->SetHiddenInGame(true);
}