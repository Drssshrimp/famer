#include "CatchAnimalsHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Widgets/SWidget.h"

namespace
{
	UTextBlock* MakeHudText(UWidgetTree* WidgetTree, FName Name, float FontSize, const FLinearColor& Color)
	{
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Text->SetColorAndOpacity(FSlateColor(Color));
		Text->SetJustification(ETextJustify::Left);
		FSlateFontInfo FontInfo = Text->GetFont();
		FontInfo.Size = FontSize;
		Text->SetFont(FontInfo);
		return Text;
	}
}

TSharedRef<SWidget> UCatchAnimalsHUDWidget::RebuildWidget()
{
	BuildWidgetTree();
	return Super::RebuildWidget();
}

void UCatchAnimalsHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)
	{
		ContinueButton->OnClicked.RemoveAll(this);
		ContinueButton->OnClicked.AddDynamic(this, &UCatchAnimalsHUDWidget::HandleContinueClicked);
	}

	SetHudState(1, 30.0f, 0, 10, false, false);
}

void UCatchAnimalsHUDWidget::BuildWidgetTree()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InfoPanel"));
	PanelBorder->SetBrushColor(FLinearColor(0.02f, 0.025f, 0.03f, 0.82f));
	PanelBorder->SetPadding(FMargin(18.0f, 14.0f));

	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelBorder);
	PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
	PanelSlot->SetAlignment(FVector2D(0.0f, 0.0f));
	PanelSlot->SetPosition(FVector2D(24.0f, 24.0f));
	PanelSlot->SetSize(FVector2D(420.0f, 180.0f));

	UVerticalBox* PanelBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PanelBox"));
	PanelBorder->SetContent(PanelBox);

	TitleText = MakeHudText(WidgetTree, TEXT("TitleText"), 24.0f, FLinearColor(0.95f, 0.96f, 1.0f));
	PanelBox->AddChildToVerticalBox(TitleText);

	UHorizontalBox* StatsBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("StatsBox"));
	PanelBox->AddChildToVerticalBox(StatsBox);

	TimerText = MakeHudText(WidgetTree, TEXT("TimerText"), 20.0f, FLinearColor(0.98f, 0.82f, 0.22f));
	CapturedText = MakeHudText(WidgetTree, TEXT("CapturedText"), 20.0f, FLinearColor(0.40f, 0.92f, 0.58f));
	StatsBox->AddChild(TimerText);
	StatsBox->AddChild(CapturedText);

	HintText = MakeHudText(WidgetTree, TEXT("HintText"), 16.0f, FLinearColor(0.78f, 0.84f, 0.92f));
	UVerticalBoxSlot* HintSlot = PanelBox->AddChildToVerticalBox(HintText);
	HintSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));

	ContinueButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ContinueButton"));

	UTextBlock* ContinueText = MakeHudText(WidgetTree, TEXT("ContinueText"), 20.0f, FLinearColor::White);
	ContinueText->SetText(FText::FromString(TEXT("Continue")));
	ContinueText->SetJustification(ETextJustify::Center);
	ContinueButton->SetContent(ContinueText);

	UCanvasPanelSlot* ContinueSlot = RootCanvas->AddChildToCanvas(ContinueButton);
	ContinueSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
	ContinueSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	ContinueSlot->SetPosition(FVector2D::ZeroVector);
	ContinueSlot->SetSize(FVector2D(260.0f, 64.0f));

	SuccessText = MakeHudText(WidgetTree, TEXT("SuccessText"), 72.0f, FLinearColor(0.28f, 1.0f, 0.42f));
	SuccessText->SetText(FText::FromString(TEXT("success")));
	SuccessText->SetJustification(ETextJustify::Center);

	UCanvasPanelSlot* SuccessSlot = RootCanvas->AddChildToCanvas(SuccessText);
	SuccessSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
	SuccessSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	SuccessSlot->SetPosition(FVector2D(0.0f, -120.0f));
	SuccessSlot->SetSize(FVector2D(520.0f, 110.0f));
}

void UCatchAnimalsHUDWidget::SetHudState(int32 NodeIndex, float RemainingSeconds, int32 CapturedCount, int32 TargetCount, bool bWaitingForContinue, bool bGameOver)
{
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(bGameOver ? TEXT("Game Over") : FString::Printf(TEXT("Node %d"), NodeIndex)));
	}

	if (TimerText)
	{
		const int32 DisplaySeconds = FMath::Max(0, FMath::CeilToInt(RemainingSeconds));
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("Time: %02ds    "), DisplaySeconds)));
	}

	if (CapturedText)
	{
		CapturedText->SetText(FText::FromString(FString::Printf(TEXT("Caught: %d/%d"), CapturedCount, TargetCount)));
	}

	if (HintText)
	{
		FString Hint = TEXT("WASD to move. Press Space or Left Mouse near an animal.");
		if (bWaitingForContinue)
		{
			Hint = TEXT("Node ended. Click Continue to enter the next node.");
		}
		if (bGameOver)
		{
			Hint = TEXT("success");
		}
		HintText->SetText(FText::FromString(Hint));
	}

	if (ContinueButton)
	{
		ContinueButton->SetVisibility(bWaitingForContinue && !bGameOver ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (SuccessText)
	{
		SuccessText->SetVisibility(bGameOver ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UCatchAnimalsHUDWidget::HandleContinueClicked()
{
	OnContinueRequested.Broadcast();
}
