// The Ball class is for spawning projectiles that freeze other players on hit. Each Ball uses up one point to fire.
// The BallSet class is to manage Balls more easily.

#ifndef IANKENNARD_BALL_H
#define IANKENNARD_BALL_H

#include "constants.h"
#include "room.h"

class Ball
{
private:
	float px, py, pz, vx, vy, vz, rot; // (x, y, z) position, (x, y, z) velocity, and diplay rotation
	int id; // ball ID
	bool purge; // delete the ball if it has passed outside of the valid game space or has hit a player
	static const float cent; // used for drawing the ball
	static const float X, Z; // the following constants are to draw an isocahedron easily
	static const float vdata[12][3]; // the following constants are to draw an isocahedron easily
	static const int tindices[20][3]; // the following constants are to draw an isocahedron easily
public:
	Ball();
	Ball(float ipx, float ipy, float ipz, float ivx, float ivy, float ivz, int iid); // (ipx, ipy, ipz) position, (ivx, ivy, ivz) velocity, and iid ID
	bool needPurge(); // does this ball need to be purged from the ball set?
	bool hit(float x, float y, float z, float rad, float height); // has this ball hit a target at (x, y, z) with radius rad and height height?
	float x(); // return ball x coordinate
	float y(); // return ball y coordinate
	float z(); // return ball z coordinate
	float ox(); // return ball x orientation
	float oy(); // return ball y orientation
	int i(); // return ball ID
	void requestPurge(); // set purge flag, will delete this ball soon
	void update(float dt); // called as often as desired. dt is the time that has passed since last call
	void draw2d(bool debug); // draw the coin 2d (for the map)
	void draw3d(bool debug); // draw the coin in 3d
	~Ball();
};

class BallSet
{
private:
	Ball balls[MAX_BALLS]; // array of balls
	int numBalls; // number of balls in array
	void purgeBalls(); // delete any balls that have passed outside of the valid game space or have hit a player
public:
	BallSet();
	int num(); // return the number of balls in the set
	Ball giveBall(int bnum); // return a pointer to the ball indexed by bnum
	bool hit(float x, float y, float z, float rad, float height); //check to see if any ball in the set has hit a target at (x, y, z) with radius rad and height height
	void spawnBall(float x, float y, float z, float ox, float oy); // create a ball at (x, y, z) with (ox, oy) orientation
	void update(float dt); // update all of the balls in the set
	void draw2d(bool debug); // draw all of the balls in the set in 2d
	void draw3d(bool debug); // draw all of the balls in the set in 3d
};

#endif