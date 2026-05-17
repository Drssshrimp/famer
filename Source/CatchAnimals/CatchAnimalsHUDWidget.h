#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CatchAnimalsHUDWidget.generated.h"

class UButton;
class SWidget;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContinueRequested);

UCLASS()
class CATCHANIMALS_API UCatchAnimalsHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	void SetHudState(int32 NodeIndex, float RemainingSeconds, int32 CapturedCount, int32 TargetCount, bool bWaitingForContinue, bool bGameOver);

	UPROPERTY(BlueprintAssignable)
	FOnContinueRequested OnContinueRequested;

private:
	UFUNCTION()
	void HandleContinueClicked();

	void BuildWidgetTree();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TimerText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CapturedText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ContinueButton;
};
