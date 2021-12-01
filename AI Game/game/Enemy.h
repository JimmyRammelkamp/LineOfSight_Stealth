#pragma once

class CGuard : public CSprite
{
public:
	enum STATE { NONE, IDLE, PATROL, CHASE };

	float PATROL_SPEED = 40;
	float CHASE_SPEED = 60;

private:
	STATE state;
	CVector enemyPosition;

	CVector aim;
	CVector point1;
	CVector point2;
	bool pointReached;

	float health;

	float time;
	float waitTime;

public:
	CGuard(Sint16 x, Sint16 y,CVector p1, CVector p2, char* pFileBitmap, Uint32 time);
	CGuard(Sint16 x, Sint16 y, CVector p1, CVector p2, CGraphics* pGraphics, Uint32 time);
	~CGuard(void);

	void OnAttacked();
	void OnUpdate(Uint32 time, Uint32 deltaTime);

	void ChangeState(STATE newState);
	STATE GetState() { return state; }

	float GetHealth() { return health; }

	void SetTime(float t) { time = t; }
	float GetWaitTime() { return waitTime; }
	void SetEnemyPosition(CVector v) { enemyPosition = v; }
	void Attacked();
};

typedef std::_gfc_std_ext<std::list<CGuard*>> CGuardList;
