// The Coin class is for creating coins to pick up and obtain points. Points are needed to win the game (not yet implemented)
// and also to fire Balls. The CoinSet class is to manage Coins more easily.

#ifndef IANKENNARD_COIN_H
#define IANKENNARD_COIN_H

#include "constants.h"

class Coin
{
private:
	float px, py, pz, rot; // x, y, and z coordinates, and display rotation
	int id; // coin ID
	bool taken; // is the coin taken? (disppears, respawn timer starts)
	int count; // respawn timer

public:
	Coin();
	Coin(float ipx, float ipy, float ipz, int iid); // (ipx, ipy, ipz) position, and iid coin ID
	bool isTaken(); // is the coin taken? (i.e., can't pick it up right now because it hasn't respawned)
	float dist(float x, float y, float z); // return the distance from (x, y, z) to the coin
	float x(); // return coin x coordinate
	float y(); // return coin y coordinate
	float z(); // return coin z coordinate
	int i(); // return coin ID
	void take(); // used to take the coin (obtain a point, disappear and start respawn timer)
	void update(float dt); // called as often as desired. dt is the time that has passed since last call
	void draw2d(bool debug); // draw the coin 2d (for the map)
	void draw3d(bool debug); // draw the coin in 3d
	~Coin();
};

class CoinSet
{
private:
	Coin coins[MAX_COINS]; // array of coins
	int numCoins; // number of coins in array
public:
	CoinSet();
	CoinSet(int coords[][3], int count); // takes in an array of coordinates to spawn coins at
	int num(); // returns how many coins there are total
	Coin giveCoin(int cnum); // return a pointer to the coin indexed by cnum
	bool takeCoin(float x, float y, float z, float rad, float height); // check to see if (x, y, z) is close enough to the coin so that it can be taken (by a player with radius rad and height height)
	void update(float dt); // update all of the coins in the set
	void draw2d(bool debug); // draw all of the coins in the set in 2d
	void draw3d(bool debug); // draw all of the coins in the set in 3d
};

#endif