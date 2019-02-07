// Fill out your copyright notice in the Description page of Project Settings.

#include "ABCharacter.h"
#include "ABAnimInstance.h"
#include "ABCharacterStatComponent.h"
#include "ABWeapon.h"
#include "DrawDebugHelpers.h"
#include "ABCharacterWidget.h"
#include "Components/WidgetComponent.h"
#include "ABAIController.h"


// Sets default values
AABCharacter::AABCharacter()
{
	// 플레이어가 조종하는 폰(캐릭터)를 제외한 나머지는 전부 AI컨트롤의 지배하에 있게만듬
	AIControllerClass = AABAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;


	// 2019-01-21 wssin
	// 디버그 드로잉 관련 셋팅
	AttackRange = 200.0f;
	AttackRadius = 50.0f;

	CharacterStat = CreateDefaultSubobject<UABCharacterStatComponent>(TEXT("CHARACTERSTAT"));

	HPBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBARWIDGET"));

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ArmLengthSpeed = 3.0f;
	ArmRotationnSpeed = 10.0f;

	// 콤보 공격을 위한 멤버 변수 초기화.
	MaxCombo = 4;
	AttackEndComboState();

	IsAttacking = false;

	// 2019-01-13 wssin
	// 점프의 높이 설정.
	GetCharacterMovement()->JumpZVelocity = 400.0f;

	SpringArm	= CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));

	HPBarWidget->SetupAttachment(GetMesh());

	SpringArm->SetupAttachment(GetCapsuleComponent());
	Camera->SetupAttachment(SpringArm);

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f),
	FRotator(0.0f, -90.0f, 0.0f));
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_CARDBOARD(TEXT("/Game/ParagonGreystone/Characters/Heroes/Greystone/Meshes/Greystone.Greystone"));
	if (SK_CARDBOARD.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SK_CARDBOARD.Object);
	}


	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	static ConstructorHelpers::FClassFinder<UAnimInstance> WARRIOR_ANIM(TEXT("/Game/ParagonGreystone/Characters/Heroes/Greystone/Greystone_AnimBlueprint.Greystone_AnimBlueprint_C"));

	//static ConstructorHelpers::FClassFinder<UAnimInstance> WARRIOR_ANIM(TEXT("/Game/ParagonGreystone/Characters/Heroes/Greystone/Animations/Jog_Fwd.Jog_Fwd"));
	if (WARRIOR_ANIM.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(WARRIOR_ANIM.Class);
	}

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("ABCharacter"));

	HPBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 205.0f));
	HPBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	static ConstructorHelpers::FClassFinder<UUserWidget> UI_HUD(TEXT("/Game/Book/UI/UI_HPBar.UI_HPBar_C"));
	
	if (UI_HUD.Succeeded())
	{
		HPBarWidget->SetWidgetClass(UI_HUD.Class);
		HPBarWidget->SetDrawSize(FVector2D(150.0f, 50.0f));
	}

	SetControlMode(EControlMode::GTA);
}

void AABCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//auto AnimInstance = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());

	//AnimInstance->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);

	ABAnim = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());

	// 애님 인스턴스에 등록된 몽타주가 종료되었을 시 발생하는 OnMontageEnded함수에 OnAttackMontageEnded를 등록하여
	// 종료 시 OnAttackMontageEnded 함수를 호출할 수 있도록 함.
	// 델리게이트
	// C언어의 콜백과 비슷해 보인다.
	ABAnim->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);

	ABAnim->OnNextAttackCheck.AddLambda([this]() -> void {
		ABLOG(Warning, TEXT("OnNextAttackCheck"));
		CanNextCombo = false;

		if (IsComboInputOn)
		{
			AttackStartComboState();
			ABAnim->JumpToAttackMontageSection(CurrentCombo);
		}
	});

	// UObject 기반 멤버 함수 델리게이트를 추가합니다. UObject 델리게이트는 자신의 오브젝트에 대한 약 레퍼런스를 유지합니다.
	// 델리게이트에 함수를 반잉딩시켜서 델리게이트가 발동되었을때 호출할 수 있도록..
	// 어택 노티파이가 발생하였을때 BroadCast하여 등록된 함수 호출
	ABAnim->OnAttackHitCheck.AddUObject(this, &AABCharacter::AttackCheck);

	CharacterStat->OnHPIsZero.AddLambda([this]() -> void {
		ABLOG(Warning, TEXT("OnHPIsZero"));
		ABAnim->SetDeadAnim();
		SetActorEnableCollision(false);

	});

	//ABAnim->OnCharacterJogStartCheck.AddLambda([this]() -> void {
	//	ABLOG(Warning, TEXT("OnHPIsZero"));
	//	ABAnim->SetDeadAnim();
	//	SetActorEnableCollision(false);

	//});

}


