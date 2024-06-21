// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusStaticDatabase.h"
#include "Engine/GameInstance.h"
#include "ArgusGameInstance.generated.h"

UCLASS()
class ARGUS_API UArgusGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UArgusStaticDatabase> m_staticDatabase;
	
};
