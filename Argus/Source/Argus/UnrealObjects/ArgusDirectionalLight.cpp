// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusDirectionalLight.h"
#include "ArgusCameraActor.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusSaveManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#endif //!UE_BUILD_SHIPPING

AArgusDirectionalLight* AArgusDirectionalLight::k_instance = nullptr;

void AArgusDirectionalLight::UpdateVisibility()
{
	bool visible = true;

#if !UE_BUILD_SHIPPING
	visible = ArgusECSDebugger::ShouldDrawFogOfWar();
#endif //!UE_BUILD_SHIPPING

	ULightComponent* directionalLightComponent = GetLightComponent();
	ARGUS_RETURN_ON_NULL(directionalLightComponent, ArgusUnrealObjectsLog);

	directionalLightComponent->SetLightFunctionMaterial(visible ? m_dynamicMaterialInstance : nullptr);
}

void AArgusDirectionalLight::BeginPlay()
{
	k_instance = this;

	Super::BeginPlay();
	ARGUS_RETURN_ON_NULL(m_fogOfWarLightFunctionMaterial, ArgusUnrealObjectsLog);
	m_dynamicMaterialInstance = UMaterialInstanceDynamic::Create(m_fogOfWarLightFunctionMaterial, this);
	ARGUS_RETURN_ON_NULL(m_dynamicMaterialInstance, ArgusUnrealObjectsLog);

	UArgusSaveManager* saveManager = UArgusSaveManager::Get();
	ARGUS_RETURN_ON_NULL(saveManager, ArgusUnrealObjectsLog);

	saveManager->m_loadCompleted.AddUniqueDynamic(this, &AArgusDirectionalLight::SetDynamicMaterialInstanceInECS);

	SetDynamicMaterialInstanceInECS();

	ULightComponent* directionalLightComponent = GetLightComponent();
	ARGUS_RETURN_ON_NULL(directionalLightComponent, ArgusUnrealObjectsLog);

	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(directionalLightComponent, ArgusUnrealObjectsLog);

	bool visible = true;

#if !UE_BUILD_SHIPPING
	visible = ArgusECSDebugger::ShouldDrawFogOfWar();
#endif //!UE_BUILD_SHIPPING

	const float spatialBounds = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	if (visible)
	{
		directionalLightComponent->SetLightFunctionMaterial(m_dynamicMaterialInstance);
	}
	
	directionalLightComponent->SetLightFunctionScale(FVector(spatialBounds, spatialBounds, AArgusCameraActor::k_cameraTraceLength));
}

void AArgusDirectionalLight::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	k_instance = nullptr;
	Super::EndPlay(endPlayReason);
}

void AArgusDirectionalLight::SetDynamicMaterialInstanceInECS()
{
	ARGUS_RETURN_ON_NULL(m_dynamicMaterialInstance, ArgusUnrealObjectsLog);
	FogOfWarComponent* fogOfWarComponent = ArgusEntity::GetSingletonEntity().GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusUnrealObjectsLog);

	fogOfWarComponent->m_dynamicMaterialInstance = m_dynamicMaterialInstance;
}