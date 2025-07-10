// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCameraActor.h"
#include "ComponentDefinitions/IdentityComponent.h"
#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"

#include "ArgusPlayerController.generated.h"

class AArgusActor;
class ArgusEntity;
class AReticleActor;
class UArgusInputActionSet;
class UArgusInputManager;
class UArgusUserWidget;

UCLASS()
class AArgusPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void ProcessArgusPlayerInput(float deltaTime);

	AArgusCameraActor::UpdateCameraPanningParameters GetScreenSpaceInputValues() const;

	bool GetMouseProjectionLocation(ECollisionChannel collisionTraceChannel, FHitResult& outHitResult) const;
	AArgusActor* GetArgusActorForArgusEntityId(uint16 entityId) const;
	AArgusActor* GetArgusActorForArgusEntity(const ArgusEntity& entity) const;
	bool GetArgusActorsFromArgusEntityIds(const TArray<uint16>& inArgusEntityIds, TArray<AArgusActor*>& outArgusActors) const;
	bool GetArgusActorsFromArgusEntities(const TArray<ArgusEntity>& inArgusEntities, TArray<AArgusActor*>& outArgusActors) const;

	void FilterArgusActorsToPlayerTeam(TArray<AArgusActor*>& argusActors) const;
	bool IsArgusActorOnPlayerTeam(const AArgusActor* const actor) const;

	void InitializeUIWidgets();

	UArgusInputManager* GetInputManager() const { return m_argusInputManager; }

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ETeam m_playerTeam = ETeam::TeamA;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> m_argusInputMappingContext = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Input")
	TSoftObjectPtr<UArgusInputActionSet> m_argusInputActionSet = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSubclassOf<UArgusUserWidget> m_baseCanvasUserWidgetClass = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSubclassOf<UArgusUserWidget> m_selectedArgusEntityUserWidgetClass = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSubclassOf<UArgusUserWidget> m_teamResourcesUserWidgetClass = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Camera")
	TSoftClassPtr<AArgusCameraActor> m_argusCameraClass = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Camera")
	FTransform m_defaultInitialCameraTransform = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Reticle")
	TSoftClassPtr<AReticleActor> m_reticleClass = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Reticle")
	FTransform m_defaultInitialReticleTransform = FTransform::Identity;

	UPROPERTY(Transient)
	TObjectPtr<AArgusCameraActor> m_argusCameraActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AReticleActor> m_reticleActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UArgusInputManager> m_argusInputManager = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UArgusUserWidget> m_baseCanvasUserWidget;

	UPROPERTY(Transient)
	TObjectPtr<UArgusUserWidget> m_selectedArgusEntityUserWidget;

	UPROPERTY(Transient)
	TObjectPtr<UArgusUserWidget> m_teamResourcesUserWidget;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void UpdateUIWidgetDisplay(const FVector2D& mouseScreenSpaceLocation);
};
