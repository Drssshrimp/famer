#include "CatchAnimal.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	UStaticMesh* GAnimalSphereMesh = nullptr;
	UStaticMesh* GAnimalConeMesh = nullptr;
}

ACatchAnimal::ACatchAnimal()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(45.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = CollisionComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		GAnimalSphereMesh = SphereMesh.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeMesh.Succeeded())
	{
		GAnimalConeMesh = ConeMesh.Object;
	}

	BodyMesh = CreateMeshPart(TEXT("Body"), FVector(0.0f, 0.0f, 4.0f), FVector(0.58f, 0.34f, 0.30f));
	HeadMesh = CreateMeshPart(TEXT("Head"), FVector(38.0f, 0.0f, 28.0f), FVector(0.30f, 0.26f, 0.25f));
	LeftEarMesh = CreateMeshPart(TEXT("LeftEar"), FVector(46.0f, -13.0f, 58.0f), FVector(0.10f, 0.08f, 0.30f));
	RightEarMesh = CreateMeshPart(TEXT("RightEar"), FVector(46.0f, 13.0f, 58.0f), FVector(0.10f, 0.08f, 0.30f));
	TailMesh = CreateMeshPart(TEXT("Tail"), FVector(-42.0f, 0.0f, 22.0f), FVector(0.16f, 0.16f, 0.16f));
	BeakMesh = CreateMeshPart(TEXT("Beak"), FVector(66.0f, 0.0f, 30.0f), FVector(0.10f, 0.08f, 0.16f));
	LeftWingMesh = CreateMeshPart(TEXT("LeftWing"), FVector(0.0f, -35.0f, 10.0f), FVector(0.34f, 0.08f, 0.15f));
	RightWingMesh = CreateMeshPart(TEXT("RightWing"), FVector(0.0f, 35.0f, 10.0f), FVector(0.34f, 0.08f, 0.15f));
	FrontLeftLegMesh = CreateMeshPart(TEXT("FrontLeftLeg"), FVector(26.0f, -18.0f, -20.0f), FVector(0.10f, 0.08f, 0.25f));
	FrontRightLegMesh = CreateMeshPart(TEXT("FrontRightLeg"), FVector(26.0f, 18.0f, -20.0f), FVector(0.10f, 0.08f, 0.25f));
	BackLeftLegMesh = CreateMeshPart(TEXT("BackLeftLeg"), FVector(-26.0f, -18.0f, -20.0f), FVector(0.10f, 0.08f, 0.25f));
	BackRightLegMesh = CreateMeshPart(TEXT("BackRightLeg"), FVector(-26.0f, 18.0f, -20.0f), FVector(0.10f, 0.08f, 0.25f));

	if (LeftEarMesh)
	{
		LeftEarMesh->SetRelativeRotation(FRotator(-18.0f, 0.0f, -12.0f));
	}
	if (RightEarMesh)
	{
		RightEarMesh->SetRelativeRotation(FRotator(-18.0f, 0.0f, 12.0f));
	}
	if (TailMesh)
	{
		TailMesh->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.18f));
	}

	AnimalId = 0;
	PlayCenter = FVector::ZeroVector;
	PlayRadius = 1600.0f;
	GroundZ = 55.0f;
	FlyPhase = 0.0f;
	MoveSpeed = 170.0f;
	DirectionChangeTime = 2.0f;
	DirectionTimer = 0.0f;
	MoveDirection = FVector::ForwardVector;
	bCaught = false;
	bFlyingAnimal = false;
	ConfigureGroundAnimal();
}

void ACatchAnimal::BeginPlay()
{
	Super::BeginPlay();

	ApplyAnimalColor();
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
	if (bFlyingAnimal)
	{
		FlyPhase += DeltaSeconds * 5.0f;
		NewLocation.Z = GroundZ + FMath::Sin(FlyPhase) * 35.0f;

		const float WingAngle = FMath::Sin(FlyPhase * 2.2f) * 32.0f;
		if (LeftWingMesh)
		{
			LeftWingMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, -18.0f + WingAngle));
		}
		if (RightWingMesh)
		{
			RightWingMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 18.0f - WingAngle));
		}
	}
	else
	{
		NewLocation.Z = GroundZ;
	}

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
	GroundZ = GetActorLocation().Z;
	FlyPhase = FMath::FRandRange(0.0f, 2.0f * PI);
	MoveSpeed = FMath::RandRange(120.0f, 230.0f);
	DirectionChangeTime = FMath::RandRange(1.2f, 3.5f);
	PickNewDirection();
}

