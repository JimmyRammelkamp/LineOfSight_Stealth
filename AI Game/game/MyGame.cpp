#include "stdafx.h"
#include "MyGame.h"

#pragma warning (disable: 4244)

#define SPEED_PLAYER	100

#define SPEED_GUARD_0	120
#define SPEED_GUARD_1	80
#define SPEED_GUARD_2	80

char* CMyGame::m_tileLayout[12] =
{
	"XXXXXXXXXXXXXXXXXXXX",
	"X                  X",
	"X XXXX       XXXXX X",
	"X XXXX       XXXXX X",
	"X XXXX   XX        X",
	"  XXX    XX  XXXXXXX",
	"X XXXX   XX  XXXXXXX",
	"X XXXX       XXXXXXX",
	"X XXXX XX XX XXXXX X",
	"X XXXX XX XX   XXX X",
	"X                  X",
	"XXXXXXXXXXXXXXXXXXXX",
};

CMyGame::CMyGame(void) :
	m_player(0, 0, "boy.png", 0), key(18 * 64 + 32, 8 * 64 + 32, "key.png",0), goal(32, 5 * 64 + 32, 64, 64, CColor::Green(),0), closedGoal(32, 5 * 64 + 32, 64, 64, CColor::Red(), 0), cover(5 * 64 + 32, 5 * 64 + 32, 64, 64, CColor::LightRed(), 0)
{

}

CMyGame::~CMyGame(void)
{
}

// MATHS! Intersection function
// Provides information about the interception point between the line segments: a-b  and c-d
// Returns true if the lines intersect (not necessarily  within the segments); false if they are parallel
// k1 (returned value): position of the intersection point along a-b direction:
//                      k1==0: at the point a; 0>k1>1: between a and b; k1==1: at b; k1<0 beyond a; k1>1: beyond b
// k2 (returned value): position of the intersection point along c-d direction
//                      k2==0: at the point c; 0>k2>1: between c and d; k2==1: at d; k2<0 beyond c; k2>1: beyond d
// Intersection point can be found as X = a + k1 * (b - a) = c + k2 (d - c)
bool Intersection(CVector a, CVector b, CVector c, CVector d, float &k1, float &k2)
{
	CVector v1 = b - a;
	CVector v2 = d - c;
	CVector con = c - a;
	float det = v1.m_x * v2.m_y - v1.m_y * v2.m_x;
	if (det != 0)
	{
		k1 = (v2.m_y * con.m_x - v2.m_x * con.m_y) / det;
		k2 = (v1.m_y * con.m_x - v1.m_x * con.m_y) / det;
		return true;	
	}
	else
		return false;
}

// returns true is the line segments a-b and c-d intersect
bool Intersection(CVector a, CVector b, CVector c, CVector d)
{
	float k1, k2;
	if (!Intersection(a, b, c, d, k1, k2))
		return false;
	return k1 >= 0 && k1 <= 1.f && k2 >= 0 && k2 <= 1.f;
}


/////////////////////////////////////////////////////
// Per-Frame Callback Funtions (must be implemented!)

