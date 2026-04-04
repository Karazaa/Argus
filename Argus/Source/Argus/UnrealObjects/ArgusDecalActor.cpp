// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusDecalActor.h"
#include "ArgusStaticData.h"
#include "Components/DecalComponent.h"

void AArgusDecalActor::SetEntity(ArgusEntity entity)
{
	Super::SetEntity(entity);

	ArgusDecalComponent* decalComponent = m_entity.GetComponent<ArgusDecalComponent>();
	ARGUS_RETURN_ON_NULL(decalComponent, ArgusUnrealObjectsLog);

	UDecalComponent* unrealDecalComponent = GetComponentByClass<UDecalComponent>();
	ARGUS_RETURN_ON_NULL(unrealDecalComponent, ArgusUnrealObjectsLog);

	const DecalSystemsSettingsComponent* settings = DecalSystemsSettingsComponent::Get();
	ARGUS_RETURN_ON_NULL(settings, ArgusECSLog);

	switch (decalComponent->m_decalType)
	{
		case EDecalType::MoveToLocation:
			if (const UMaterialRecord* moveMaterialRecord = ArgusStaticData::GetRecord<UMaterialRecord>(settings->m_moveToLocationDecalMaterial.GetId()))
			{
				unrealDecalComponent->SetDecalMaterial(moveMaterialRecord->m_material.LoadAndStorePtr());
			}
			break;
		case EDecalType::AttackMoveToLocation:
			if (const UMaterialRecord* attackMoveMaterialRecord = ArgusStaticData::GetRecord<UMaterialRecord>(settings->m_attackMoveToLocationDecalMaterial.GetId()))
			{
				unrealDecalComponent->SetDecalMaterial(attackMoveMaterialRecord->m_material.LoadAndStorePtr());
			}
			break;
	}
}