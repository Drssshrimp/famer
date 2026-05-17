#include "CatchAnimalsWorldSubsystem.h"

#include "CatchAnimal.h"
#include "CatchAnimalsHUDWidget.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

void UCatchAnimalsWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCatchAnimalsWorldSubsystem::Deinitialize()
{
	ClearAnimals();
	if (HudWidget)
	{
		HudWidget->RemoveFromParent();
		HudWidget = nullptr;
	}
	Super::Deinitialize();
}

void UCatchAnimalsWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	StartGame();
}

void UCatchAnimalsWorldSubsystem::Tick(float DeltaTime)
{
	if (!bStarted)
	{
		StartGame();
	}

	CheckCatchInput();

	if (bGameOver || bWaitingForContinue)
	{
		UpdateHud();
		return;
	}

	RemainingNodeTime -= DeltaTime;
	if (RemainingNodeTime <= 0.0f)
	{
		EndCurrentNode();
	}

	UpdateHud();
}

TStatId UCatchAnimalsWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCatchAnimalsWorldSubsystem, STATGROUP_Tickables);
}

bool UCatchAnimalsWorldSubsystem::IsTickable() const
{
	const UWorld* World = GetWorld();
	return World && World->IsGameWorld();
}

void UCatchAnimalsWorldSubsystem::ContinueToNextNode()
{
	if (!bWaitingForContinue || bGameOver)
	{
		return;
	}

	CurrentNodeIndex++;
	StartNode();
}

void UCatchAnimalsWorldSubsystem::StartGame()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	CachedPlayerController = UGameplayStatics::GetPlayerController(World, 0);
	CachedPawn = CachedPlayerController.IsValid() ? CachedPlayerController->GetPawn() : nullptr;
	if (!CachedPlayerController.IsValid() || !CachedPawn.IsValid())
	{
		return;
	}

	HudWidget = CreateWidget<UCatchAnimalsHUDWidget>(CachedPlayerController.Get(), UCatchAnimalsHUDWidget::StaticClass());
	if (HudWidget)
	{
		HudWidget->AddToViewport(10);
		HudWidget->OnContinueRequested.AddDynamic(this, &UCatchAnimalsWorldSubsystem::ContinueToNextNode);
	}

	bStarted = true;
	CurrentNodeIndex = 1;
	StartNode();
}

void UCatchAnimalsWorldSubsystem::StartNode()
{
	bWaitingForContinue = false;
	RemainingNodeTime = NodeDuration;
	ClearAnimals();
	SpawnAnimalsForNode();
	SetPlayerInputPaused(false);
	UpdateHud();
}

void UCatchAnimalsWorldSubsystem::EndCurrentNode()
{
	bWaitingForContinue = true;
	RemainingNodeTime = 0.0f;
	SetPlayerInputPaused(true);
	UpdateHud();
}

void UCatchAnimalsWorldSubsystem::FinishGame()
{
	bGameOver = true;
	bWaitingForContinue = false;
	RemainingNodeTime = 0.0f;
	ClearAnimals();
	SetPlayerInputPaused(true);
	UpdateHud();
}

void UCatchAnimalsWorldSubsystem::SpawnAnimalsForNode()
{
	UWorld* World = GetWorld();
	if (!World || bGameOver)
	{
		return;
	}

	const int32 NeededAnimals = FMath::Max(0, AnimalsPerNode - SpawnedAnimals.Num());
	for (int32 Index = 0; Index < NeededAnimals; ++Index)
	{
		FVector SpawnLocation;
		if (!FindGroundLocationNearPlayer(SpawnLocation))
		{
			const FVector Center = GetPlayCenter();
			const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
			const float Radius = FMath::FRandRange(250.0f, 900.0f);
			SpawnLocation = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 80.0f);
		}

		ACatchAnimal* Animal = World->SpawnActor<ACatchAnimal>(ACatchAnimal::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
		if (Animal)
		{
			Animal->InitializeAnimal(++AnimalIdCounter, GetPlayCenter(), PlayRadius);
			SpawnedAnimals.Add(Animal);
		}
	}
}