void CMyGame::OnUpdate()
{
	if (!IsGameMode()) return;

	Uint32 t = GetTime();

	// Player Control:a
	// The Player movement is contrained to the tile system.
	// Whilst moving, the player always heads towards the destination point at the centre of one of the neighbouring tiles - stored as m_dest.
	// Player control only activates when player either motionless or passed across the destination point (dest)
	if (m_player.GetSpeed() < 0.1 || m_player.GetSpeed() > 0.1 && Dot(m_dest - m_player.GetPosition(), m_player.GetVelocity()) < 0)
	{
		CVector newDir(0, 0);		// new direction - according to the keyboard input
		char *newAnim = "idle";		// new animation - according to the keyboard input
		
		if (IsKeyDown(SDLK_a))
		{
			newDir = CVector(-1, 0);
			newAnim = "walkL";
		}
		else if (IsKeyDown(SDLK_d))
		{
			newDir = CVector(1, 0);
			newAnim = "walkR";
		}
		else if (IsKeyDown(SDLK_w))
		{
			newDir = CVector(0, 1);
			newAnim = "walkU";
		}
		else if (IsKeyDown(SDLK_s))
		{
			newDir = CVector(0, -1);
			newAnim = "walkD";
		}

		// Collision test of the new heading point
		CVector newDest = m_dest + 64 * newDir;
		for (CSprite* pTile : m_tiles)
			if (pTile->HitTest(newDest))
				newDest = m_dest;	// no change of destination in case of a collision with a tile
		m_dest = newDest;

		// Set new velocity and new animation only if new direction different than current direction (dot product test)
		if (Dot(m_player.GetVelocity(), newDir) < 0.1)
		{
			m_player.SetVelocity(100 * newDir);
			m_player.SetAnimation(newAnim);
		}

		/*if (IsKeyDown(SDLK_RSHIFT))
			m_player.SetSpeed(150);*/

		/*if (!IsKeyDown(SDLK_RSHIFT))
			m_player.SetSpeed(100);*/

		// a little bit of trickery to ensure the player is always aligned to the tiles
		m_player.SetPosition(64 * floorf(m_player.GetPosition().m_x / 64) + 32, 64 * floorf(m_player.GetPosition().m_y / 64) + 32);
	}
	m_player.Update(t);

	if (m_player.GetPos() == CVector(5 * 64 + 32, 5 * 64 + 32)) hiding = true;
	if (m_player.GetPos() != CVector(5 * 64 + 32, 5 * 64 + 32)) hiding = false;
	if (m_player.HitTest(&key)) locked = false;

	for (CGuard* pGuard : m_guards)
	{
		pGuard->Update(GetTime());
		pGuard->SetEnemyPosition(m_player.GetPos());
		pGuard->SetTime(GetTime());
	}
		


	// LINE OF SIGHT TEST
	for (CGuard* pGuard : m_guards)
	{

		bool spotted = true;

		// browse through all tiles - if line of sight test shows any tile to obscure the player, then we have no killer after all
		for (CSprite* pTile : m_tiles)
		{
			// Check intersection of the "Guard - Player" sight line with both diagonals of the tile.
			// If there is intersection - there is no killer - so, m_pKiller = NULL;

			// TO DO:
			// Call the Intersection function twice, once for each diagonal of the tile
			// If the function returns true in any case, call the following:
			CVector bottomright(pTile->GetTopRight().m_x, pTile->GetBottomLeft().m_y);
			CVector topleft( pTile->GetBottomLeft().m_x, pTile->GetTopRight().m_y);
			
			if (Intersection(pGuard->GetPos(),m_player.GetPos(),pTile->GetBottomLeft(),pTile->GetTopRight()))
				spotted = false;
			if (Intersection(pGuard->GetPos(),m_player.GetPos(),bottomright,topleft))
				spotted = false;
				
			if (spotted == false)
			{
				if(pGuard->GetState() == CGuard::CHASE)
					pGuard->ChangeState(CGuard::PATROL);
				break;	// small optimisation, if line of sight test already failed, no point to look further
			}
				
		}

		// if the player is in plain sight of the guard...
		if (spotted)
		{
			// Additional test - only killing if the player within 60 degrees from the guard's front (they have no eyes in the back of their head)
			CVector v = m_player.GetPosition() - pGuard->GetPosition();
			
			// TO DO: Calculate the Dot Product of the displacement vector (v - calculated above) and the guard's velocity vector.
			// Normalise both vectors for the dot!
			// If the result is greater than 0.5, the player is within 60 degrees from the front of the guard.
			// Otherwise, the guard should not see the player (again, m_pKiller = NULL)
			if (Dot(Normalise(v), Normalise(pGuard->GetVelocity())) <= 0.5)
				spotted = false;
			if (hiding) spotted = false;

		}


		
		// if still the killer found - the game is over and look no more!
		if (spotted)
		{
			pGuard->ChangeState(CGuard::CHASE);

			if (m_player.HitTest(pGuard))
			{
				GameOver();
				return;
			}
			return;
		}
	}
	if (GetTime() / 1000 == 60)
	{
		tooSlow = true;
		GameOver();

	}
	// WINNING TEST
	if (m_player.GetLeft() < 0 && locked == false)
		GameOver();
}

