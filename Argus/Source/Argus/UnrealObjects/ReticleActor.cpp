// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ReticleActor.h"
#include "ArgusEntity.h"
#include "ArgusEntityTemplate.h"
#include "ArgusStaticData.h"
#include "Components/DecalComponent.h"
#include "DataComponentDefinitions/TransformComponentData.h"
#include "RecordDefinitions/AbilityRecord.h"

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

	const UAbilityRecord* abilityRecord = ArgusStaticData::GetRecord<UAbilityRecord>(abilityRecordId);
	if (!abilityRecord)
	{
		return;
	}

	if (!abilityRecord->m_requiresReticle)
	{
		return;
	}

	if (abilityRecord->m_abilityType == EAbilityTypes::Construct)
	{
		EnableConstructionReticle(abilityRecord);
	}
	else
	{
		EnableAbilityReticle(abilityRecord);
	}

	m_decalComponent->SetHiddenInGame(false);
}

void AReticleActor::EnableConstructionReticle(const UAbilityRecord* abilityRecord)
{
	if (!abilityRecord)
	{
		return;
	}

	const UArgusActorRecord* argusActorRecord = abilityRecord->m_argusActorRecord.LoadSynchronous();
	if (!argusActorRecord)
	{
		return;
	}

	const UArgusEntityTemplate* argusEntityTemplate = argusActorRecord->m_entityTemplateOverride.LoadSynchronous();
	if (!argusEntityTemplate)
	{
		return;
	}

	const UTransformComponentData* transformComponent = argusEntityTemplate->GetComponentFromTemplate<UTransformComponentData>();
	if (!transformComponent)
	{
		return;
	}

	const float reticleRadius = transformComponent->m_radius;
}

void AReticleActor::EnableAbilityReticle(const UAbilityRecord* abilityRecord)
{

}

void AReticleActor::DisableReticle()
{
	if (!m_decalComponent)
	{
		return;
	}

	m_decalComponent->SetHiddenInGame(true);
}