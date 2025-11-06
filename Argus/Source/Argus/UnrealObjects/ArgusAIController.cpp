// Copyright Karazaa. This is a part of an RTS project called Argus.


#include "ArgusAIController.h"

#include "Engine/World.h"
#include "ArgusGameInstance.h"
#include "ArgusEntity.h"
#include "ArgusActor.h"
#include "ArgusLogging.h"
#include "ArgusGameModeBase.h"
#include "Systems/SpawningSystems.h"

void AArgusAIController::UpdateHiddenActors()
{
	m_hiddenActors.Empty();
	const UWorld* world = GetWorld();
	ARGUS_RETURN_ON_NULL(world, ArgusUnrealObjectsLog);

	const UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	ARGUS_RETURN_ON_NULL(gameInstance, ArgusUnrealObjectsLog);

	for (auto& entityId : gameInstance->GetAllRegisteredArgusEntityIds())
	{
		ArgusEntity argusEntity = ArgusEntity::RetrieveEntity(entityId);
		if (argusEntity == ArgusEntity::k_emptyEntity)
		{
			continue;
		}
		if (const IdentityComponent* identityComponent = argusEntity.GetComponent<IdentityComponent>())
		{
			if (identityComponent->m_team != m_playerTeam)
			{
				if (!identityComponent->IsSeenBy(m_playerTeam))
				{
					AArgusActor* argusActor = gameInstance->GetArgusActorFromArgusEntity(argusEntity);
					m_hiddenActors.Add(argusActor);
				}
			}
		}
	}
}

bool AArgusAIController::GetArgusActorsFromArgusEntityIds(const TArray<uint16>& inArgusEntityIds, TArray<AArgusActor*>& outArgusActors) const
{
	outArgusActors.SetNumZeroed(inArgusEntityIds.Num());

	const UWorld* world = GetWorld();
	ARGUS_RETURN_ON_NULL_BOOL(world, ArgusUnrealObjectsLog);

	const UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	ARGUS_RETURN_ON_NULL_BOOL(gameInstance, ArgusUnrealObjectsLog);

	for (int i = 0; i < inArgusEntityIds.Num(); ++i)
	{
		outArgusActors[i] = gameInstance->GetArgusActorFromArgusEntity(ArgusEntity::RetrieveEntity(inArgusEntityIds[i]));
	}

	return true;
}

bool AArgusAIController::GetArgusActorsFromArgusEntities(const TArray<ArgusEntity>& inArgusEntities, TArray<AArgusActor*>& outArgusActors) const
{
	const UWorld* world = GetWorld();
	ARGUS_RETURN_ON_NULL_BOOL(world, ArgusUnrealObjectsLog);

	const UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	ARGUS_RETURN_ON_NULL_BOOL(gameInstance, ArgusUnrealObjectsLog);
	outArgusActors.SetNumZeroed(inArgusEntities.Num());

	for (int i = 0; i < inArgusEntities.Num(); ++i)
	{
		outArgusActors[i] = gameInstance->GetArgusActorFromArgusEntity(inArgusEntities[i]);
	}

	return true;
}

void  AArgusAIController::FilterArgusActorsToPlayerTeam(TArray<AArgusActor*>& argusActors) const
{
	argusActors = argusActors.FilterByPredicate
	(
		[this](AArgusActor* actorToCheck)
		{
			if (!actorToCheck)
			{
				return false;
			}

			return actorToCheck->GetEntity().IsAlive() && !actorToCheck->GetEntity().IsPassenger() && IsArgusActorOnPlayerTeam(actorToCheck);
		}
	);

}
bool  AArgusAIController::IsArgusActorOnPlayerTeam(const AArgusActor* const actor) const
{
	ARGUS_RETURN_ON_NULL_BOOL(actor, ArgusUnrealObjectsLog);

	const IdentityComponent* identityComponent = actor->GetEntity().GetComponent<IdentityComponent>();
	if (!identityComponent)
	{
		return false;
	}

	return identityComponent->m_team == m_playerTeam;
}

bool AArgusAIController::HasRequiredEntities()
{
	UWorld* world = GetWorld();
	if (!world)
	{
		return false;
	}
	UArgusGameInstance* argusGameInstance = Cast<UArgusGameInstance>(world->GetGameInstance());
	if (!argusGameInstance)
	{
		return false;
	}
	TArray<uint16> EntityIds = argusGameInstance->GetAllRegisteredArgusEntityIds();
	TArray<AArgusActor*> argusActors;
	if (GetArgusActorsFromArgusEntityIds(EntityIds, argusActors))
	{
		FilterArgusActorsToPlayerTeam(argusActors);
		TSet<TSubclassOf<AArgusActor>> foundClasses;
		for (AArgusActor* actor : argusActors)
		{
			if (!actor)
			{
				continue;
			}
			for (TSubclassOf<AArgusActor>& requiredClass : m_requiredClasses)
			{
				if (actor->IsA(requiredClass))
				{
					foundClasses.Add(requiredClass);
				}
			}
		}
		return foundClasses.Num() == m_requiredClasses.Num();
	}
	
	return false;
}

TArray<AArgusActor*> AArgusAIController::GetAllTeamActors()
{
	TArray<AArgusActor*> allActors = GetAllArgusActors();
	FilterArgusActorsToPlayerTeam(allActors);
	return allActors;
}

TArray<AArgusActor*> AArgusAIController::GetAllArgusActors()
{
	const UWorld* world = GetWorld();
	if (!world)
	{
		return TArray<AArgusActor*>();
	}

	const UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		return TArray<AArgusActor*>();
	}

	TArray<AArgusActor*> allActors;
	GetArgusActorsFromArgusEntityIds(gameInstance->GetAllRegisteredArgusEntityIds(), allActors);
	return allActors;
}

TArray<AArgusActor*> AArgusAIController::GetArgusActorsWithTeamRelationship(const TSet<ETeamRelationship> relationships)
{

	return TArray<AArgusActor*>();
}