void ACatchAnimal::SetFlyingAnimal(bool bInFlyingAnimal)
{
	bFlyingAnimal = bInFlyingAnimal;
	MoveSpeed = bFlyingAnimal ? FMath::RandRange(170.0f, 280.0f) : FMath::RandRange(120.0f, 230.0f);
	if (bFlyingAnimal)
	{
		GroundZ = GetActorLocation().Z + 160.0f;
		SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GroundZ));
		ConfigureBirdAnimal();
	}
	else
	{
		GroundZ = GetActorLocation().Z;
		ConfigureGroundAnimal();
	}
	ApplyAnimalColor();
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

void ACatchAnimal::ApplyAnimalColor()
{
	const FLinearColor BodyColors[] = {
		FLinearColor(0.85f, 0.58f, 0.34f),
		FLinearColor(0.92f, 0.90f, 0.82f),
		FLinearColor(0.35f, 0.28f, 0.23f),
		FLinearColor(0.72f, 0.72f, 0.68f),
		FLinearColor(0.95f, 0.78f, 0.48f)
	};

	const FLinearColor BodyColor = BodyColors[FMath::Abs(AnimalId) % UE_ARRAY_COUNT(BodyColors)];
	const FLinearColor DarkColor = BodyColor * 0.65f;

	for (UStaticMeshComponent* Part : MeshParts)
	{
		if (!Part)
		{
			continue;
		}

		UMaterialInstanceDynamic* DynamicMaterial = Part->CreateAndSetMaterialInstanceDynamic(0);
		if (DynamicMaterial)
		{
			const bool bAccentPart = Part == LeftEarMesh || Part == RightEarMesh || Part == FrontLeftLegMesh || Part == FrontRightLegMesh || Part == BackLeftLegMesh || Part == BackRightLegMesh || Part == BeakMesh;
			const FLinearColor PartColor = Part == BeakMesh ? FLinearColor(1.0f, 0.70f, 0.18f) : (bAccentPart ? DarkColor : BodyColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), PartColor);
			DynamicMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.45f);
		}
	}
}

UStaticMeshComponent* ACatchAnimal::CreateMeshPart(FName Name, const FVector& RelativeLocation, const FVector& RelativeScale)
{
	UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Part->SetupAttachment(RootComponent);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetRelativeLocation(RelativeLocation);
	Part->SetRelativeScale3D(RelativeScale);
	Part->SetStaticMesh((Name.ToString().Contains(TEXT("Ear")) || Name.ToString().Contains(TEXT("Beak"))) ? GAnimalConeMesh : GAnimalSphereMesh);
	MeshParts.Add(Part);
	return Part;
}