void CMyGame::OnDraw(CGraphics* g)
{
	m_tiles.for_each(&CSprite::Draw, g);
	if (locked == false) goal.Draw(g);
	if (locked == true) closedGoal.Draw(g);
	cover.Draw(g);
	if (locked == true) key.Draw(g);
	m_player.Draw(g);

	for (CGuard* pGuard : m_guards)
		pGuard->Draw(g);

	for (CGuard* pGuard : m_guards)
	{
		if (pGuard->GetState() == CGuard::CHASE)
		{
			*g << font(14) << color(CColor::Black()) << xy(pGuard->GetPos().m_x - 20, pGuard->GetPos().m_y + 20) << "GET THEM!";
			g->DrawLine(pGuard->GetPosition(), m_player.GetPosition(), 4, CColor::Red());
		}
			
		if (pGuard->GetState() == CGuard::PATROL)
			*g << font(14) << color(CColor::Black()) << xy(pGuard->GetPos().m_x - 20, pGuard->GetPos().m_y + 20) << "All Clear";
		if (pGuard->GetState() == CGuard::IDLE)
			*g << font(14) << color(CColor::Black()) << xy(pGuard->GetPos().m_x - 20, pGuard->GetPos().m_y + 20) << "All Clear";
		
		*g << font(14) << color(CColor::Red()) << xy(pGuard->GetPos().m_x, pGuard->GetPos().m_y + 40) << pGuard->GetStatus();
		*g << font(14) << color(CColor::Red()) << xy(pGuard->GetPos().m_x, pGuard->GetPos().m_y + 60) << pGuard->GetWaitTime();
		*g << font(14) << color(CColor::Red()) << xy(pGuard->GetPos().m_x, pGuard->GetPos().m_y + 80) << pGuard->GetSpeed();

		if (IsGameOver())
		{
			if (m_player.GetLeft() < 0)
				*g << font(48) << color(CColor::DarkBlue()) << vcenter << center << "MISSION ACCOMPLISHED" << endl;
			else if (m_player.HitTest(pGuard))
				*g << font(48) << color(CColor::Red()) << vcenter << center << "WASTED!" << endl;
			
			else if(tooSlow == true)
				*g << font(48) << color(CColor::Red()) << vcenter << center << "TOO SLOW" << endl;
		}
			

	}


	*g << font(14) << color(CColor::Red()) << xy(20, 20) << "Playerx: " << m_player.GetPosition().m_x;
	*g << font(14) << color(CColor::Red()) << xy(20, 40) << "Playery: " << m_player.GetPosition().m_y;
	*g << font(14) << color(CColor::Red()) << xy(20, 60) << "PlayerSpeed: " << m_player.GetSpeed(); 
	*g << font(14) << color(CColor::Red()) << xy(20, 80) << "PlayerSpeed: " << GetTime(); 
	*g << font(14) << color(CColor::Red()) << xy (0,750) << center  << "Time: " << 60 - (GetTime()/1000) ;
}

/////////////////////////////////////////////////////
// Game Life Cycle

