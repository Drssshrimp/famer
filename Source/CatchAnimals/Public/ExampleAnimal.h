#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "AIPatrolCharacter.generated.h"

UENUM(BlueprintType)
enum class EAIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),       // 待机/随机游走
    Alert       UMETA(DisplayName = "Alert"),      // 警戒（静止）
    Flee        UMETA(DisplayName = "Flee")        // 逃跑（远离玩家）
};

UCLASS()
class YOURPROJECT_API AAIPatrolCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AAIPatrolCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ─── 碰撞检测组件 ───────────────────────────────────────────
    /** 内圈：半径 500，触发逃跑 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Detection")
    USphereComponent* InnerDetectionSphere;

    /** 外圈：半径 1000，触发警戒 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Detection")
    USphereComponent* OuterDetectionSphere;

    // ─── AI 状态 ─────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
    EAIState CurrentAIState;

    // ─── 待机游走参数 ─────────────────────────────────────────────
    /** 待机时随机移动的最大半径 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Idle")
    float WanderRadius = 800.f;

    /** 两次随机移动之间的等待时间（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Idle")
    float WanderInterval = 3.f;

    /** 随机移动间隔的随机浮动范围 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Idle")
    float WanderIntervalVariance = 1.5f;

    // ─── 逃跑参数 ────────────────────────────────────────────────
    /** 逃跑时的移动速度倍率（相对默认速度） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Flee")
    float FleeSpeedMultiplier = 1.5f;

    /** 逃跑目标点距离角色自身的距离 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Flee")
    float FleeDistance = 1200.f;

private:
    // ─── 内部引用 ─────────────────────────────────────────────────
    UPROPERTY()
    AActor* TrackedPlayer;

    FTimerHandle WanderTimerHandle;
    float DefaultWalkSpeed;

    // ─── 碰撞回调 ─────────────────────────────────────────────────

    UFUNCTION()
    void OnInnerSphereBeginOverlap(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnInnerSphereEndOverlap(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    UFUNCTION()
    void OnOuterSphereBeginOverlap(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnOuterSphereEndOverlap(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    // ─── 行为逻辑 ─────────────────────────────────────────────────
    void SetAIState(EAIState NewState);
    void StartWandering();
    void DoWander();
    void StopWandering();
    void UpdateFlee(float DeltaTime);

    bool IsPlayer(AActor* Actor) const;
};
