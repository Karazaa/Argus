// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Argus_FunctionalTest_Navigation.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AArgus_FunctionalTest_Navigation::AArgus_FunctionalTest_Navigation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ArgusEntity::FlushAllEntities();
}

void AArgus_FunctionalTest_Navigation::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(true);
}

void AArgus_FunctionalTest_Navigation::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	if (!bIsRunning && GetWorld()->HasBegunPlay())
	{
		StartNavigationTest();
	}

	CheckIfSucceeded();
}

void AArgus_FunctionalTest_Navigation::OnTimeout()
{
	Super::OnTimeout();
	ExitTest();
}

void AArgus_FunctionalTest_Navigation::StartNavigationTest()
{
	RunTest();

	if (!m_argusActor.IsValid())
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Failed test due to invalid setup. m_argusActor must be a valid reference."));
		ExitTest();
		return;
	}

	if (!m_goalActor.IsValid())
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Failed test due to invalid setup. m_goalActor must be a valid reference."));
		ExitTest();
		return;
	}

	ArgusEntity argusEntity = m_argusActor->GetEntity();
	if (!argusEntity)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Failed test due to ArgusActor not having a valid backing ArgusEntity."));
		ExitTest();
		return;
	}

	TaskComponent* taskComponent = argusEntity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Failed test due to ArgusActor not having a valid backing Task Component."));
		ExitTest();
		return;
	}

	TargetingComponent* targetingComponent = argusEntity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Failed test due to ArgusActor not having a valid backing Targeting Component."));
		ExitTest();
		return;
	}

	targetingComponent->m_targetLocation = m_goalActor->GetActorLocation();
	taskComponent->m_currentTask = ETask::ProcessMoveToLocationCommand;
}

void AArgus_FunctionalTest_Navigation::CheckIfSucceeded()
{
	if (!m_argusActor.IsValid() || !m_goalActor.IsValid())
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Failed test due to resetting actor references."));
		ExitTest();
		return;
	}

	if (FVector::DistSquared(m_argusActor->GetActorLocation(), m_goalActor->GetActorLocation()) < FMath::Square(m_successDistance))
	{
		FinishTest(EFunctionalTestResult::Succeeded, TEXT("ArgusActor successfully reached goal actor!"));
		ExitTest();
	}
}

void AArgus_FunctionalTest_Navigation::ExitTest()
{
	ArgusEntity::FlushAllEntities();
	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	if (playerController)
	{
		playerController->ConsoleCommand("quit");
	}
}