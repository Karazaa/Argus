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
	m_decalComponent = CreateDefaultSubobject<UDecalComponent>(FName("DecalComponent"));
	SetRootComponent(m_decalComponent);
	DisableReticleDecalComponent();
}

void AReticleActor::UpdateReticle()
{
	ARGUS_TRACE(AReticleActor::UpdateReticle);

	if (!m_decalComponent)
	{
		return;
	}

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
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
			EnableReticleDecalComponent(reticleComponent);
		}

		SetActorLocation(reticleComponent->m_reticleLocation);

		FLinearColor& proposedColor = reticleComponent->m_isBlocked ? m_invalidReticleColor : m_validReticleColor;
		if (proposedColor != m_decalComponent->DecalColor)
		{
			m_decalComponent->SetDecalColor(proposedColor);
		}
	}
}

void AReticleActor::EnableReticleDecalComponent(const ReticleComponent* reticleComponent)
{
	if (!m_decalComponent)
	{
		return;
	}

	if (!reticleComponent)
	{
		return;
	}

	const UAbilityRecord* abilityRecord = ArgusStaticData::GetRecord<UAbilityRecord>(reticleComponent->m_abilityRecordId);
	if (!abilityRecord)
	{
		return;
	}

	if (!abilityRecord->GetRequiresReticle())
	{
		return;
	}

	FVector currentDecalSize = m_decalComponent->DecalSize;
	currentDecalSize.Y = reticleComponent->m_radius;
	currentDecalSize.Z = reticleComponent->m_radius;
	m_decalComponent->DecalSize = currentDecalSize;

	if (UMaterialInterface* reticleMaterial = abilityRecord->m_reticleMaterial.LoadAndStorePtr())
	{
		m_decalComponent->SetDecalMaterial(reticleMaterial);
	}

	m_decalComponent->SetHiddenInGame(false);
}

void AReticleActor::DisableReticleDecalComponent()
{
	if (!m_decalComponent)
	{
		return;
	}

	m_decalComponent->SetHiddenInGame(true);
}