// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Engine/DirectionalLight.h"
#include "ArgusDirectionalLight.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;

UCLASS()
class AArgusDirectionalLight : public ADirectionalLight
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void SetDynamicMaterialInstanceInECS();

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInterface> m_fogOfWarLightFunctionMaterial = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> m_dynamicMaterialInstance = nullptr;
};