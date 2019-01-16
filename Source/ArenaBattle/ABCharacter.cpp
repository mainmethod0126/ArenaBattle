// Fill out your copyright notice in the Description page of Project Settings.

#include "ABCharacter.h"
#include "ABAnimInstance.h"

// Sets default values
AABCharacter::AABCharacter()
{
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
}


// Called when the game starts or when spawned
void AABCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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