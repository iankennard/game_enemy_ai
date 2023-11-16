// The Enemy class is a type of Pawn. It is controlled by the AI, as opposed to the Player class, which is controlled by the human playing the game.
// The AI function of the Enemy class is the most complex (and messy) part of this game.

#ifndef IANKENNARD_ENEMY_H
#define IANKENNARD_ENEMY_H

#include "constants.h"
#include "pawn.h"
#include "player.h"

class Enemy: public Pawn
{
private:
	float nextlocx, nextlocy; // (x, y) coordinate of the next point the enemy wishes to go
	float lastlocx, lastlocy; // (x, y) coordinate of the previous point the enemy was at, in case it needs to backtrack
	int still; // counter for how long the enemy hasn't significantly moved (check for being stuck)
	float destx, desty; // (x, y) coorindate of the ultimate destination that enemy wishes to reach
	int waypoint; // the number of the waypoint the enemy is on
	int curCoin; // the coin the enemy is currently targetting (if any)
	bool traveling; // is the enemy en route to a specific location?
	bool dodging; // is the enemy currently attempting to dodge a ball?
	bool fleeing; // is the enemy currently attempting to flee from the player?
	bool attacking; // is the enemy currently attempting to shoot at the player?
	bool stopped; // is the enemy purposely stopped (not moving) at the moment?
	int stoptimer; // length of time the enemy will stop
	float knownPlayer[5]; // information the enemy has about the player: px, py, vx, vy, age of sighting
	bool kBalls[MAX_BALLS]; // an array that tells whether the enemy knows about a particular ball
	bool kCoins[MAX_COINS]; // an array that tells whether the enemy knows about a particular coin
	Player *player; // pointer to the player instance
	bool kWorld[WORLD_HEIGHT][WORLD_WIDTH]; // an array that tells whether the enemy knows about a particular area of the game space
	bool kExplore[WORLD_HEIGHT][WORLD_WIDTH]; // an array that tells whether the enemy wants to explore a particular area of the game space next
	int timeout; // time to wait before starting to explore randomly
	int everyFive; // counter that updates where the enemy was last
	char status[200]; // string of text saying what the enemy is doing
	char debugstatus[200]; // additional information about the enemy
	float oldlocx, oldlocy; // the location that the enemy was going to before
	float blink; // counter for how fast the debugging objective marker changes color
	bool inView(float x, float y); // returns whether or not (x, y) is in the enemy's sight
	bool inRoom(float x, float y); // returns whether or not (x, y) in the game space
	void ai(); // called during updates to figure out what the enemy should do next
	float hfunc(int x1, int y1, int x2, int y2); // heuristic function for A* search
	bool nodeincs(int x, int y); // is (x, y) in the closed set for A*?
	bool nodeinos(int x, int y); // is (x, y) in the open set for A*?
	void expandNodes(int x, int y, int nx, int ny, int destx, int desty); // try expanding the path out to (nx, ny) from (x, y) given destination (destx, desty) with A*
	void buildPath(int origx, int origy, int destx, int desty); // create the waypoints involved in getting from (origx, origy) to (destx, desty)
	void findPath(int origx, int origy, int destx, int desty); // determine the best path from (origx, origy) to (destx, desty) with A*
	bool lineBlocked(float origx, float origy, float destx, float desty); // returns whether or not a line from (origx, origy) to (destx, desty) is block by a wall

	int path[100]; // an array of waypoints on the way to a destination
	int pathlen; // number of waypoints

	int oldpath[100]; // previous array of waypoints on the way to a destination
	int oldpathlen; // number of waypoints

	int openset[100]; // open set for A*
	int osnodeleft; // number of nodes left in the open set
	int closedset[100]; // closed set for A*
	int csnodeleft; // number of nodes left in the closed set
	float g_score[WORLD_HEIGHT][WORLD_WIDTH]; // scores for A* heuristic
	float h_score[WORLD_HEIGHT][WORLD_WIDTH]; // scores for A* heuristic
	float f_score[WORLD_HEIGHT][WORLD_WIDTH]; // scores for A* heuristic
	int camefrom[WORLD_HEIGHT][WORLD_WIDTH]; // which point the path was expanded from in the event the heuristic says the current node was a bad choice so we can backtrack

public:
	Enemy();
	Enemy(float ipx, float ipy, float ipz, float ivx, float ivy, float ivz, float ipox, float ipoy, float ivo, BallSet *ib, CoinSet *ic, Player *iplayer); // (ipx, ipy, ipz) position, (ivx, ivy, ivz) velocity, ipox, ipoy orientations, ivo orientation velocity, pointers to the ball set, coin set, and player
	char *giveStatus(); // return a string of text describing what the enemy is doing
	char *giveDebugStatus(); // return a debug version of a string of text describing what the enemy is doing
	void update(float dt); // called as often as desired. dt is the time that has passed since last call
	void draw2d(bool debug); // draw the enemy 2d (for the map)
	void draw3d(bool debug); // draw the enemy 3d
	~Enemy();
};

#endif