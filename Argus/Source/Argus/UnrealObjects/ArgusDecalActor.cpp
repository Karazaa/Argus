// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusDecalActor.h"
#include "Components/DecalComponent.h"

void AArgusDecalActor::SetEntity(ArgusEntity entity)
{
	Super::SetEntity(entity);

	ArgusDecalComponent* decalComponent = m_entity.GetComponent<ArgusDecalComponent>();
	ARGUS_RETURN_ON_NULL(decalComponent, ArgusUnrealObjectsLog);

	MaterialCacheComponent* materialCacheComponent = ArgusEntity::GetSingletonEntity().GetComponent<MaterialCacheComponent>();
	ARGUS_RETURN_ON_NULL(materialCacheComponent, ArgusUnrealObjectsLog);

	UDecalComponent* unrealDecalComponent = GetComponentByClass<UDecalComponent>();
	ARGUS_RETURN_ON_NULL(unrealDecalComponent, ArgusUnrealObjectsLog);

	switch (decalComponent->m_decalType)
	{
		case EDecalType::MoveToLocation:
			unrealDecalComponent->SetDecalMaterial(materialCacheComponent->m_moveToLocationDecalMaterial.LoadAndStorePtr());
			break;
		case EDecalType::AttackMoveToLocation:
			unrealDecalComponent->SetDecalMaterial(materialCacheComponent->m_attackMoveToLocationDecalMaterial.LoadAndStorePtr());
			break;
	}
}