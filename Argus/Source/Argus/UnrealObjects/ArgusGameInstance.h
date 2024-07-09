// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusEntity.h"
#include "ArgusStaticDatabase.h"
#include "Engine/GameInstance.h"
#include "ArgusGameInstance.generated.h"

class AArgusActor;

UCLASS()
class ARGUS_API UArgusGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	static const UArgusStaticDatabase* GetStaticDatabase();
	
	virtual void Init() override;
	virtual void Shutdown() override;
	void RegisterArgusEntityActor(const TWeakObjectPtr<AArgusActor> argusActor);
	void DeregisterArgusEntityActor(const TWeakObjectPtr<AArgusActor> argusActor);

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UArgusStaticDatabase> m_staticDatabase;

private:
	static UArgusStaticDatabase* s_staticDatabaseLoadedReference;

	TMap<uint16, TWeakObjectPtr<AArgusActor>> m_argusEntityActorMap;
};
