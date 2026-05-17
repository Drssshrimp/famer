#include "CatchAnimal.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ACatchAnimal::ACatchAnimal()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(45.0f);
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

	MeshComponent->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.35f));

	AnimalId = 0;
	PlayCenter = FVector::ZeroVector;
	PlayRadius = 1600.0f;
	MoveSpeed = 170.0f;
	DirectionChangeTime = 2.0f;
	DirectionTimer = 0.0f;
	MoveDirection = FVector::ForwardVector;
	bCaught = false;
}

void ACatchAnimal::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (DynamicMaterial)
	{
		const FLinearColor Colors[] = {
			FLinearColor(0.95f, 0.25f, 0.18f),
			FLinearColor(0.15f, 0.65f, 0.95f),
			FLinearColor(0.25f, 0.85f, 0.35f),
			FLinearColor(0.98f, 0.78f, 0.18f),
			FLinearColor(0.80f, 0.32f, 0.92f)
		};
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Colors[FMath::Abs(AnimalId) % UE_ARRAY_COUNT(Colors)]);
	}

	PickNewDirection();
}

void ACatchAnimal::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bCaught)
	{
		return;
	}

	DirectionTimer -= DeltaSeconds;
	if (DirectionTimer <= 0.0f)
	{
		PickNewDirection();
	}

	FVector NewLocation = GetActorLocation() + MoveDirection * MoveSpeed * DeltaSeconds;
	NewLocation.Z = 45.0f;

	const FVector FromCenter = NewLocation - PlayCenter;
	const FVector FlatFromCenter(FromCenter.X, FromCenter.Y, 0.0f);
	if (FlatFromCenter.SizeSquared() > FMath::Square(PlayRadius))
	{
		const FVector Clamped = PlayCenter + FlatFromCenter.GetSafeNormal() * PlayRadius;
		NewLocation.X = Clamped.X;
		NewLocation.Y = Clamped.Y;
		MoveDirection = -FlatFromCenter.GetSafeNormal();
	}

	SetActorLocation(NewLocation);
	if (!MoveDirection.IsNearlyZero())
	{
		SetActorRotation(MoveDirection.Rotation());
	}
}

void ACatchAnimal::InitializeAnimal(int32 InAnimalId, const FVector& InPlayCenter, float InPlayRadius)
{
	AnimalId = InAnimalId;
	PlayCenter = InPlayCenter;
	PlayRadius = InPlayRadius;
	MoveSpeed = FMath::RandRange(120.0f, 230.0f);
	DirectionChangeTime = FMath::RandRange(1.2f, 3.5f);
	PickNewDirection();
}

void ACatchAnimal::MarkCaught()
{
	if (bCaught)
	{
		return;
	}

	bCaught = true;
	SetActorEnableCollision(false);
	Destroy();
}

void ACatchAnimal::PickNewDirection()
{
	const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
	MoveDirection = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
	DirectionTimer = DirectionChangeTime;
}
