#include "CatchAnimalsPlayerController.h"

ACatchAnimalsPlayerController::ACatchAnimalsPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ACatchAnimalsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetCheckpointInputMode(false);
}

void ACatchAnimalsPlayerController::SetCheckpointInputMode(bool bCheckpointMode)
{
	bShowMouseCursor = true;

	if (bCheckpointMode)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
	}
	else
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}
}
