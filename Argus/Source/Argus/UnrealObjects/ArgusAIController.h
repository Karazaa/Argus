// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ArgusController.h"
#include "ComponentDefinitions/IdentityComponent.h"
#include "ArgusAIController.generated.h"

class AArgusActor;
class ArgusEntity;

/**
 * 
 */
UCLASS(BlueprintType)
class ARGUS_API AArgusAIController : public AAIController, public IArgusController
{
	GENERATED_BODY()
	
public:

	/**
	* Updates which actors this controller can see based on team affiliation and fog of war.
	*/
	UFUNCTION(BlueprintCallable, Category = "Argus AIController")
	void UpdateHiddenActors();
	bool GetArgusActorsFromArgusEntityIds(const TArray<uint16>& inArgusEntityIds, TArray<AArgusActor*>& outArgusActors) const;
	bool GetArgusActorsFromArgusEntities(const TArray<ArgusEntity>& inArgusEntities, TArray<AArgusActor*>& outArgusActors) const;
	ETeam GetPlayerTeam() const { return m_playerTeam; }

	void FilterArgusActorsToPlayerTeam(TArray<AArgusActor*>& argusActors) const;
	bool IsArgusActorOnPlayerTeam(const AArgusActor* const actor) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Argus AIController")
	TArray<TSubclassOf<AArgusActor>> GetRequredEntityClasses() override { return m_requiredClasses; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Argus AIController")
	bool HasRequiredEntities() override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Argus AIController")
	TSet<TSoftObjectPtr<AArgusActor>> m_hiddenActors;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Argus AIController")
	ETeam m_playerTeam = ETeam::TeamA;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Argus AIController")
	TArray<TSubclassOf<AArgusActor>> m_requiredClasses;
};
