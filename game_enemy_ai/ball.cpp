// The Ball class is for spawning projectiles that freeze other players on hit. Each Ball uses up one point to fire.
// The BallSet class is to manage Balls more easily.

#include "ball.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>

extern float length(float x, float y, float z);
extern void normalize(float &x, float &y, float &z);
extern int numRooms;
extern Room rooms[WORLD_HEIGHT][WORLD_WIDTH];

const float Ball::cent = BALL_DISPLAY_SIZE/2;
const float Ball::X = .525731112119133606; // for displaying balls as isocahedrons
const float Ball::Z = .850650808352039932; // for displaying balls as isocahedrons
const float Ball::vdata[12][3] = { {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},   // for displaying balls as isocahedrons
	                               {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
	                               {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0} };
const int Ball::tindices[20][3] = { {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},             // for displaying balls as isocahedrons
	                                {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
	                                {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
	                                {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

Ball::Ball() {
	px = 0; py = 0; pz = 0;
	vx = 0; vy = 0; vz = 0;
	rot = 0;
	id = 0;
	purge = false;
}

Ball::Ball(float ipx, float ipy, float ipz, float ivx, float ivy, float ivz, int iid) { // (ipx, ipy, ipz) position, (ivx, ivy, ivz) velocity, and iid ID
	px = ipx; py = ipy; pz = ipz;
	vx = ivx; vy = ivy; vz = ivz;
	rot = 0;
	id = iid;
	purge = false;
}

bool Ball::needPurge() { // does this ball need to be purged from the ball set?
	return purge;
}

bool Ball::hit(float x, float y, float z, float rad, float height) { // has this ball hit a target at (x, y, z) with radius rad and height height?
	return (length(px-x,py-y,0)<rad && pz<z+height/2 && pz>z-height/2);
}

float Ball::x() {return px;} // return ball x coordinate

float Ball::y() {return py;} // return ball y coordinate

float Ball::z() {return pz;} // return ball z coordinate

float Ball::ox() {return atan2(vy,vx);} // return ball x orientation

float Ball::oy() {return atan2(vz,sqrt(vx*vx+vy*vy));} // return ball y orientation

int Ball::i() {return id;} // return ball ID

void Ball::requestPurge() { // set purge flag, will delete this ball soon
	purge = true;
}

void Ball::update(float dt) { // called as often as desired. dt is the time that has passed since last call
	vx += GRAVX*dt; vy += GRAVY*dt; vz += GRAVZ*dt;
	px += vx*dt; py += vy*dt; pz += vz*dt;
	rot += BALL_ROT*dt;
	while(rot >= 2*PI)
		rot -= 2*PI;

	bool inside = false; // check to see if the ball is inside the game space
	for(int i=0; i<WORLD_HEIGHT; i++) {
		for(int j=0; j<WORLD_WIDTH; j++) {
			if(!BARRIER[i][j] && rooms[i][j].isIn(px, py, pz, 0, 0)) {
				inside = true;
				break;
			}
		}
	}
	if(!inside) // delete if no longer in game space
		purge = true;
}

void Ball::draw2d(bool debug) { // draw the coin 2d (for the map)
	glPushMatrix();

	glTranslatef(px, py, 0);

	glPushMatrix();

	glRotatef(180*rot/PI, 0, 0, 1);

	glColor3f(1,1,1);
	glBegin(GL_QUADS);		
	glVertex2f(-cent,cent);
	glVertex2f(-cent,-cent);
	glVertex2f(cent,-cent);
	glVertex2f(cent,cent);
	glEnd();

	glColor3f(0,0,0);
	glLineWidth(4);
	glBegin(GL_LINE_LOOP);
	glVertex2f(-cent,cent);
	glVertex2f(-cent,-cent);
	glVertex2f(cent,-cent);
	glVertex2f(cent,cent);
	glEnd();

	glPopMatrix();

	glPopMatrix();
}

void Ball::draw3d(bool debug) { // draw the coin in 3d
	float cent = BALL_DISPLAY_SIZE/2;
		
	/*glPushMatrix();

	glTranslatef(px, py, pz);

	glPushMatrix();

	glRotatef(180*rot/PI, 0, 0, 1);

	glColor3f(1,1,1);
	glBegin(GL_QUADS);		
	glVertex3f(-cent,0,cent);
	glVertex3f(-cent,0,-cent);
	glVertex3f(cent,0,-cent);
	glVertex3f(cent,0,cent);

	glVertex3f(-cent,0,cent);
	glVertex3f(cent,0,cent);
	glVertex3f(cent,0,-cent);
	glVertex3f(-cent,0,-cent);

	glVertex3f(0,-cent,cent);
	glVertex3f(0,-cent,-cent);
	glVertex3f(0,cent,-cent);
	glVertex3f(0,cent,cent);

	glVertex3f(0,-cent,cent);
	glVertex3f(0,cent,cent);
	glVertex3f(0,cent,-cent);
	glVertex3f(0,-cent,-cent);
	glEnd();

	glColor3f(0,0,0);
	glLineWidth(4);
	glBegin(GL_LINE_STRIP);
	glVertex3f(-cent-OUTLINE_OFFSET,0,cent+OUTLINE_OFFSET);
	glVertex3f(-cent-OUTLINE_OFFSET,0,-cent-OUTLINE_OFFSET);
	glVertex3f(cent+OUTLINE_OFFSET,0,-cent-OUTLINE_OFFSET);
	glVertex3f(cent+OUTLINE_OFFSET,0,cent+OUTLINE_OFFSET);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0,-cent-OUTLINE_OFFSET,cent+OUTLINE_OFFSET);
	glVertex3f(0,-cent-OUTLINE_OFFSET,-cent-OUTLINE_OFFSET);
	glVertex3f(0,cent+OUTLINE_OFFSET,-cent-OUTLINE_OFFSET);
	glVertex3f(0,cent+OUTLINE_OFFSET,cent+OUTLINE_OFFSET);
	glEnd();

	glPopMatrix();

	glPopMatrix();*/

	glPushMatrix();

	glTranslatef(px, py, pz);

	glPushMatrix();
	glRotatef(180*rot/PI, 0, 0, 1);
	
	glScalef(cent,cent,cent);
	glColor3f(1,1,1);
	glBegin(GL_TRIANGLES);
	for (int i=0; i<20; i++) {
		glVertex3fv(&vdata[tindices[i][0]][0]);
		glVertex3fv(&vdata[tindices[i][1]][0]);
		glVertex3fv(&vdata[tindices[i][2]][0]);
	}
	glEnd();

	glColor3f(0,0,0);
	glLineWidth(4);
	for(int i=0; i<20; i++) {
		glBegin(GL_LINE_LOOP);
		glVertex3fv(&vdata[tindices[i][0]][0]);
		glVertex3fv(&vdata[tindices[i][1]][0]);
		glVertex3fv(&vdata[tindices[i][2]][0]);
		glEnd();
	}
	
	glPopMatrix();
	glPopMatrix();
}

Ball::~Ball() {}



void BallSet::purgeBalls() { // delete any balls that have passed outside of the valid game space or have hit a player
	int offset=0;
	for(int i=0; i<numBalls; i++) {
		if(balls[i].needPurge()) {
			offset++;
			numBalls--;
		}
		else {
			balls[i-offset] = balls[i];
		}
	}
}

BallSet::BallSet() {
	numBalls = 0;
}

int BallSet::num() { // return the number of balls in the set
	return numBalls;
}

Ball BallSet::giveBall(int bnum) { // return a pointer to the ball indexed by bnum
	if(bnum >= numBalls)
		throw "Out of bounds";
	return balls[bnum];
}

bool BallSet::hit(float x, float y, float z, float rad, float height) { //check to see if any ball in the set has hit a target at (x, y, z) with radius rad and height height
	for(int i=0; i<numBalls; i++) {
		if(balls[i].hit(x,y,z,rad,height)) {
			balls[i].requestPurge();
			return true;
		}
	}
	return false;
}

void BallSet::spawnBall(float x, float y, float z, float ox, float oy) { // create a ball at (x, y, z) with (ox, oy) orientation
	if(numBalls == MAX_BALLS) { // if there are already too many balls, delete oldest one to make room for new one
		for(int i=0; i<MAX_BALLS; i++) {
			balls[i] = balls[i+1];
		}
		balls[MAX_BALLS-1] = Ball(x+cos(ox)*cos(oy)*BALL_OFF, y+sin(ox)*BALL_OFF, z+sin(oy)*BALL_OFF, cos(ox)*cos(oy)*BALL_VEL, sin(ox)*cos(oy)*BALL_VEL, sin(oy)*BALL_VEL, MAX_BALLS-1);
	}
	else {
		balls[numBalls] = Ball(x+cos(ox)*cos(oy)*BALL_OFF, y+sin(ox)*BALL_OFF, z+sin(oy)*BALL_OFF, cos(ox)*cos(oy)*BALL_VEL, sin(ox)*cos(oy)*BALL_VEL, sin(oy)*BALL_VEL, numBalls);
		numBalls++;
	}
}

void BallSet::update(float dt) { // update all of the balls in the set
	for(int i=0; i<numBalls; i++) {
		balls[i].update(dt);
	}
	if(numBalls>0)
		purgeBalls(); // start deleting balls that are flagged for purge
}

void BallSet::draw2d(bool debug) { // draw all of the balls in the set in 2d
	for(int i=0; i<numBalls; i++) {
		balls[i].draw2d(debug);
	}
}

void BallSet::draw3d(bool debug) { // draw all of the balls in the set in 3d
	for(int i=0; i<numBalls; i++) {
		balls[i].draw3d(debug);
	}
}