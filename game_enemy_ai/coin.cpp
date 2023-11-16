// The Coin class is for creating coins to pick up and obtain points. Points are needed to win the game (not yet implemented)
// and also to fire Balls. The CoinSet class is to manage Coins more easily.

#include "coin.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>

extern float length(float x, float y, float z);
extern void normalize(float &x, float &y, float &z);
extern int numRooms;
extern Room rooms[WORLD_HEIGHT][WORLD_WIDTH];

Coin::Coin() {
	px = 0; py = 0; pz = 0;
	rot = 0;
	id = 0; // not good, shouldn't happen
	taken = false;
}

Coin::Coin(float ipx, float ipy, float ipz, int iid) { // (ipx, ipy, ipz) position, and iid coin ID
	px = ipx; py = ipy; pz = ipz;
	rot = 0;
	id = iid;
	taken = false;
}

bool Coin::isTaken() { // is the coin taken? (i.e., can't pick it up right now because it hasn't respawned)
	return taken;
}

float Coin::dist(float x, float y, float z) { // return the distance from (x, y, z) to the coin
	return length(px-x, py-y, pz-z);
}

float Coin::x() {return px;} // return coin x coordinate

float Coin::y() {return py;} // return coin y coordinate

float Coin::z() {return pz;} // return coin z coordinate

int Coin::i() {return id;} // return coin ID

void Coin::take() { // used to take the coin (obtain a point, disappear and start respawn timer)
	taken = true;
	count = 0;
}

void Coin::update(float dt) { // called as often as desired. dt is the time that has passed since last call
	rot += COIN_ROT*dt; // graphical use only
	while(rot >= 2*PI)
		rot -= 2*PI;
	if(taken) // count up on respawn timer
		count++;
	if(count >= COIN_RESPAWN_TIME) // if respawn timer is up, make coin reappear
		taken = false;
}

void Coin::draw2d(bool debug) { // draw the coin 2d (for the map)
	if(taken)
		return;
	float cent = COIN_DISPLAY_SIZE/2;

	glPushMatrix();

	glTranslatef(px, py, 0);
	glRotatef(180*rot/PI, 0, 0, 1);

	glColor3f(1,1,.5);
	glBegin(GL_TRIANGLES);
	glVertex2f(cent, 0);
	glVertex2f(cent*COS_2PI_3, cent*SIN_2PI_3);
	glVertex2f(cent*COS_4PI_3, cent*SIN_4PI_3);
	glEnd();

	glColor3f(0,0,0);
	glLineWidth(4);
	glBegin(GL_LINE_LOOP);
	glVertex2f(cent, 0);
	glVertex2f(cent*COS_2PI_3, cent*SIN_2PI_3);
	glVertex2f(cent*COS_4PI_3, cent*SIN_4PI_3);
	glEnd();

	glPopMatrix();
}

