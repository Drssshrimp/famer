#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CatchAnimalsPawn.generated.h"

class UCameraComponent;
class UPawnMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class UFloatingPawnMovement;

UCLASS()
class CATCHANIMALS_API ACatchAnimalsPawn : public APawn
{
	GENERATED_BODY()

public:
	ACatchAnimalsPawn();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	void SetInputBlocked(bool bBlocked);

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void CatchNearbyAnimal();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

	bool bInputBlocked;
};