// Called when the game starts or when spawned
void AABCharacter::BeginPlay()
{
	Super::BeginPlay();
	//
	//FName WeaponeSocket(TEXT("sword_Socket"));

	//// 액터를 월드에 생성하고 그걸 현재의 무기로 설정
	//auto CurWeapon = GetWorld()->SpawnActor<AABWeapon>(FVector::ZeroVector, FRotator::ZeroRotator);
	//if (nullptr != CurWeapon)
	//{
	//	CurWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponeSocket);
	//}

	auto CharacterWidget = Cast<UABCharacterWidget>(HPBarWidget->GetUserWidgetObject());
	if (nullptr != CharacterWidget)
	{
		CharacterWidget->BindCharacterStat(CharacterStat);
	}
}

// Called every frame
void AABCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, ArmLengthTo, DeltaTime, ArmLengthSpeed);

	switch (enumCurrentConrolMode)
	{
	case EControlMode::DIABLO:
		SpringArm->RelativeRotation = FMath::RInterpTo(SpringArm->RelativeRotation, ArmRotationTo, DeltaTime, ArmRotationnSpeed);
		break;
	
	default:
		break;
	}


	switch (enumCurrentConrolMode)
	{
	case EControlMode::DIABLO :
		if (vecDirectionToMove.SizeSquared() > 0.0f)
		{
			GetController()->SetControlRotation(FRotationMatrix::MakeFromX(vecDirectionToMove).Rotator());
			AddMovementInput(vecDirectionToMove);
		}
	break;

	default:
		break;
	}

}

// Called to bind functionality to input
void AABCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("UpDown"), this, &AABCharacter::UpDown);
	PlayerInputComponent->BindAxis(TEXT("LeftRight"), this, &AABCharacter::LeftRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AABCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AABCharacter::Turn);
	PlayerInputComponent->BindAction(TEXT("ViewChange"), EInputEvent::IE_Pressed, this, &AABCharacter::ViewChange);
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Attack"), EInputEvent::IE_Pressed, this, &AABCharacter::Attack);


}

void AABCharacter::UpDown(float NewAxisValue)
{
	/*AddMovementInput(GetActorForwardVector(), NewAxisValue);*/

	//AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X), NewAxisValue);

	switch (enumCurrentConrolMode)
	{
	case EControlMode::DIABLO :
		vecDirectionToMove.X = NewAxisValue;
		break;
	case EControlMode::GTA :
		AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X), NewAxisValue);
		break;
	default:
		break;
	}
}

void AABCharacter::LeftRight(float NewAxisValue)
{
	//AddMovementInput(GetActorRightVector(), NewAxisValue);

	switch (enumCurrentConrolMode)
	{
	case EControlMode::DIABLO:
		vecDirectionToMove.Y = NewAxisValue;
		break;
	case EControlMode::GTA:
		AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::Y), NewAxisValue);
		break;
	default:
		break;
	}
}

void AABCharacter::LookUp(float NewAxisValue)
{
	switch (enumCurrentConrolMode)
	{
	case EControlMode::GTA:
		AddControllerPitchInput(NewAxisValue);
		break;
	default:
		break;
	}
}

void AABCharacter::Turn(float NewAxisValue)
{
	switch (enumCurrentConrolMode)
	{
	case EControlMode::GTA:
		AddControllerYawInput(NewAxisValue);
		break;
	default:
		break;
	}

}


