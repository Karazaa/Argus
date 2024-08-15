// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Engine/DataAsset.h"
#include "ArgusInputActionSet.generated.h"

UCLASS()
class UArgusInputActionSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UInputAction> m_selectAction = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UInputAction> m_selectAdditiveAction = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UInputAction> m_marqueeSelectAction = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UInputAction> m_marqueeSelectAdditiveAction = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UInputAction> m_moveToAction = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UInputAction> m_setWaypointAction = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UInputAction> m_zoomAction = nullptr;
};
