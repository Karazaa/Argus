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
class UArgusInputActionSet;
class UArgusInputManager;

UCLASS()
class AArgusPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void ProcessArgusPlayerInput(float deltaTime);

	AArgusCameraActor::UpdateCameraPanningParameters GetScreenSpaceInputValues() const;

	bool GetMouseProjectionLocation(FHitResult& outHitResult) const;
	bool GetArgusActorsFromArgusEntities(const TArray<ArgusEntity>& inArgusEntities, TArray<AArgusActor*>& outArgusActors) const;

	void FilterArgusActorsToPlayerTeam(TArray<AArgusActor*>& argusActors) const;
	bool IsArgusActorOnPlayerTeam(const AArgusActor* const actor) const;

	void InitializeUIWidgets();

	UArgusInputManager* GetInputManager() const { return m_argusInputManager; }

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ETeam m_playerTeam = ETeam::TeamA;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UInputMappingContext> m_argusInputMappingContext = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UArgusInputActionSet> m_argusInputActionSet = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UUserWidget> m_selectedArgusEntityUserWidgetClass;

	TObjectPtr<AArgusCameraActor> m_argusCameraActor = nullptr;
	TObjectPtr<UArgusInputManager> m_argusInputManager = nullptr;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};
