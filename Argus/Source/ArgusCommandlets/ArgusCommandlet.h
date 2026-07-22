// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Commandlets/Commandlet.h"
#include "ArgusCommandlet.generated.h"

UCLASS()
class ARGUSCOMMANDLETS_API UArgusCommandlet : public UCommandlet 
{
	GENERATED_BODY()

public:
	UArgusCommandlet();

	int32 Main(const FString& parameters) final;

protected:
	virtual void OnStart() {}
	virtual void DoWork() {}
	virtual void OnFinish() {}
};