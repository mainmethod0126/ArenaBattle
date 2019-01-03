// Fill out your copyright notice in the Description page of Project Settings.

#include "Fountain.h"

// Sets default values
AFountain::AFountain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 2019-01-02 WSSIN 언리얼에서는 생성자에서 컴포넌트를 생성할 때 new 대신 CreateDefaultSubobject 씀.
	Body	= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BODY"));
	Water	= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WATER"));
	Light	= CreateDefaultSubobject<UPointLightComponent>(TEXT("LIGHT"));
	Splash	= CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SPLASH"));
	Movement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("MOVEMENT"));

	// 2019-01-02 WSSIN 액터는 루트컴포넌트를 무조건 지정해야한다!! 영어 문장에서 주어없는 문장은 없는 것 처럼 오케이?
	// 루트 지정 RootComponent = "생성된 컴포넌트 객체"
	// Water객체를 Body의 자식이 되도록 선언.
	RootComponent = Body;
	Water->SetupAttachment(Body);
	Light->SetupAttachment(Body);
	Splash->SetupAttachment(Body);

	// 액터가 생성될때 기본 위치를 지정해주는 키워드 
	// F 접두어는 언리얼 오브젝트와 관련 없는 일반 C++ 클래스 혹은 구조체를 의미함.
	Water->SetRelativeLocation(FVector(0.0f, 0.0f, 135.0f));
	Light->SetRelativeLocation(FVector(0.0f, 0.0f, 195.0f));
	Splash->SetRelativeLocation(FVector(0.0f, 0.0f, 195.0f));

	// 2019-01-02 WSSIN
	// 에셋의 키값으로 경로를 쓴다.
	// {오브젝트 타입}'{폴더명}/{파일명}.{애셋명}'
	// 오브젝트 타입	: 애셋의 타입을 명시적으로 지정한다.
	// 폴더명/파일명	: 물리적인 딧그크에 위치한 애셋의 경로 정보를 의미한다. 경로 정보는 다른 애셋과 중복될 수 없고 유일해야 한다.
	// 애셋명			: 에디터에서 보여지는 애셋의 이름을 의미한다. 애셋의 이름은 중복될 수 있다.


	// 변수 SM_BDOY 선언
	//  PATH가 너무 길어서 아래와 같이 문자열을 잘라서 만듬.
	// Body 애셋 셋팅
	static FString strBodyPath = TEXT("/Game/InfinityBladeGrassLands/Environments/");
	strBodyPath.Append(TEXT("Plains/Env_Plains_Ruins/StaticMesh/"));
	strBodyPath.Append(TEXT("SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_BODY(*strBodyPath); // FString -> TCHAR*

	if (SM_BODY.Succeeded())
	{
		Body->SetStaticMesh(SM_BODY.Object);
	}

	// Water 애셋 셋팅 static 의 이유는 게임 도중에 애셋이 변경될 일이 없기때문에
	static FString strWaterPath = TEXT("/Game/InfinityBladeGrassLands/Effects/");
	strWaterPath.Append(TEXT("FX_Meshes/Env/SM_Plains_Fountain_02.SM_Plains_Fountain_02"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SM_WATER(*strWaterPath); // FString -> TCHAR*

	if (SM_WATER.Succeeded())
	{
		Water->SetStaticMesh(SM_WATER.Object);
	}

	RotateSpeed = 30.0f;
	Movement->RotationRate = FRotator(0.0f, RotateSpeed, 0.0f);
	

}

void AFountain::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ABLOG_S(Warning);
}


// Called when the game starts or when spawned
void AFountain::BeginPlay()
{
	Super::BeginPlay();
	
	ABLOG_S(Warning);
	ABLOG(Warning, TEXT("Actor Name : %s, ID : %d, Location X : %.3f"), *GetName(), ID, GetActorLocation().X);
	//(ArenaBattle, Warning, TEXT("Actor Name : %s, ID : %d, Location X : %.3f"), *GetName(), ID, GetActorLocation().X);

}

// Called every frame
void AFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// FRotator(Pitch, Yaw, Roll)
	// Pitch : 좌우를 기준으로 돌아가는 회전이다. 언리얼 엔진에서는 Y축 회전을 표현할 때 사용한다.
	// Yaw : 상하를 기준으로 돌아가는 회전이다. 언리얼 엔진에서는 Z축 회전을 표현할 때 사용한다.
	// Roll : 정면을 기준으로 돌아가는 회전이다. 언리얼 엔진에서는 X축 회전을 표현할 때 사용한다.

	// DeltaTime을 사용해 초당 일정한 속도로 분수대를 회전시키는 코드
	/*AddActorLocalRotation(FRotator(0.0f, RotateSpeed * DeltaTime, 0.0f));
*/
}


void AFountain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ABLOG_S(Warning);
}