// one time initialisation
void CMyGame::OnInitialize()
{
	// Create Tiles
	for (int y = 0; y < 12; y++)
		for (int x = 0; x < 20; x++)
		{
			if (m_tileLayout[y][x] == ' ')
				continue;

			int nTile = 5;
			if (y > 0 && m_tileLayout[y - 1][x] == ' ') nTile -= 3;
			if (y < 11 && m_tileLayout[y + 1][x] == ' ') nTile += 3;
			if (x > 0 && m_tileLayout[y][x - 1] == ' ') nTile--;
			if (x < 20 && m_tileLayout[y][x + 1] == ' ') nTile++;
			if (nTile == 5 && x > 0 && y > 0 && m_tileLayout[y - 1][x - 1] == ' ') nTile = 14;
			if (nTile == 5 && x < 20 && y > 0 && m_tileLayout[y - 1][x + 1] == ' ') nTile = 13;
			if (nTile == 5 && x > 0 && y < 11 && m_tileLayout[y + 1][x - 1] == ' ') nTile = 11;
			if (nTile == 5 && x < 20 && y < 11 && m_tileLayout[y + 1][x + 1] == ' ') nTile = 10;
			
			nTile--;
			m_tiles.push_back(new CSprite(x * 64.f + 32.f, y * 64.f + 32.f, new CGraphics("tiles.png", 3, 5, nTile % 3, nTile / 3), CColor::White(), 0));
		}

	// Prepare the Player for the first use
	m_player.LoadAnimation("player.png", "walkR", CSprite::Sheet(13, 21).Row(9).From(0).To(8));
	m_player.LoadAnimation("player.png", "walkD", CSprite::Sheet(13, 21).Row(10).From(0).To(8));
	m_player.LoadAnimation("player.png", "walkL", CSprite::Sheet(13, 21).Row(11).From(0).To(8));
	m_player.LoadAnimation("player.png", "walkU", CSprite::Sheet(13, 21).Row(12).From(0).To(8));
	m_player.LoadAnimation("player.png", "idle",  CSprite::Sheet(13, 21).Row(13).From(0).To(0));  //  CSprite::Sheet(13, 21).Tile(0, 13));
}

// called when a new game is requested (e.g. when F2 pressed)
// use this function to prepare a menu or a welcome screen
void CMyGame::OnDisplayMenu()
{
	StartGame();	// exits the menu mode and starts the game mode
}

// called when a new game is started
// as a second phase after a menu or a welcome screen
void CMyGame::OnStartGame()
{
	m_guards.delete_all();

	// Reinitialise the player
	m_player.SetPosition(64 * 18 + 32, 64 + 32);
	m_player.SetVelocity(0, 0);
	m_player.SetAnimation("idle");
	m_dest = m_player.GetPosition();
	hiding = false;
	locked = true;
	tooSlow = false;

	//Guard Patrol Pointsa
	CVector g1p1 = CVector(64 * 9 + 32, 64 * 1 + 32);
	CVector g1p2 = CVector(64 * 1 + 32, 64 * 1 + 32);

	CVector g2p1 = CVector(64 * 6 + 32, 64 * 2 + 32);
	CVector g2p2 = CVector(64 * 6 + 32, 64 * 9 + 32);

	CVector g3p1 = CVector(64 * 1 + 32, 64 * 10 + 32);
	CVector g3p2 = CVector(64 * 18 + 32, 64 * 10 + 32);

	m_guards.push_back(new CGuard(64 * 17 + 32, 64 * 1 + 32, g1p1, g1p2, "guard.png", 0));
	m_guards.push_back(new CGuard(64 * 6 + 32, 64 * 3 + 32, g2p1,g2p2, "guard.png", 0));
	m_guards.push_back(new CGuard(64 * 2 + 32, 64 * 10 + 32,g3p1, g3p2, "guard.png", 0));

}

// called when a new level started - first call for nLevel = 1
void CMyGame::OnStartLevel(Sint16 nLevel)
{
}

// called when the game is over
void CMyGame::OnGameOver()
{
}

// one time termination code
void CMyGame::OnTerminate()
{
}

/////////////////////////////////////////////////////
// Keyboard Event Handlers

void CMyGame::OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode)
{
	if (sym == SDLK_F4 && (mod & (KMOD_LALT | KMOD_RALT)))
		StopGame();
	if (sym == SDLK_SPACE)
		PauseGame();
	if (sym == SDLK_F2)
		NewGame();


}

void CMyGame::OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode)
{
}


/////////////////////////////////////////////////////
// Mouse Events Handlers

void CMyGame::OnMouseMove(Uint16 x,Uint16 y,Sint16 relx,Sint16 rely,bool bLeft,bool bRight,bool bMiddle)
{
}

void CMyGame::OnLButtonDown(Uint16 x, Uint16 y)
{
	CVector v(x, y);	// destination
}

void CMyGame::OnLButtonUp(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonUp(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonUp(Uint16 x,Uint16 y)
{
}
