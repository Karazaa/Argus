// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "LocationMemory.h"
#include "ArgusGameInstance.h"

#include "MemoryComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARGUS_API UMemoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMemoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<AArgusActor*, FLocationMemory> m_MemoryMap;

	UFUNCTION(BlueprintCallable)
	void ReomveExpiredMemories(const float currentTime);

	UFUNCTION(BlueprintCallable)
	void UpdateMemory();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double m_memoryLifetime = 30.0f;
		
};
