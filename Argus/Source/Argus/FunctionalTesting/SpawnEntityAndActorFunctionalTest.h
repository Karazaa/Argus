// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusFunctionalTest.h"
#include "RecordDefinitions/ArgusActorRecord.h"
#include "SpawnEntityAndActorFunctionalTest.generated.h"

UCLASS()
class ARGUS_API ASpawnEntityAndActorFunctionalTest : public AArgusFunctionalTest
{
	GENERATED_BODY()

private:
	static constexpr const char* k_testName = "Argus.FunctionalTest.SpawnEntityAndActor";
	static constexpr uint8       k_totalNumSteps = 4u;

public:
	ASpawnEntityAndActorFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UArgusActorRecord> m_argusActorRecord;

	UPROPERTY(EditAnywhere)
	float m_secondsToWaitAfterSpawningActor = 3.0f;

protected:
	virtual bool    DidArgusFunctionalTestFail()	override;
	virtual bool    DidCurrentTestStepSucceed()		override;
	virtual void    StartNextTestStep()				override;
	virtual uint8   GetTotalNumTestSteps()			override { return k_totalNumSteps; }

private:
	void StartLoadArgusActorRecordTestStep();
	void StartSpawnArgusEntityTestStep();
	void StartSpawnArgusActorTestStep();
	void StartDespawnArgusActorTestStep();

	bool DidLoadArgusActorRecordTestStep();
	bool DidSpawnArgusEntityTestStep();
	bool DidSpawnArgusActorTestStep();
	bool DidDespawnArgusActorTestStep();

	TObjectPtr<UArgusActorRecord> m_loadedArgusActorRecord = nullptr;
	ArgusEntity m_spawnerEntity = ArgusEntity::k_emptyEntity;
	uint16 m_expectedSpawnedEntityId = ArgusECSConstants::k_maxEntities;
};