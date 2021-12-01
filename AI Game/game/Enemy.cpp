#include "stdafx.h"
#include "Enemy.h"

CGuard::CGuard(Sint16 x, Sint16 y, CVector p1, CVector p2, char* pFileBitmap, Uint32 time) : CSprite(x, y, 0, 0, time)
{
	LoadAnimation(pFileBitmap, "walkR", CSprite::Sheet(13, 21).Row(9).From(0).To(8));
	LoadAnimation(pFileBitmap, "walkD", CSprite::Sheet(13, 21).Row(10).From(0).To(8));
	LoadAnimation(pFileBitmap, "walkL", CSprite::Sheet(13, 21).Row(11).From(0).To(8));
	LoadAnimation(pFileBitmap, "walkU", CSprite::Sheet(13, 21).Row(12).From(0).To(8));

	health = 100;
	state = NONE;
	
	point1 = p1;
	point2 = p2;
	aim = p2;
	pointReached = false;

	waitTime = 0;

	ChangeState(PATROL);
}

CGuard::CGuard(Sint16 x, Sint16 y, CVector p1, CVector p2, CGraphics* pGraphics, Uint32 time) : CSprite(x, y, 0, 0, time)
{

	LoadAnimation(pGraphics, "walkR", CSprite::Sheet(13, 21).Row(9).From(0).To(8));
	LoadAnimation(pGraphics, "walkD", CSprite::Sheet(13, 21).Row(10).From(0).To(8));
	LoadAnimation(pGraphics, "walkL", CSprite::Sheet(13, 21).Row(11).From(0).To(8));
	LoadAnimation(pGraphics, "walkU", CSprite::Sheet(13, 21).Row(12).From(0).To(8));
	SetAnimation("walkR", 8);

	health = 100;
	state = NONE;

	point1 = p1;
	point2 = p2;
	aim = p2;
	pointReached = false;

	waitTime = 0;

	ChangeState(PATROL);
}

CGuard::~CGuard(void)
{
}

void CGuard::OnAttacked()
{
	health -= 0.5f;
	if (health < 0) health = 0;
}

void CGuard::OnUpdate(Uint32 time, Uint32 deltaTime)
{
	// Transitions
	float enemyDistance = Distance(enemyPosition, GetPosition());
	switch (state)
	{
	case IDLE:
		if (waitTime < time) ChangeState(PATROL);
		break;
	case PATROL:
		if (GetPos().m_x <= (point1.m_x + .5) && GetPos().m_x >= (point1.m_x - .5) && GetPos().m_y <= (point1.m_y + .5) && GetPos().m_y >= (point1.m_y - .5)) ChangeState(IDLE);
		if (GetPos().m_x <= (point2.m_x + .5) && GetPos().m_x >= (point2.m_x - .5) && GetPos().m_y <= (point2.m_y + .5) && GetPos().m_y >= (point2.m_y - .5)) ChangeState(IDLE);

		//if()
		//if (enemyDistance < 200) ChangeState(CHASE);
		//else if (health < 20) ChangeState(IDLE);
		break;
	case CHASE:
		//if (enemyDistance > 250) ChangeState(IDLE);
		//else if (enemyDistance < 50) ChangeState(ATTACK);
		//else if (health < 30) ChangeState(FLEE);
		break;

	}

	// State-dependent actions
	switch (state)
	{
	
	case IDLE:

		SetSpeed(0);

		break;

	case PATROL:
	

		if (GetPos().m_x <= (point1.m_x +.5) && GetPos().m_x >= (point1.m_x - .5) && GetPos().m_y <= (point1.m_y + .5) && GetPos().m_y >= (point1.m_y - .5)) aim = point2;
		if (GetPos().m_x <= (point2.m_x + .5) && GetPos().m_x >= (point2.m_x - .5) && GetPos().m_y <= (point2.m_y + .5) && GetPos().m_y >= (point2.m_y - .5)) aim = point1;

		SetVelocity(Normalise(aim - GetPos()) * 100);
		
		break;

	case CHASE:

		CVector desiredVelocity, steering;

		desiredVelocity = enemyPosition - GetPosition();
		desiredVelocity = Normalise(desiredVelocity) * 120;
		steering = desiredVelocity - GetVelocity();
		Accelerate(steering * deltaTime / 1000.f);

		//SetVelocity(Normalise(enemyPosition - GetPos()) * 100);

		break;
	}

	// Generic behaviour: bounce from the screen margins
	if (GetX() < 32 || GetX() > 1280 - 32) SetVelocity(Reflect(GetVelocity(), CVector(1, 0)));
	if (GetY() < 32 || GetY() > 768 - 32) SetVelocity(Reflect(GetVelocity(), CVector(0, 1)));

	//Animation
	if (GetDirection() <= 45 && GetDirection() >= -45)
		if (GetStatus() != 1)
		{
			SetStatus(1);
			SetAnimation("walkU");
		}

	if (GetDirection() > 45 && GetDirection() <= 135)
		if (GetStatus() != 2)
		{
			SetStatus(2);
			SetAnimation("walkR");
		}
	if (GetDirection() > 135 || GetDirection() < -135)
		if (GetStatus() != 3)
		{
			SetStatus(3);
			SetAnimation("walkD");
		}
	if (GetDirection() >= -135 && GetDirection() < -45)
		if (GetStatus() != 4)
		{
			SetStatus(4);
			SetAnimation("walkL");
		}


	

	CSprite::OnUpdate(time, deltaTime);
}

void CGuard::ChangeState(STATE newState)
{
	if (newState == state)
		return;		// No state change

	state = newState;

	switch (state)
	{
	case IDLE:
		waitTime = time + 1000;
		break;
	case PATROL:
		//SetDirection(rand() % 360);
		//SetSpeed(PATROL_SPEED);
		//SetAnimation("walk");
		break;
	case CHASE:
		//SetDirection(enemyPosition - GetPosition());
		//SetSpeed(CHASE_SPEED);
		//SetAnimation("walk");
		break;
	//case ATTACK:
	//	SetVelocity(0, 0);
	//	SetAnimation("attack");
	//	break;
	//case FLEE:
	//	SetDirection(GetPosition() - enemyPosition);
	//	SetSpeed(CHASE_SPEED);
	//	SetAnimation("walk");
	//	break;
	//case DIE:
	//	SetVelocity(0, 0);
	//	SetAnimation("death");
	//	break;
	}
}
