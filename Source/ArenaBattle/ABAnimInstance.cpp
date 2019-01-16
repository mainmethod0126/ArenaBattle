// Fill out your copyright notice in the Description page of Project Settings.

#include "ABAnimInstance.h"

UABAnimInstance::UABAnimInstance()
{
	CurrentPawnSpeed = 0.0f;
	
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ATTACK_MONTAGE(TEXT("/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations/Greystone_Skeleton_Montage.Greystone_Skeleton_Montage"));

	if (ATTACK_MONTAGE.Succeeded())
	{
		AttackMontage = ATTACK_MONTAGE.Object;
	}
}

void UABAnimInstance::PlayAttackMontage()
{
	Montage_Play(AttackMontage, 1.0f);
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