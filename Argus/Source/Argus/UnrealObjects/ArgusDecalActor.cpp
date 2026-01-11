// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusDecalActor.h"
#include "ArgusMaterialCache.h"
#include "Components/DecalComponent.h"

void AArgusDecalActor::SetEntity(ArgusEntity entity)
{
	Super::SetEntity(entity);

	ArgusDecalComponent* decalComponent = m_entity.GetComponent<ArgusDecalComponent>();
	ARGUS_RETURN_ON_NULL(decalComponent, ArgusUnrealObjectsLog);

	UDecalComponent* unrealDecalComponent = GetComponentByClass<UDecalComponent>();
	ARGUS_RETURN_ON_NULL(unrealDecalComponent, ArgusUnrealObjectsLog);

	FArgusMaterialCache* materialCache = FArgusMaterialCache::Get();
	ARGUS_RETURN_ON_NULL(materialCache, ArgusUnrealObjectsLog);

	switch (decalComponent->m_decalType)
	{
		case EDecalType::MoveToLocation:
			unrealDecalComponent->SetDecalMaterial(materialCache->m_moveToLocationDecalMaterial.LoadAndStorePtr());
			break;
		case EDecalType::AttackMoveToLocation:
			unrealDecalComponent->SetDecalMaterial(materialCache->m_attackMoveToLocationDecalMaterial.LoadAndStorePtr());
			break;
	}
}