#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CatchAnimalsGameMode.generated.h"

class ACatchAnimal;
class ACatchAnimalsPawn;
class UCatchAnimalsHUDWidget;

UCLASS()
class CATCHANIMALS_API ACatchAnimalsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACatchAnimalsGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void CaptureAnimal(ACatchAnimal* Animal);

	UFUNCTION()
	void ContinueToNextNode();

private:
	void EnsurePlayerPawn();
	void BuildRuntimeLevel();
	void StartNode();
	void EndCurrentNode();
	void FinishGame();
	void SpawnAnimalsForNode();
	void ClearAnimals();
	void UpdateHud();
	void SetPlayerPausedForCheckpoint(bool bPaused);

	UPROPERTY()
	TObjectPtr<UCatchAnimalsHUDWidget> HudWidget;

	UPROPERTY()
	TArray<TObjectPtr<ACatchAnimal>> SpawnedAnimals;

	int32 CurrentNodeIndex;
	int32 CapturedCount;
	int32 TargetCaptureCount;
	int32 AnimalsPerNode;
	int32 AnimalIdCounter;
	float NodeDuration;
	float RemainingNodeTime;
	float PlayRadius;
	bool bWaitingForContinue;
	bool bGameOver;
};
