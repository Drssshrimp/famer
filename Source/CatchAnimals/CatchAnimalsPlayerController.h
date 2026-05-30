#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CatchAnimalsPlayerController.generated.h"

UCLASS()
class CATCHANIMALS_API ACatchAnimalsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACatchAnimalsPlayerController();

	virtual void BeginPlay() override;

	void SetCheckpointInputMode(bool bCheckpointMode);
};
