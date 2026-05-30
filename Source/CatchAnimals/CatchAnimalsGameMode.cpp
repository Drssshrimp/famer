#include "CatchAnimalsGameMode.h"

#include "CatchAnimal.h"
#include "CatchAnimalsHUDWidget.h"
#include "CatchAnimalsPawn.h"
#include "CatchAnimalsPlayerController.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

ACatchAnimalsGameMode::ACatchAnimalsGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultPawnClass = ACatchAnimalsPawn::StaticClass();
	PlayerControllerClass = ACatchAnimalsPlayerController::StaticClass();
	HUDClass = nullptr;

	CurrentNodeIndex = 0;
	CapturedCount = 0;
	TargetCaptureCount = 10;
	AnimalsPerNode = 6;
	AnimalIdCounter = 0;
	NodeDuration = 30.0f;
	RemainingNodeTime = NodeDuration;
	PlayRadius = 1700.0f;
	bWaitingForContinue = false;
	bGameOver = false;
}

void ACatchAnimalsGameMode::BeginPlay()
{
	Super::BeginPlay();

	BuildRuntimeLevel();
	EnsurePlayerPawn();

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		HudWidget = CreateWidget<UCatchAnimalsHUDWidget>(PlayerController, UCatchAnimalsHUDWidget::StaticClass());
		if (HudWidget)
		{
			HudWidget->AddToViewport();
			HudWidget->OnContinueRequested.AddDynamic(this, &ACatchAnimalsGameMode::ContinueToNextNode);
		}
	}

	CurrentNodeIndex = 1;
	StartNode();
}

void ACatchAnimalsGameMode::EnsurePlayerPawn()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!World || !PlayerController || PlayerController->GetPawn())
	{
		return;
	}

	ACatchAnimalsPawn* PlayerPawn = World->SpawnActor<ACatchAnimalsPawn>(
		ACatchAnimalsPawn::StaticClass(),
		FVector(0.0f, 0.0f, 70.0f),
		FRotator::ZeroRotator);

	if (PlayerPawn)
	{
		PlayerController->Possess(PlayerPawn);
	}
}

void ACatchAnimalsGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bGameOver || bWaitingForContinue)
	{
		UpdateHud();
		return;
	}

	RemainingNodeTime -= DeltaSeconds;
	if (RemainingNodeTime <= 0.0f)
	{
		EndCurrentNode();
	}

	UpdateHud();
}

void ACatchAnimalsGameMode::CaptureAnimal(ACatchAnimal* Animal)
{
	if (bGameOver || bWaitingForContinue || !Animal || Animal->IsCaught())
	{
		return;
	}

	Animal->MarkCaught();
	SpawnedAnimals.RemoveAll([Animal](const TObjectPtr<ACatchAnimal>& SpawnedAnimal)
	{
		return SpawnedAnimal == Animal;
	});
	CapturedCount++;

	if (CapturedCount >= TargetCaptureCount)
	{
		FinishGame();
		return;
	}

	if (SpawnedAnimals.Num() <= 2)
	{
		SpawnAnimalsForNode();
	}

	UpdateHud();
}

void ACatchAnimalsGameMode::ContinueToNextNode()
{
	if (!bWaitingForContinue || bGameOver)
	{
		return;
	}

	CurrentNodeIndex++;
	StartNode();
}

void ACatchAnimalsGameMode::BuildRuntimeLevel()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const bool bHasGround = UGameplayStatics::GetActorOfClass(this, AStaticMeshActor::StaticClass()) != nullptr;
	if (!bHasGround)
	{
		AStaticMeshActor* Ground = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Ground)
		{
			UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
			if (PlaneMesh)
			{
				Ground->GetStaticMeshComponent()->SetStaticMesh(PlaneMesh);
			}

			Ground->SetActorScale3D(FVector(45.0f, 45.0f, 1.0f));
			Ground->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
			Ground->GetStaticMeshComponent()->SetMaterial(0, nullptr);
		}
	}

	if (!UGameplayStatics::GetActorOfClass(this, ADirectionalLight::StaticClass()))
	{
		ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0.0f, 0.0f, 900.0f), FRotator(-45.0f, -30.0f, 0.0f));
		if (Sun)
		{
			Sun->GetLightComponent()->SetIntensity(3.2f);
		}
	}

	if (!UGameplayStatics::GetActorOfClass(this, ASkyLight::StaticClass()))
	{
		ASkyLight* SkyLight = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (SkyLight)
		{
			SkyLight->GetLightComponent()->SetIntensity(1.0f);
		}
	}
}

void ACatchAnimalsGameMode::StartNode()
{
	bWaitingForContinue = false;
	RemainingNodeTime = NodeDuration;
	ClearAnimals();
	SpawnAnimalsForNode();
	SetPlayerPausedForCheckpoint(false);
	UpdateHud();
}

void ACatchAnimalsGameMode::EndCurrentNode()
{
	bWaitingForContinue = true;
	RemainingNodeTime = 0.0f;
	SetPlayerPausedForCheckpoint(true);
	UpdateHud();
}

void ACatchAnimalsGameMode::FinishGame()
{
	bGameOver = true;
	bWaitingForContinue = false;
	RemainingNodeTime = 0.0f;
	ClearAnimals();
	SetPlayerPausedForCheckpoint(true);
	UpdateHud();
}

void ACatchAnimalsGameMode::SpawnAnimalsForNode()
{
	UWorld* World = GetWorld();
	if (!World || bGameOver)
	{
		return;
	}

	const int32 NeededAnimals = FMath::Max(0, AnimalsPerNode - SpawnedAnimals.Num());
	for (int32 Index = 0; Index < NeededAnimals; ++Index)
	{
		const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
		const float Radius = FMath::Sqrt(FMath::FRand()) * (PlayRadius - 220.0f);
		const FVector SpawnLocation(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 45.0f);

		ACatchAnimal* Animal = World->SpawnActor<ACatchAnimal>(ACatchAnimal::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
		if (Animal)
		{
			Animal->InitializeAnimal(++AnimalIdCounter, FVector::ZeroVector, PlayRadius);
			SpawnedAnimals.Add(Animal);
		}
	}
}

void ACatchAnimalsGameMode::ClearAnimals()
{
	for (ACatchAnimal* Animal : SpawnedAnimals)
	{
		if (IsValid(Animal))
		{
			Animal->Destroy();
		}
	}
	SpawnedAnimals.Empty();
}

void ACatchAnimalsGameMode::UpdateHud()
{
	if (HudWidget)
	{
		HudWidget->SetHudState(CurrentNodeIndex, RemainingNodeTime, CapturedCount, TargetCaptureCount, bWaitingForContinue, bGameOver);
	}
}

void ACatchAnimalsGameMode::SetPlayerPausedForCheckpoint(bool bPaused)
{
	ACatchAnimalsPawn* PlayerPawn = Cast<ACatchAnimalsPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (PlayerPawn)
	{
		PlayerPawn->SetInputBlocked(bPaused);
	}

	ACatchAnimalsPlayerController* PlayerController = Cast<ACatchAnimalsPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (PlayerController)
	{
		PlayerController->SetCheckpointInputMode(bPaused);
	}
}
