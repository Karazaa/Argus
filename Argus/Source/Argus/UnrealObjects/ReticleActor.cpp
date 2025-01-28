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
	DisableReticleDecalComponent();
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

	if (!reticleComponent->IsReticleEnabled() && !m_decalComponent->bHiddenInGame)
	{
		DisableReticleDecalComponent();
	}
	else if (reticleComponent->IsReticleEnabled())
	{
		if (m_decalComponent->bHiddenInGame)
		{
			EnableReticleDecalComponent(reticleComponent->m_abilityRecordId);
		}

		SetActorLocation(reticleComponent->m_reticleLocation);

		FLinearColor& proposedColor = reticleComponent->m_isBlocked ? m_invalidReticleColor : m_validReticleColor;
		if (proposedColor != m_decalComponent->DecalColor)
		{
			m_decalComponent->SetDecalColor(proposedColor);
		}
	}
}

void AReticleActor::EnableReticleDecalComponent(uint32 abilityRecordId)
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
		EnableReticleDecalComponentForConstruction(abilityRecord);
	}
	else
	{
		EnableReticleDecalComponentForAbility(abilityRecord);
	}

	if (UMaterial* reticleMaterial = abilityRecord->m_reticleMaterial.LoadSynchronous())
	{
		m_decalComponent->SetDecalMaterial(reticleMaterial);
	}

	m_decalComponent->SetHiddenInGame(false);
}

void AReticleActor::EnableReticleDecalComponentForConstruction(const UAbilityRecord* abilityRecord)
{
	if (!abilityRecord)
	{
		return;
	}

	if (!m_decalComponent)
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

	FVector currentDecalSize = m_decalComponent->DecalSize;
	currentDecalSize.Y = transformComponent->m_radius;
	currentDecalSize.Z = transformComponent->m_radius;
	m_decalComponent->DecalSize = currentDecalSize;
}

void AReticleActor::EnableReticleDecalComponentForAbility(const UAbilityRecord* abilityRecord)
{

}

void AReticleActor::DisableReticleDecalComponent()
{
	if (!m_decalComponent)
	{
		return;
	}

	m_decalComponent->SetHiddenInGame(true);
}