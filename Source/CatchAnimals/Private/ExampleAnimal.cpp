#include "AIPatrolCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h" // 可选：用于调试可视化

AAIPatrolCharacter::AAIPatrolCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ─── 外圈碰撞（半径 1000）─────────────────────────────────────
    OuterDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OuterDetectionSphere"));
    OuterDetectionSphere->SetupAttachment(RootComponent);
    OuterDetectionSphere->SetSphereRadius(1000.f);
    OuterDetectionSphere->SetCollisionProfileName(TEXT("Trigger"));
    OuterDetectionSphere->SetGenerateOverlapEvents(true);

    // ─── 内圈碰撞（半径 500）─────────────────────────────────────
    InnerDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InnerDetectionSphere"));
    InnerDetectionSphere->SetupAttachment(RootComponent);
    InnerDetectionSphere->SetSphereRadius(500.f);
    InnerDetectionSphere->SetCollisionProfileName(TEXT("Trigger"));
    InnerDetectionSphere->SetGenerateOverlapEvents(true);

    // 初始状态
    CurrentAIState = EAIState::Idle;
    TrackedPlayer = nullptr;
}

void AAIPatrolCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 绑定碰撞回调
    OuterDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAIPatrolCharacter::OnOuterSphereBeginOverlap);
    OuterDetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AAIPatrolCharacter::OnOuterSphereEndOverlap);

    InnerDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAIPatrolCharacter::OnInnerSphereBeginOverlap);
    InnerDetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AAIPatrolCharacter::OnInnerSphereEndOverlap);

    // 记录默认速度，逃跑时加速
    if (GetCharacterMovement())
    {
        DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    }

    // 进入待机游走
    StartWandering();
}

void AAIPatrolCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentAIState == EAIState::Flee)
    {
        UpdateFlee(DeltaTime);
    }

#if WITH_EDITOR
    // 调试：在编辑器中可视化检测范围
    DrawDebugSphere(GetWorld(), GetActorLocation(), 500.f, 24, FColor::Red, false, -1.f, 0, 2.f);
    DrawDebugSphere(GetWorld(), GetActorLocation(), 1000.f, 24, FColor::Yellow, false, -1.f, 0, 2.f);
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// 碰撞回调
// ─────────────────────────────────────────────────────────────────────────────

void AAIPatrolCharacter::OnOuterSphereBeginOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!IsPlayer(OtherActor)) return;

    TrackedPlayer = OtherActor;

    // 只有当前是 Idle 时才切换到 Alert；若已是 Flee 则不降级
    if (CurrentAIState == EAIState::Idle)
    {
        SetAIState(EAIState::Alert);
    }
}

void AAIPatrolCharacter::OnOuterSphereEndOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!IsPlayer(OtherActor)) return;

    // 玩家完全离开外圈，回到待机
    TrackedPlayer = nullptr;
    SetAIState(EAIState::Idle);
}

void AAIPatrolCharacter::OnInnerSphereBeginOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!IsPlayer(OtherActor)) return;

    TrackedPlayer = OtherActor;
    SetAIState(EAIState::Flee);
}

void AAIPatrolCharacter::OnInnerSphereEndOverlap(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!IsPlayer(OtherActor)) return;

    // 玩家退出内圈但可能仍在外圈 → 降级到 Alert
    if (CurrentAIState == EAIState::Flee)
    {
        SetAIState(EAIState::Alert);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 状态机
// ─────────────────────────────────────────────────────────────────────────────

void AAIPatrolCharacter::SetAIState(EAIState NewState)
{
    if (CurrentAIState == NewState) return;

    EAIState OldState = CurrentAIState;
    CurrentAIState = NewState;

    UE_LOG(LogTemp, Log, TEXT("[AIPatrol] State: %d → %d"), (int32)OldState, (int32)NewState);

    // ── 退出旧状态 ──
    switch (OldState)
    {
    case EAIState::Idle:
        StopWandering();
        break;
    case EAIState::Flee:
        // 恢复默认速度
        if (GetCharacterMovement())
            GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
        break;
    default:
        break;
    }

    // ── 进入新状态 ──
    switch (NewState)
    {
    case EAIState::Idle:
        StartWandering();
        break;

    case EAIState::Alert:
    {
        // 停止所有移动，原地静止
        if (AAIController* AIC = Cast<AAIController>(GetController()))
        {
            AIC->StopMovement();
        }
        break;
    }

    case EAIState::Flee:
        // 提升速度
        if (GetCharacterMovement())
            GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * FleeSpeedMultiplier;
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 待机游走
// ─────────────────────────────────────────────────────────────────────────────

void AAIPatrolCharacter::StartWandering()
{
    // 立即执行一次，再定时循环
    DoWander();
}

void AAIPatrolCharacter::DoWander()
{
    if (CurrentAIState != EAIState::Idle) return;

    AAIController* AIC = Cast<AAIController>(GetController());
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

    if (AIC && NavSys)
    {
        FNavLocation RandomPoint;
        const bool bFound = NavSys->GetRandomReachablePointInRadius(
            GetActorLocation(), WanderRadius, RandomPoint);

        if (bFound)
        {
            AIC->MoveToLocation(RandomPoint.Location, 10.f);
        }
    }

    // 随机化下一次间隔
    const float NextInterval = WanderInterval
        + FMath::RandRange(-WanderIntervalVariance, WanderIntervalVariance);

    GetWorldTimerManager().SetTimer(
        WanderTimerHandle,
        this,
        &AAIPatrolCharacter::DoWander,
        FMath::Max(0.5f, NextInterval),
        false);
}

void AAIPatrolCharacter::StopWandering()
{
    GetWorldTimerManager().ClearTimer(WanderTimerHandle);

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        AIC->StopMovement();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 逃跑逻辑（每帧更新目标方向）
// ─────────────────────────────────────────────────────────────────────────────

void AAIPatrolCharacter::UpdateFlee(float DeltaTime)
{
    if (!TrackedPlayer) return;

    AAIController* AIC = Cast<AAIController>(GetController());
    if (!AIC) return;

    // 计算远离玩家的方向
    FVector AwayDir = (GetActorLocation() - TrackedPlayer->GetActorLocation()).GetSafeNormal();
    FVector FleeTarget = GetActorLocation() + AwayDir * FleeDistance;

    // 尝试在 NavMesh 上找到最近可达点
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation ProjectedPoint;
        if (NavSys->ProjectPointToNavigation(FleeTarget, ProjectedPoint, FVector(200.f, 200.f, 200.f)))
        {
            FleeTarget = ProjectedPoint.Location;
        }
    }

    AIC->MoveToLocation(FleeTarget, 20.f, false);
}

// ─────────────────────────────────────────────────────────────────────────────
// 辅助：判断是否为玩家
// ─────────────────────────────────────────────────────────────────────────────

bool AAIPatrolCharacter::IsPlayer(AActor* Actor) const
{
    return Actor && Actor->ActorHasTag(FName("Player"));
}
