#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CatchAnimal.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UMaterialInstanceDynamic;

UCLASS()
class CATCHANIMALS_API ACatchAnimal : public AActor
{
	GENERATED_BODY()

public:
	ACatchAnimal();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeAnimal(int32 InAnimalId, const FVector& InPlayCenter, float InPlayRadius);
	void SetFlyingAnimal(bool bInFlyingAnimal);
	void MarkCaught();

	bool IsCaught() const { return bCaught; }
	int32 GetAnimalId() const { return AnimalId; }

protected:
	virtual void BeginPlay() override;

private:
	void PickNewDirection();
	void ApplyAnimalColor();
	UStaticMeshComponent* CreateMeshPart(FName Name, const FVector& RelativeLocation, const FVector& RelativeScale);
	void ConfigureGroundAnimal();
	void ConfigureBirdAnimal();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> HeadMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> LeftEarMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> RightEarMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> TailMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BeakMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> LeftWingMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> RightWingMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FrontLeftLegMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FrontRightLegMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BackLeftLegMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BackRightLegMesh;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> MeshParts;

	int32 AnimalId;
	FVector PlayCenter;
	float PlayRadius;
	float GroundZ;
	float FlyPhase;
	float MoveSpeed;
	float DirectionChangeTime;
	float DirectionTimer;
	FVector MoveDirection;
	bool bCaught;
	bool bFlyingAnimal;
};