void AABCharacter::SetControlMode(EControlMode NewCotnrolMode)
{
	enumCurrentConrolMode = NewCotnrolMode;

	switch (enumCurrentConrolMode)
	{
	case AABCharacter::EControlMode::GTA:
		// 2019-01-09 wssin
			// SpringArm = 삼인친 시점 편리하게 구성하게 도와주는 컴포넌트,
		//	// TargetArmLength  = 카메라 거치대 위치
		//SpringArm->TargetArmLength = 450.0f; // 4.5 미터
		//SpringArm->SetRelativeRotation(FRotator::ZeroRotator);

		// 시점변경 보간이동을 위한 변수
		ArmLengthTo = 450.0f;

		SpringArm->bUsePawnControlRotation = true;

		// bInherit~ = true 일 경우 부모 컨트롤러 부터 축을 상속 받는다.
		SpringArm->bInheritPitch = true;
		SpringArm->bInheritRoll = true;
		SpringArm->bInheritYaw = true;
		SpringArm->bDoCollisionTest = true;

		// 컨트롤 회전의 Yaw축과 Pawn의 Yaw과 연동시켜줄지 확인하는 기능 true면 연동
		// 예를들어 이걸 false로 지정하면 카메라가 좌우로 움직여도 카메라 시점만 이동하고
		// 캐릭터는 그 방향으로 돌아가지 않음.
		bUseControllerRotationYaw = false;

		// 캐릭터가 움직이는 방향으로 캐릭터를 자동으로 회전시켜주는 기능
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 300.0f, 0.0f); // Character 회전 속도를 300.0f 단위가 어떻게 되는건지 모르겠네
		break;
	case AABCharacter::EControlMode::DIABLO:
		/*SpringArm->TargetArmLength = 800.0f;
		SpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
		*/

		// 시점변경 보간이동을 위한 변수
		ArmLengthTo = 800.0f;
		ArmRotationTo = FRotator(-45.0f, 0.0f, 0.0f);

		SpringArm->bUsePawnControlRotation = false;
		SpringArm->bInheritPitch = false;
		SpringArm->bInheritRoll = false;
		SpringArm->bInheritYaw = false;
		SpringArm->bDoCollisionTest = false;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 300.0f, 0.0f); // Character 회전 속도를 300.0f 단위가 어떻게 되는건지 모르겠네
		break;

	case EControlMode::NPC:
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 480.0f, 0.0f);
		break;

	default:
		break;
	}
}

void AABCharacter::ViewChange()
{
	switch (enumCurrentConrolMode)
	{
	case EControlMode::GTA:
		GetController()->SetControlRotation(GetActorRotation());
		SetControlMode(EControlMode::DIABLO);
		break;

	case EControlMode::DIABLO:
		GetController()->SetControlRotation(SpringArm->RelativeRotation);
		SetControlMode(EControlMode::GTA);
		break;

	default:
		break;
	}
}

void AABCharacter::Attack()
{
	//if (IsAttacking)
	//{
	//	return;
	//}

	//auto AnimInstance = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());
	//if (AnimInstance == nullptr)
	//{
	//	return;
	//}

	//ABAnim->PlayAttackMontage();
	//IsAttacking = true;


	if (IsAttacking)
	{
		ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 1, MaxCombo));
		if (CanNextCombo)
		{
			IsComboInputOn = true;
		}
	}
	else
	{
		ABCHECK(CurrentCombo == 0);
		AttackStartComboState();
		ABAnim->PlayAttackMontage();
		ABAnim->JumpToAttackMontageSection(CurrentCombo);
		IsAttacking = true;
	}
}

void AABCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	ABCHECK(IsAttacking);
	ABCHECK(CurrentCombo > 0);
	IsAttacking = false;
	AttackEndComboState();
}

void AABCharacter::AttackStartComboState()
{
	CanNextCombo = true;
	IsComboInputOn = false;
	ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 0, MaxCombo - 1));
	CurrentCombo = FMath::Clamp<int32>(CurrentCombo + 1, 1, MaxCombo);
}

void AABCharacter::AttackEndComboState()
{
	IsComboInputOn = false;
	CanNextCombo = false;
	CurrentCombo = 0;
}

