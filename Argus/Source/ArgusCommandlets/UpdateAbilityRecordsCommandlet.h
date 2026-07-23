// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCommandlet.h"
#include "UpdateAbilityRecordsCommandlet.generated.h"

UCLASS()
class ARGUSCOMMANDLETS_API UUpdateAbilityRecordsCommandlet : public UArgusCommandlet
{
	GENERATED_BODY()

protected:
	virtual void OnStart() override;
	virtual int32 DoWork() override;
	virtual void OnFinish() override;
};