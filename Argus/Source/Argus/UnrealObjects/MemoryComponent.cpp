// Copyright Karazaa. This is a part of an RTS project called Argus.



#include "MemoryComponent.h"
#include "Engine/World.h"
#include "ArgusGameInstance.h"
#include "ArgusActor.h"

#include "ArgusController.h"

// Sets default values for this component's properties
UMemoryComponent::UMemoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void UMemoryComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateMemory();
}


// Called every frame
void UMemoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateMemory();
	// ...
}

void UMemoryComponent::ReomveExpiredMemories(const float currentTime)
{
	{
		TArray<AArgusActor*> keysToRemove;
		for (auto& [actor, mem] : m_MemoryMap)
		{
			if (!UKismetSystemLibrary::IsValid(actor) || mem.IsExpired(currentTime))
			{
				
					keysToRemove.Add(actor);
			}
		}
		for (const AArgusActor* key : keysToRemove)
		{
			m_MemoryMap.Remove(key);
		}
	}
}

void UMemoryComponent::UpdateMemory()
{
	
	UWorld* world = GetWorld();
	if (!world)
	{
		m_MemoryMap.Empty();
		return;
	}
	UArgusGameInstance* argusGameInstance = Cast<UArgusGameInstance>(world->GetGameInstance());
	if (!argusGameInstance)
	{
		m_MemoryMap.Empty();
		return;
	}
	const float time = world->GetTimeSeconds();
	ReomveExpiredMemories(time);
	if (IArgusController* argusController = Cast<IArgusController>(GetOwner()))
	{
		ETeam playerTeam = argusController->GetControlledTeam();
		TArray<uint16> entityIds = argusGameInstance->GetAllRegisteredArgusEntityIds();
		for (const uint16 entityId : entityIds)
		{
			ArgusEntity entity = ArgusEntity::RetrieveEntity(entityId);
			// ignore entities on our team
			if (entity.IsOnTeam(playerTeam))
			{
				continue;
			}
			// create a memory if it's not on our team, it can be killed or has extractable resources, and we can see it.
			if (entity.IsAlive() || entity.HasExtractableResources())
			{
				if (AArgusActor* argusActor = argusGameInstance->GetArgusActorFromArgusEntity(entity))
				{
					if (!argusActor)
					{
						continue;
					}

					if (argusActor->IsSeenBy(playerTeam))
					{
						m_MemoryMap.Add(argusActor, FLocationMemory(time, argusActor->GetActorLocation(), m_memoryLifetime));
					}
				}
			}
		}
	}
		
	
}