void  AABCharacter::AttackCheck()
{
	// 물리적 충돌이 탐지된 경우 관련된 정보를 담을 구조체
	FHitResult HitResult;

	// 탐색 방법에 대한 설정 값을 모아둔 구조체
	
	//struct ENGINE_API FCollisionObjectQueryParams
	//{
	//	enum InitType
	//	{
	//		AllObjects,
	//		AllStaticObjects,
	//		AllDynamicObjects
	//	};


	FCollisionQueryParams Params(NAME_None, false, this);
	
	// 트레이스 채널을 사용해 물리적 충돌 여부를 가리는 함수
	bool bResult = GetWorld()->SweepSingleByChannel(
		HitResult, // 물리적 충돌이 탐지된 경우 관련된 정보를 담을 구조체
		GetActorLocation(), // 탐색을 시작할 위치 액터가 있는 곳
		GetActorLocation() + GetActorForwardVector() * AttackRange, // 탐색을 끝낼 위치 엑터의 시선방향으로부터 200cm 떨어진 곳
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2, // 트레이스 채널에 추가한 Attack이 채널 2번이다
		FCollisionShape::MakeSphere(AttackRadius), // 탐색에 사용할 도형을 만든다. 반지름 50cm 구
		Params // 탐색 방법을 설정하는 파라미터, 공격 명령을 내리는 자신은 이 탐색에 감지되지 않도록 포인터 this를 무시할 액터 목록에 넣어줘야 한다.
	);

// 디버그 시에만 디버그 드로우를 사용하려고
#if ENABLE_DRAW_DEBUG

	//FVector TraceVec	= 캐릭터가 보고있는 방향의 벡터 * 확인하고자하는 길이 200cm
	FVector TraceVec	= GetActorForwardVector() * AttackRange;

	// Center			= 캐릭터의 위치로부터 확인하고자하는 벡터의 중간지점을 찾는 식
	FVector Center		= GetActorLocation() + TraceVec * 0.5f;

	// 캡슐의 중간 높이(길이) 150 cm
	float HalfHeight	= AttackRange * 0.5f + AttackRadius;
	
	// 캡슐 눕히기 위한 회전행렬
	FQuat CapsuleRot	= FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	FColor DrawColor	= bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 5.0f;

	DrawDebugCapsule(GetWorld(),
		Center,
		HalfHeight,
		AttackRadius,
		CapsuleRot,
		DrawColor,
		false,
		DebugLifeTime);

#endif
	 

	if (bResult)
	{
		if (HitResult.Actor.IsValid())
		{
			// 충돌된 액터 정보 출력
			ABLOG(Warning, TEXT("Hit Actor Name : %s"), *HitResult.Actor->GetName());

			// 2019-01-26 wwshin
			// 
			FDamageEvent DamageEvent;
			HitResult.Actor->TakeDamage(CharacterStat->GetAttack(), DamageEvent, GetController(), this);
		}
	}
}



float AABCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	ABLOG(Warning, TEXT("Actor : %s took Damage : %f"), *GetName(), FinalDamage);
	
	//if (FinalDamage > 0.0f)
	//{
	//	ABAnim->SetDeadAnim();
	//	SetActorEnableCollision(false);
	//}

	CharacterStat->SetDamage(FinalDamage);

	return FinalDamage;
}


bool AABCharacter::CanSetWeapon()
{
	return (nullptr == CurrentWeapon);
}

void AABCharacter::SetWeapon(class AABWeapon* NewWeapon)
{
	ABCHECK(nullptr != NewWeapon && nullptr == CurrentWeapon);
	FName WeaponSocket(TEXT("sword_Socket"));
	if (nullptr != NewWeapon)
	{
		NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
		NewWeapon->SetOwner(this);
		CurrentWeapon = NewWeapon;
	}
}

void AABCharacter::PossessedBy(AController * NewController)
{
	Super::PossessedBy(NewController);

	if (IsPlayerControlled())
	{
		SetControlMode(EControlMode::DIABLO);
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	}
	else
	{
		SetControlMode(EControlMode::NPC);
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	}
}

