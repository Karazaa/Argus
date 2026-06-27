// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FogOfWarActor.h"
#include "ArgusEntity.h"
#include "ArgusSaveManager.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#endif //!UE_BUILD_SHIPPING

AFogOfWarActor* AFogOfWarActor::k_instance = nullptr;

AFogOfWarActor::AFogOfWarActor()
{
	PrimaryActorTick.bCanEverTick = false;
	m_planeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	ARGUS_RETURN_ON_NULL(m_planeMesh, ArgusUnrealObjectsLog);
	SetRootComponent(m_planeMesh);
	k_instance = this;
}

void AFogOfWarActor::BeginDestroy()
{
	k_instance = nullptr;
	Super::BeginDestroy();
}

void AFogOfWarActor::UpdateVisibility()
{
	bool visible = true;

#if !UE_BUILD_SHIPPING
	visible = ArgusECSDebugger::ShouldDrawFogOfWar();
#endif //!UE_BUILD_SHIPPING

	m_planeMesh->SetVisibility(visible, true);
}

void AFogOfWarActor::BeginPlay()
{
	Super::BeginPlay();
	ARGUS_RETURN_ON_NULL(m_planeMesh, ArgusUnrealObjectsLog);
	ARGUS_RETURN_ON_NULL(m_blurredMaterial, ArgusUnrealObjectsLog);
	ARGUS_RETURN_ON_NULL(m_noBlurMaterial, ArgusUnrealObjectsLog);
	const FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusUnrealObjectsLog);

	UArgusSaveManager* saveManager = UArgusSaveManager::Get();
	ARGUS_RETURN_ON_NULL(saveManager, ArgusUnrealObjectsLog);

	saveManager->m_loadCompleted.AddUniqueDynamic(this, &AFogOfWarActor::SetDynamicMaterialInstanceInECS);

	m_dynamicMaterialInstance = m_planeMesh->CreateDynamicMaterialInstance(0, fogOfWarComponent->m_shouldUseDitherAlpha ? m_noBlurMaterial : m_blurredMaterial, TEXT("FogOfWarDynamicMaterialInstance"));
	SetDynamicMaterialInstanceInECS();
}

void AFogOfWarActor::SetDynamicMaterialInstanceInECS()
{
	ARGUS_RETURN_ON_NULL(m_dynamicMaterialInstance, ArgusUnrealObjectsLog);
	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusUnrealObjectsLog);

	fogOfWarComponent->m_dynamicMaterialInstance = m_dynamicMaterialInstance;

	UpdateVisibility();
}