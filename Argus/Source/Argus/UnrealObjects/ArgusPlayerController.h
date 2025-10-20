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
class UArgusUIElement;

UENUM()
enum class EArgusCursorType : uint8
{
	Default,
	Select,
	Move,
	Attack,
	Ability0,
	Ability1,
	Ability2,
	Ability3,
	ViewPortTop,
	ViewPortBottom,
	ViewPortLeft,
	ViewPortRight,
	Invalid
};

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
	ETeam GetPlayerTeam() const { return m_playerTeam; }

	void FilterArgusActorsToPlayerTeam(TArray<AArgusActor*>& argusActors) const;
	bool IsArgusActorOnPlayerTeam(const AArgusActor* const actor) const;

	void InitializeUIWidgets();

	UArgusInputManager* GetInputManager() const { return m_argusInputManager; }

	/**
	* Function to show a move to location effect for an ArgusActor. N.B. the effect would be called for each selected actor, so if you're moving 5 selected actors, this would be called 5 times.
	*
	* @Param argusActor The actor that is moving
	* @Param inputMovementState The movement state that the actor is moving in e.g. moving to a location, moving towards an enemy etc.
	* @Param targetActor The actor that is the target of the move (if applicable, nullptr if not)
	* @Param targetLocation The location that is the target of the move
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Argus|PlayerController")
	void ArgusActorMoveToLocation(AArgusActor* argusActor, EMovementState inputMovementState, AArgusActor* targetActor, FVector targetLocation);

	/**
	* Updates the cursor to the highest priority type between the current cursor and the new cursor.
	* Rather than just setting it directly the logic to determine which cursor should be shown is handled here.
	* The idea is that instead of various systems fighting over what the cursor should be, they just report what they want and if its a higher priority than the current one it gets set.
	*/
	void UpdateCursorPriority(EArgusCursorType newCursor);

	/**
	* Sets the cursor to display. Default implemntation sets the hardware cursor. Override in Blueprint if you're implementing a different type of cursor.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Argus|PlayerController")
	void SetArgusCursor(EArgusCursorType newCursorType);

	UFUNCTION(BlueprintCallable, Category = "Argus|PlayerController")
	EArgusCursorType GetArgusCursor() const;
	
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ETeam m_playerTeam = ETeam::TeamA;

	UPROPERTY(BlueprintReadOnly, Category = "Argus|PlayerController|Input")
	EArgusCursorType m_argusCursor = EArgusCursorType::Default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> m_argusInputMappingContext = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Input")
	TSoftObjectPtr<UArgusInputActionSet> m_argusInputActionSet = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSubclassOf<UArgusUIElement> m_baseCanvasUserWidgetClass = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSubclassOf<UArgusUIElement> m_selectedArgusEntityUserWidgetClass = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSubclassOf<UArgusUIElement> m_teamResourcesUserWidgetClass = nullptr;

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
	TObjectPtr<UArgusUIElement> m_baseCanvasUserWidget;

	UPROPERTY(Transient)
	TObjectPtr<UArgusUIElement> m_selectedArgusEntityUserWidget;

	UPROPERTY(Transient)
	TObjectPtr<UArgusUIElement> m_teamResourcesUserWidget;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void UpdateUIWidgetDisplay(const FVector2D& mouseScreenSpaceLocation);
};