void UCatchAnimalsWorldSubsystem::ClearAnimals()
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

void UCatchAnimalsWorldSubsystem::UpdateHud()
{
	if (HudWidget)
	{
		HudWidget->SetHudState(CurrentNodeIndex, RemainingNodeTime, CapturedCount, TargetCaptureCount, bWaitingForContinue, bGameOver);
	}
}

void UCatchAnimalsWorldSubsystem::CheckCatchInput()
{
	APlayerController* PlayerController = CachedPlayerController.Get();
	if (!PlayerController)
	{
		PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		CachedPlayerController = PlayerController;
	}

	if (!PlayerController)
	{
		return;
	}

	if (PlayerController->WasInputKeyJustPressed(EKeys::SpaceBar) || PlayerController->WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		HandleCatchInput();
	}
}

void UCatchAnimalsWorldSubsystem::HandleCatchInput()
{
	if (bGameOver || bWaitingForContinue)
	{
		return;
	}

	APawn* PlayerPawn = CachedPawn.Get();
	if (!PlayerPawn)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		CachedPawn = PlayerPawn;
	}

	if (!PlayerPawn)
	{
		return;
	}

	const float CatchRadius = 260.0f;
	ACatchAnimal* BestAnimal = nullptr;
	float BestDistanceSquared = FMath::Square(CatchRadius);

	for (ACatchAnimal* Animal : SpawnedAnimals)
	{
		if (!IsValid(Animal) || Animal->IsCaught())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(PlayerPawn->GetActorLocation(), Animal->GetActorLocation());
		if (DistanceSquared <= BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestAnimal = Animal;
		}
	}

	if (BestAnimal)
	{
		CaptureAnimal(BestAnimal);
	}
}

void UCatchAnimalsWorldSubsystem::CaptureAnimal(ACatchAnimal* Animal)
{
	if (!Animal || Animal->IsCaught())
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

	if (SpawnedAnimals.Num() <= 3)
	{
		SpawnAnimalsForNode();
	}

	UpdateHud();
}

void UCatchAnimalsWorldSubsystem::SetPlayerInputPaused(bool bPaused)
{
	APlayerController* PlayerController = CachedPlayerController.Get();
	if (!PlayerController)
	{
		return;
	}

	APawn* PlayerPawn = CachedPawn.Get();
	if (!PlayerPawn)
	{
		PlayerPawn = PlayerController->GetPawn();
		CachedPawn = PlayerPawn;
	}

	if (PlayerPawn)
	{
		PlayerPawn->CustomTimeDilation = bPaused ? 0.0f : 1.0f;
	}

	PlayerController->SetIgnoreMoveInput(bPaused);
	PlayerController->SetIgnoreLookInput(bPaused);
	PlayerController->bShowMouseCursor = bPaused;

	if (bPaused)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
	}
	else
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

FVector UCatchAnimalsWorldSubsystem::GetPlayCenter() const
{
	const APawn* PlayerPawn = CachedPawn.Get();
	return PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
}

bool UCatchAnimalsWorldSubsystem::FindGroundLocationNearPlayer(FVector& OutLocation) const
{
	UWorld* World = GetWorld();
	const APawn* PlayerPawn = CachedPawn.Get();
	if (!World || !PlayerPawn)
	{
		return false;
	}

	const FVector Center = PlayerPawn->GetActorLocation();
	for (int32 Attempt = 0; Attempt < 24; ++Attempt)
	{
		const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
		const float Radius = FMath::FRandRange(350.0f, 1200.0f);
		const FVector Candidate = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
		const FVector TraceStart = Candidate + FVector(0.0f, 0.0f, 1200.0f);
		const FVector TraceEnd = Candidate - FVector(0.0f, 0.0f, 1800.0f);

		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(CatchAnimalsSpawnTrace), false);
		Params.AddIgnoredActor(PlayerPawn);

		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
		{
			OutLocation = Hit.ImpactPoint + FVector(0.0f, 0.0f, 55.0f);
			return true;
		}
	}

	return false;
}
