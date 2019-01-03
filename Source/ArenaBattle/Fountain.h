// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "CoreMinimal.h"
#include "ArenaBattle.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Fountain.generated.h"

UCLASS() // 2019-01-02 WSSIN 언리얼 오브젝트 클래스 선언 매크로


// 2019-01-02 WSSIN "모듈명_API" 키워드는 DLL 내 클래스 정보를 외부에 공개할지 결정하는 DLLEXPORT 키워드 와 같다.
// 이것이 없으면 다른 모듈에서 해당 객체에 접근할 수 없다.
class ARENABATTLE_API AFountain : public AActor
{
	GENERATED_BODY() // 2019-01-02 WSSIN 언리얼 오브젝트 클래스 선언 매크로
	
public:	
	// Sets default values for this actor's properties
	AFountain(); // 2019-01-02 WSSIN 언리얼 오브젝트 클래스 이름 접두사 A = 액터 U = 액터를 제외한 나머지

protected:
	// Called when the game starts or when spawned

	// 2019-01-03 wssin
	// 액터의 이벤트 호출 순서
	// 1. PostInitializeComponents : 액터에 속한 모든 컴포넌트의 세팅이 완료되면 언리얼 엔진은 액터의 PostInitializeComponents 함수를 호출한다.
	// 2. BeginPlay : 액터가 게임에 참여할 때 (배우 입장~) 실행
	// 3. Tick : 액터가 무대에 올라가 있을 때 매 프레임마다 호출
	// 4. EndPlay : 액터가 무대에서 퇴장할 때 호출

	// C++에서 override 키워드를 사용하여 컴파일러에게 상속받을 것을 알려주는 것이 좋다.
	// 가상 함수를 상속받아 구현할 경우에는!
	
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 2019-01-02 WSSIN
	// KOR : 언리얼 실행 환경이 선언한 객체를 자동으로 관리하게 만들려는 키워드.
	// 원래 C++에서는 직접 메모리 해제해줘야 하는대 이렇게하면 언리얼이 알아서 해줌.
	// 크~ 편리하구만
	// 의문1) 어? 이러면 스마트 포인터 사용할 필요가 없어지나?
	// 의문 확인 : 언리얼 오브젝트 객체들만이 자동관리 가능.
	// 내가 직접만들어낸 객체를 안될 듯?
	
	// VisibleAnyWhere	= 객체를 볼 수 있지만 해당 객체를 다른 객체로 변경할 수는 없다.
	// EditAnyWhere		= 속성의 데이터를 변경하게 해준다.
	// Category			= 분류(일종의 그룹)을 지정할 수 있다.

	// 분수대 뼈대
	UPROPERTY(VisibleAnyWhere)
	UStaticMeshComponent* Body;

	// 분수대 물
	UPROPERTY(VisibleAnyWhere)
	UStaticMeshComponent* Water;

	// 분수대 조명효과
	UPROPERTY(VisibleAnyWhere)
	UPointLightComponent* Light;

	// 분수대 물 출렁임 효과
	UPROPERTY(VisibleAnyWhere)
	UParticleSystemComponent* Splash;;
		
	UPROPERTY(EditAnyWhere, Category=ID)
	int32 ID;

	// 2019-01-03 wssin
	// 정보 은닉을 위하여 private으로 선언하고,
	// private을 EditAnyWhere로 사용할 시 컴파일 에러가 발생하니 이것을 방지하기 위하여
	// AllowPrivateAccess라는 META 키워드를 추가하면 에디터에서 이를 편집함과 동시에
	// 변수 데이터를 은닉할 수 있게 돼 프로그래밍 영역에서 캡슐화가 가능해진다.
 private:
	UPROPERTY(EditAnyWhere, Category = Stat, Meta = (AllowPrivateAccess = true))
	float RotateSpeed;

	UPROPERTY(VisibleAnyWhere)
	URotatingMovementComponent* Movement;
	
};
