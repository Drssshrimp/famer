#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CatchAnimalsWorldSubsystem.generated.h"

class ACatchAnimal;
class UCatchAnimalsHUDWidget;

UCLASS()
class CATCHANIMALS_API UCatchAnimalsWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	UFUNCTION()
	void ContinueToNextNode();

private:
	void StartGame();
	void StartNode();
	void EndCurrentNode();
	void FinishGame();
	void SpawnAnimalsForNode();
	void ClearAnimals();
	void UpdateHud();
	void CheckCatchInput();
	void HandleCatchInput();
	void PlayCatchAnimation() const;
	void CaptureAnimal(ACatchAnimal* Animal);
	void SetPlayerInputPaused(bool bPaused);
	FVector GetPlayCenter() const;
	bool FindGroundLocationNearPlayer(FVector& OutLocation) const;

	UPROPERTY()
	TObjectPtr<UCatchAnimalsHUDWidget> HudWidget;

	UPROPERTY()
	TArray<TObjectPtr<ACatchAnimal>> SpawnedAnimals;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> CachedPlayerController;

	UPROPERTY()
	TWeakObjectPtr<APawn> CachedPawn;

	int32 CurrentNodeIndex = 0;
	int32 CapturedCount = 0;
	int32 TargetCaptureCount = 10;
	int32 AnimalsPerNode = 8;
	int32 AnimalIdCounter = 0;
	float NodeDuration = 30.0f;
	float RemainingNodeTime = 30.0f;
	float PlayRadius = 1800.0f;
	bool bWaitingForContinue = false;
	bool bGameOver = false;
	bool bStarted = false;
};
