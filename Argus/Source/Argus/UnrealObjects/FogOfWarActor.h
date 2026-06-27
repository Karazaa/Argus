// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "GameFramework/Actor.h"
#include "FogOfWarActor.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;

UCLASS()
class AFogOfWarActor : public AActor
{
	GENERATED_BODY()

public:
	static AFogOfWarActor* Get() { return k_instance; }

	AFogOfWarActor();
	void BeginDestroy() override;
	void UpdateVisibility();

protected:
	static AFogOfWarActor* k_instance;
	virtual void BeginPlay() override;

	UFUNCTION()
	void SetDynamicMaterialInstanceInECS();

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> m_planeMesh = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> m_dynamicMaterialInstance = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInterface> m_blurredMaterial = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInterface> m_noBlurMaterial = nullptr;

	UPROPERTY(EditAnywhere)
	bool m_shouldBlurFogOfWar = true;
};