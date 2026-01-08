// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusDecalActor.h"
#include "Components/DecalComponent.h"

void AArgusDecalActor::SetEntity(ArgusEntity entity)
{
	Super::SetEntity(entity);

	ArgusDecalComponent* decalComponent = m_entity.GetComponent<ArgusDecalComponent>();
	ARGUS_RETURN_ON_NULL(decalComponent, ArgusUnrealObjectsLog);

	UDecalComponent* unrealDecalComponent = GetComponentByClass<UDecalComponent>();
	ARGUS_RETURN_ON_NULL(unrealDecalComponent, ArgusUnrealObjectsLog);

	switch (decalComponent->m_decalType)
	{
		case EDecalType::MoveToLocation:
			unrealDecalComponent->SetDecalColor(FLinearColor::Green);
			break;
		case EDecalType::AttackMoveToLocation:
			unrealDecalComponent->SetDecalColor(FLinearColor::Red);
			break;
	}
}