void ACatchAnimal::ConfigureGroundAnimal()
{
	if (BodyMesh) { BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 4.0f)); BodyMesh->SetRelativeScale3D(FVector(0.58f, 0.34f, 0.30f)); BodyMesh->SetVisibility(true); }
	if (HeadMesh) { HeadMesh->SetRelativeLocation(FVector(38.0f, 0.0f, 28.0f)); HeadMesh->SetRelativeScale3D(FVector(0.30f, 0.26f, 0.25f)); HeadMesh->SetVisibility(true); }
	if (LeftEarMesh) { LeftEarMesh->SetRelativeLocation(FVector(46.0f, -13.0f, 58.0f)); LeftEarMesh->SetRelativeScale3D(FVector(0.10f, 0.08f, 0.30f)); LeftEarMesh->SetRelativeRotation(FRotator(-18.0f, 0.0f, -12.0f)); LeftEarMesh->SetVisibility(true); }
	if (RightEarMesh) { RightEarMesh->SetRelativeLocation(FVector(46.0f, 13.0f, 58.0f)); RightEarMesh->SetRelativeScale3D(FVector(0.10f, 0.08f, 0.30f)); RightEarMesh->SetRelativeRotation(FRotator(-18.0f, 0.0f, 12.0f)); RightEarMesh->SetVisibility(true); }
	if (TailMesh) { TailMesh->SetRelativeLocation(FVector(-42.0f, 0.0f, 22.0f)); TailMesh->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.18f)); TailMesh->SetVisibility(true); }
	if (BeakMesh) { BeakMesh->SetVisibility(false); }
	if (LeftWingMesh) { LeftWingMesh->SetVisibility(false); }
	if (RightWingMesh) { RightWingMesh->SetVisibility(false); }
	if (FrontLeftLegMesh) { FrontLeftLegMesh->SetRelativeLocation(FVector(26.0f, -18.0f, -20.0f)); FrontLeftLegMesh->SetRelativeScale3D(FVector(0.10f, 0.08f, 0.25f)); FrontLeftLegMesh->SetVisibility(true); }
	if (FrontRightLegMesh) { FrontRightLegMesh->SetRelativeLocation(FVector(26.0f, 18.0f, -20.0f)); FrontRightLegMesh->SetRelativeScale3D(FVector(0.10f, 0.08f, 0.25f)); FrontRightLegMesh->SetVisibility(true); }
	if (BackLeftLegMesh) { BackLeftLegMesh->SetRelativeLocation(FVector(-26.0f, -18.0f, -20.0f)); BackLeftLegMesh->SetRelativeScale3D(FVector(0.10f, 0.08f, 0.25f)); BackLeftLegMesh->SetVisibility(true); }
	if (BackRightLegMesh) { BackRightLegMesh->SetRelativeLocation(FVector(-26.0f, 18.0f, -20.0f)); BackRightLegMesh->SetRelativeScale3D(FVector(0.10f, 0.08f, 0.25f)); BackRightLegMesh->SetVisibility(true); }
}

void ACatchAnimal::ConfigureBirdAnimal()
{
	if (BodyMesh) { BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f)); BodyMesh->SetRelativeScale3D(FVector(0.42f, 0.25f, 0.24f)); BodyMesh->SetVisibility(true); }
	if (HeadMesh) { HeadMesh->SetRelativeLocation(FVector(34.0f, 0.0f, 18.0f)); HeadMesh->SetRelativeScale3D(FVector(0.22f, 0.20f, 0.20f)); HeadMesh->SetVisibility(true); }
	if (LeftEarMesh) { LeftEarMesh->SetVisibility(false); }
	if (RightEarMesh) { RightEarMesh->SetVisibility(false); }
	if (TailMesh) { TailMesh->SetRelativeLocation(FVector(-38.0f, 0.0f, 5.0f)); TailMesh->SetRelativeScale3D(FVector(0.22f, 0.12f, 0.08f)); TailMesh->SetVisibility(true); }
	if (BeakMesh) { BeakMesh->SetRelativeLocation(FVector(58.0f, 0.0f, 18.0f)); BeakMesh->SetRelativeScale3D(FVector(0.12f, 0.08f, 0.10f)); BeakMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f)); BeakMesh->SetVisibility(true); }
	if (LeftWingMesh) { LeftWingMesh->SetRelativeLocation(FVector(0.0f, -30.0f, 4.0f)); LeftWingMesh->SetRelativeScale3D(FVector(0.38f, 0.08f, 0.13f)); LeftWingMesh->SetVisibility(true); }
	if (RightWingMesh) { RightWingMesh->SetRelativeLocation(FVector(0.0f, 30.0f, 4.0f)); RightWingMesh->SetRelativeScale3D(FVector(0.38f, 0.08f, 0.13f)); RightWingMesh->SetVisibility(true); }
	if (FrontLeftLegMesh) { FrontLeftLegMesh->SetVisibility(false); }
	if (FrontRightLegMesh) { FrontRightLegMesh->SetVisibility(false); }
	if (BackLeftLegMesh) { BackLeftLegMesh->SetVisibility(false); }
	if (BackRightLegMesh) { BackRightLegMesh->SetVisibility(false); }
}
