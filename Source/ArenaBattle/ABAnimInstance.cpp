// Fill out your copyright notice in the Description page of Project Settings.

#include "ABAnimInstance.h"

UABAnimInstance::UABAnimInstance()
{
	CurrentPawnSpeed = 0.0f;
	CurrentPawnAcecl = 0.0f;


	IsDead	= false;
	IsInAir = false;
	IsAccelerating = false;

	Yaw			= 0.0f;
	Pitch		= 0.0f;
	Roll		= 0.0f;
	YawDelta	= 0.0f;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ATTACK_MONTAGE(TEXT("/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations/Greystone_Skeleton_Montage.Greystone_Skeleton_Montage"));

	if (ATTACK_MONTAGE.Succeeded())
	{
		AttackMontage = ATTACK_MONTAGE.Object;
	}
}

void UABAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	auto Pawn = TryGetPawnOwner();
	if (!::IsValid(Pawn)) return;

	if (!IsDead) 
	{
		if (::IsValid(Pawn))
		{
			CurrentPawnSpeed = Pawn->GetVelocity().Size();
			auto Character = Cast<ACharacter>(Pawn);
			if (Character)
			{
				// 공중에 떠있는지 확인
				IsInAir = Character->GetMovementComponent()->IsFalling();
				
				if (Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
				{
					IsAccelerating = true;
				}
				else
				{
					IsAccelerating = false;
				}

				// 2019-02-07 wsshin 델타 로테이션 구하기
				FRotator AimRotation = Character->GetBaseAimRotation();
				FRotator ActorRotation = Character->GetActorRotation();
				FRotator DeltaRotation = AimRotation - ActorRotation;
				
				Yaw			= DeltaRotation.Yaw;
				Pitch		= DeltaRotation.Pitch;
				Roll		= DeltaRotation.Roll;

				FRotator DeltaRotation2 = RotationLastTick - ActorRotation;
				
				if (GetCurveValue(TEXT("FullBody")) > 0)
				{
					IsFullBody = true;
				}
				else
				{
					IsFullBody = false;
				}

				YawDelta = FMath::FInterpTo(YawDelta, (DeltaRotation2.Yaw / DeltaSeconds) / 7.0, DeltaSeconds, 6.0);

				ABLOG(Warning, TEXT("IsAccelerating : %d"), IsAccelerating);
				ABLOG(Warning, TEXT("IsInAir : %d"), IsInAir);

				RotationLastTick = Character->GetActorRotation();
			}
		}
	}
}


void UABAnimInstance::PlayAttackMontage()
{
	ABCHECK(!IsDead);
	Montage_Play(AttackMontage, 1.0f);
}

void UABAnimInstance::AnimNotify_EndCheck()
{
	ABLOG_S(Warning);
	OnCharacterJogStartCheck.Broadcast();
}


// 몽타주에 AttackHitCheck 노티파이가 발생하였을 시 호출 됨 
void UABAnimInstance::AnimNotify_AttackHitCheck()
{
	ABLOG_S(Warning);
	OnAttackHitCheck.Broadcast();
}

// 몽타주에 NextAttackCheck 노티파이가 발생하였을 시 호출 됨
void UABAnimInstance::AnimNotify_NextAttackCheck()
{
	ABLOG_S(Warning);

	// 등록된 모든 
	OnNextAttackCheck.Broadcast();
}

void UABAnimInstance::JumpToAttackMontageSection(int32 NewSection)
{
	ABCHECK(!IsDead);
	ABCHECK(Montage_IsPlaying(AttackMontage));
	Montage_JumpToSection(GetAttackMontageSectionName(NewSection), AttackMontage);
}

FName UABAnimInstance::GetAttackMontageSectionName(int32 Section)
{
	ABLOG_S(Warning);
	
	FName fnTemp;

	if (Section > 1)
	{
		fnTemp = *FString::Printf(TEXT("Attack%d"), Section);
	}

	//FName fnTemp = *FString::Printf(TEXT("Attack%d"), Section);
	ABCHECK(FMath::IsWithinInclusive<int32>(Section, 1, 4), NAME_None);
	return FName(*FString::Printf(TEXT("Attack%d"), Section));
}