void Coin::draw3d(bool debug) { // draw the coin in 3d
	if(taken)
		return;
	float cent = COIN_DISPLAY_SIZE/2;
	float thick = COIN_DISPLAY_WIDTH/2;

	glPushMatrix();

	glTranslatef(px, py, pz);
		
	glColor3f(1,1,.5);

	glPushMatrix();
	//glRotatef(180*rot/PI/2, 0, 0, 1);
	glRotatef(180*rot/PI, 0, 1, 0);

	glPushMatrix();

	glRotatef(180*rot/PI, 0, 0, 1);

	glBegin(GL_TRIANGLES);
	glVertex3f(cent, 0, thick);
	glVertex3f(cent*COS_2PI_3, cent*SIN_2PI_3, thick);
	glVertex3f(cent*COS_4PI_3, cent*SIN_4PI_3, thick);

	glVertex3f(cent, 0, -thick);
	glVertex3f(cent*COS_4PI_3, cent*SIN_4PI_3, -thick);
	glVertex3f(cent*COS_2PI_3, cent*SIN_2PI_3, -thick);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(cent, 0, -thick);
	glVertex3f(cent*COS_2PI_3, cent*SIN_2PI_3, -thick);
	glVertex3f(cent*COS_2PI_3, cent*SIN_2PI_3, thick);
	glVertex3f(cent, 0, thick);

	glVertex3f(cent, 0, thick);
	glVertex3f(cent*COS_4PI_3, cent*SIN_4PI_3, thick);
	glVertex3f(cent*COS_4PI_3, cent*SIN_4PI_3, -thick);
	glVertex3f(cent, 0, -thick);

	glVertex3f(cent*COS_2PI_3, cent*SIN_2PI_3, thick);
	glVertex3f(cent*COS_2PI_3, cent*SIN_2PI_3, -thick);
	glVertex3f(cent*COS_4PI_3, cent*SIN_4PI_3, -thick);
	glVertex3f(cent*COS_4PI_3, cent*SIN_4PI_3, thick);
	glEnd();

	glColor3f(0,0,0);
	glLineWidth(4);
	glBegin(GL_LINE_STRIP);
	glVertex3f((cent+OUTLINE_OFFSET)*COS_4PI_3, (cent+OUTLINE_OFFSET)*SIN_4PI_3, thick+OUTLINE_OFFSET);
	glVertex3f((cent+OUTLINE_OFFSET), 0, thick+OUTLINE_OFFSET);
	glVertex3f((cent+OUTLINE_OFFSET)*COS_2PI_3, (cent+OUTLINE_OFFSET)*SIN_2PI_3, thick+OUTLINE_OFFSET);
	glVertex3f((cent+OUTLINE_OFFSET)*COS_4PI_3, (cent+OUTLINE_OFFSET)*SIN_4PI_3, thick+OUTLINE_OFFSET);
	glVertex3f((cent+OUTLINE_OFFSET)*COS_4PI_3, (cent+OUTLINE_OFFSET)*SIN_4PI_3, -thick-OUTLINE_OFFSET);
	glVertex3f((cent+OUTLINE_OFFSET)*COS_2PI_3, (cent+OUTLINE_OFFSET)*SIN_2PI_3, -thick-OUTLINE_OFFSET);
	glVertex3f((cent+OUTLINE_OFFSET), 0, -thick-OUTLINE_OFFSET);
	glVertex3f((cent+OUTLINE_OFFSET)*COS_4PI_3, (cent+1)*SIN_4PI_3, -thick-OUTLINE_OFFSET);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f((cent+OUTLINE_OFFSET)*COS_2PI_3, (cent+OUTLINE_OFFSET)*SIN_2PI_3, thick+OUTLINE_OFFSET);
	glVertex3f((cent+OUTLINE_OFFSET)*COS_2PI_3, (cent+OUTLINE_OFFSET)*SIN_2PI_3, -thick-OUTLINE_OFFSET);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f((cent+OUTLINE_OFFSET), 0, thick+OUTLINE_OFFSET);
	glVertex3f((cent+OUTLINE_OFFSET), 0, -thick-OUTLINE_OFFSET);
	glEnd();

	glPopMatrix();

	glPopMatrix();

	glPopMatrix();
}

Coin::~Coin() {}



CoinSet::CoinSet() {
	numCoins = 0;
}

CoinSet::CoinSet(int coords[][3], int count) { // takes in an array of coordinates to spawn coins at
	numCoins = count;
	for(int i=0; i<numCoins; i++) {
		coins[i] = Coin(coords[i][0], coords[i][1], coords[i][2], i);
	}
}

int CoinSet::num() { // returns how many coins there are total
	return numCoins;
}

Coin CoinSet::giveCoin(int cnum) { // return a pointer to the coin indexed by cnum
	if(cnum >= numCoins)
		throw "Out of bounds";
	return coins[cnum];
}

bool CoinSet::takeCoin(float x, float y, float z, float rad, float height) { // check to see if (x, y, z) is close enough to the coin so that it can be taken (by a player with radius rad and height height)
	for(int i=0; i<numCoins; i++) {
		if(!coins[i].isTaken() && coins[i].dist(x,y,z) < COIN_DISPLAY_SIZE/2+rad) {
			coins[i].take();
			return true;
		}
	}
	return false;
}

void CoinSet::update(float dt) { // update all of the coins in the set
	for(int i=0; i<numCoins; i++) {
		coins[i].update(dt);
	}
}

void CoinSet::draw2d(bool debug) { // draw all of the coins in the set in 2d
	for(int i=0; i<numCoins; i++) {
		coins[i].draw2d(debug);
	}
}

void CoinSet::draw3d(bool debug) { // draw all of the coins in the set in 3d
	for(int i=0; i<numCoins; i++) {
		coins[i].draw3d(debug);
	}
}