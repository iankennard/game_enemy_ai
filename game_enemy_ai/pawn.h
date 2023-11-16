// The Pawn class is a base class to inherit for the Player and Enemy. It defines features common to both.

#ifndef IANKENNARD_PAWN_H
#define IANKENNARD_PAWN_H

#include "constants.h"
#include "ball.h"
#include "coin.h"

class Pawn
{
protected:
	float px, py, pz, vx, vy, vz, pox, poy, vo; // (px, py, pz) position, (vx, vy, vz) velocity, pox, poy orientation, vo orientation velocity
	bool frozen; // is the pawn frozen?
	int ftimer; // if frozen, countdown timer to be unfrozen
	bool render3d; // should the pawn be rendered in 3d? (needed for changing camera focus)
	int score; // number of points the pawn has
	BallSet *balls; // pointer to the ball set
	CoinSet *coins; // pointer to the coin set
	float dist(float x, float y, float z); // distance of pawn from (x, y, z)

public:
	Pawn();
	Pawn(float ipx, float ipy, float ipz, float ivx, float ivy, float ivz, float ipox, float ipoy, float ivo, BallSet *ib, CoinSet *ic); // (ipx, ipy, ipz) position, (ivx, ivy, ivz) velocity, ipox, ipoy orientation, ivo orientation velocity, pointers to ball set and coin set
	float x(); // return pawn x coordinate
	float y(); // return pawn y coordinate
	float z(); // return pawn z coordinate
	float velx(); // return pawn x velocity coordinate
	float vely(); // return pawn y velocity coordinate
	float velz(); // return pawn z velocity coordinate
	float ox(); // return pawn x orientation
	float oy(); // return pawn y orientation
	int sc(); // return pawn score
	void update(float dt); // called as often as desired. dt is the time that has passed since last call
	void move(float dist, float x, float y, float z); // move the pawn a distance of dist units in the (x, y, z) direction. needed for outside collision adjustment
	void shoot(); // shoots a ball
	void freeze(); // makes pawn frozen
	void setRender3d(bool onoff); // set 3d rendering to onoff
	void addScore(int num); // add num points to the current score
	~Pawn();
};

#endif