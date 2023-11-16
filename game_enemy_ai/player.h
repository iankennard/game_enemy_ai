// The Player class is a type of Pawn. It is controlled by the human playing the game, as opposed to the Enemy class, which is controlled by the AI.

#ifndef IANKENNARD_PLAYER_H
#define IANKENNARD_PLAYER_H

#include "constants.h"
#include "pawn.h"

class Player: public Pawn
{
private:
	float v; // this variable makes movement smooth
	float vl; // this variable makes rotational movement smooth
	int lastmove, lastlmove; // these variables countdown to when the movement should smoothly decrease to 0 due to lack of input
	bool invisible; // whether or not the player is invisible

public:
	Player();
	Player(float ipx, float ipy, float ipz, float ivx, float ivy, float ivz, float ipox, float ipoy, BallSet *ib, CoinSet *ic); // (ipx, ipy, ipz) position, (ivx, ivy, ivz) velocity, ipox, ipoy orientation, pointers to ball set and coin set
	bool isInvisible(); // is the player invisible?
	void togInvisible(); // toggle player invisibility
	void keyboard(unsigned char key, int x, int y); // process a keyboard command. (x, y) is current mouse position
	void keyboardUp(unsigned char key, int x, int y); // process a keyboard command stopping. (x, y) is current mouse position
	void mouse(int x, int y); // process mouse movement. (x, y) is the amount the mouse moved
	void update(float dt); // called as often as desired. dt is the time that has passed since last call
	void draw2d(bool debug); // draw the player 2d (for the map)
	void draw3d(bool debug); // draw the player 3d
	~Player();
};

#endif