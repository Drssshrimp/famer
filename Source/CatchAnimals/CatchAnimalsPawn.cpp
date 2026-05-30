#include "CatchAnimalsPawn.h"

#include "CatchAnimal.h"
#include "CatchAnimalsGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ACatchAnimalsPawn::ACatchAnimalsPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(42.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMesh.Object);
	}
	MeshComponent->SetRelativeScale3D(FVector(0.65f, 0.65f, 0.65f));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->SetRelativeLocation(FVector(-650.0f, 0.0f, 720.0f));
	CameraComponent->SetRelativeRotation(FRotator(-55.0f, 0.0f, 0.0f));

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	MovementComponent->SetUpdatedComponent(CollisionComponent);
	MovementComponent->MaxSpeed = 650.0f;
	MovementComponent->Acceleration = 2400.0f;
	MovementComponent->Deceleration = 2800.0f;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	bInputBlocked = false;
}

void ACatchAnimalsPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACatchAnimalsPawn::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACatchAnimalsPawn::MoveRight);
	PlayerInputComponent->BindAction(TEXT("Catch"), IE_Pressed, this, &ACatchAnimalsPawn::CatchNearbyAnimal);
}

UPawnMovementComponent* ACatchAnimalsPawn::GetMovementComponent() const
{
	return MovementComponent;
}

void ACatchAnimalsPawn::SetInputBlocked(bool bBlocked)
{
	bInputBlocked = bBlocked;
	if (bInputBlocked)
	{
		MovementComponent->StopMovementImmediately();
	}
}

void ACatchAnimalsPawn::MoveForward(float Value)
{
	if (bInputBlocked || FMath::IsNearlyZero(Value))
	{
		return;
	}

	AddMovementInput(FVector::ForwardVector, Value);
}

void ACatchAnimalsPawn::MoveRight(float Value)
{
	if (bInputBlocked || FMath::IsNearlyZero(Value))
	{
		return;
	}

	AddMovementInput(FVector::RightVector, Value);
}

void ACatchAnimalsPawn::CatchNearbyAnimal()
{
	if (bInputBlocked)
	{
		return;
	}

	ACatchAnimalsGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ACatchAnimalsGameMode>() : nullptr;
	if (!GameMode)
	{
		return;
	}

	const float CatchRadius = 170.0f;
	TArray<AActor*> FoundAnimals;
	UGameplayStatics::GetAllActorsOfClass(this, ACatchAnimal::StaticClass(), FoundAnimals);

	ACatchAnimal* BestAnimal = nullptr;
	float BestDistanceSquared = FMath::Square(CatchRadius);
	for (AActor* Actor : FoundAnimals)
	{
		ACatchAnimal* Animal = Cast<ACatchAnimal>(Actor);
		if (!Animal || Animal->IsCaught())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(GetActorLocation(), Animal->GetActorLocation());
		if (DistanceSquared <= BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestAnimal = Animal;
		}
	}

	if (BestAnimal)
	{
		GameMode->CaptureAnimal(BestAnimal);
	}
}
