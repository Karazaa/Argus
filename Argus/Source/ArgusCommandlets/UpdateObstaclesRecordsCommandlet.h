// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusCommandlet.h"
#include "UpdateObstaclesRecordsCommandlet.generated.h"

UCLASS()
class ARGUSCOMMANDLETS_API UUpdateObstaclesRecordsCommandlet : public UArgusCommandlet
{
	GENERATED_BODY()

protected:
	virtual void OnStart() override;
	virtual int32 DoWork() override;
	virtual void OnFinish() override;
};