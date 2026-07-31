// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusDirectionalLight.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusSaveManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

void AArgusDirectionalLight::BeginPlay()
{
	Super::BeginPlay();
	ARGUS_RETURN_ON_NULL(m_fogOfWarLightFunctionMaterial, ArgusUnrealObjectsLog);
	m_dynamicMaterialInstance = UMaterialInstanceDynamic::Create(m_fogOfWarLightFunctionMaterial, this);
	ARGUS_RETURN_ON_NULL(m_dynamicMaterialInstance, ArgusUnrealObjectsLog);

	UArgusSaveManager* saveManager = UArgusSaveManager::Get();
	ARGUS_RETURN_ON_NULL(saveManager, ArgusUnrealObjectsLog);

	saveManager->m_loadCompleted.AddUniqueDynamic(this, &AArgusDirectionalLight::SetDynamicMaterialInstanceInECS);

	SetDynamicMaterialInstanceInECS();

	UDirectionalLightComponent* directionalLightComponent = GetComponent();
	ARGUS_RETURN_ON_NULL(directionalLightComponent, ArgusUnrealObjectsLog);

	directionalLightComponent->SetLightFunctionMaterial(m_dynamicMaterialInstance);
}

void AArgusDirectionalLight::SetDynamicMaterialInstanceInECS()
{
	ARGUS_RETURN_ON_NULL(m_dynamicMaterialInstance, ArgusUnrealObjectsLog);
	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusUnrealObjectsLog);

	fogOfWarComponent->m_dynamicMaterialInstance = m_dynamicMaterialInstance;
}