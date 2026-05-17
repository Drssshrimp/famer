#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CatchAnimal.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class CATCHANIMALS_API ACatchAnimal : public AActor
{
	GENERATED_BODY()

public:
	ACatchAnimal();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeAnimal(int32 InAnimalId, const FVector& InPlayCenter, float InPlayRadius);
	void MarkCaught();

	bool IsCaught() const { return bCaught; }
	int32 GetAnimalId() const { return AnimalId; }

protected:
	virtual void BeginPlay() override;

private:
	void PickNewDirection();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	int32 AnimalId;
	FVector PlayCenter;
	float PlayRadius;
	float GroundZ;
	float MoveSpeed;
	float DirectionChangeTime;
	float DirectionTimer;
	FVector MoveDirection;
	bool bCaught;
};
