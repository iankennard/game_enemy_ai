#include "pawn.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>

extern float length(float x, float y, float z);
extern void normalize(float &x, float &y, float &z);
extern int numRooms;
extern Room rooms[WORLD_HEIGHT][WORLD_WIDTH];

float Pawn::dist(float x, float y, float z) { // distance of pawn from (x, y, z)
	return sqrt((px-x)*(px-x)+(py-y)*(py-y)+(pz-z)*(pz-z));
}

Pawn::Pawn() {
	px=0; py=0; pz=0;
	vx=0; vy=0; vz=0;
	pox=0; poy=0; vo=0;
	frozen=false; ftimer=-1;
	render3d = true;
	score=INITIAL_SCORE;
}

Pawn::Pawn(float ipx, float ipy, float ipz, float ivx, float ivy, float ivz, float ipox, float ipoy, float ivo, BallSet *ib, CoinSet *ic) { // (ipx, ipy, ipz) position, (ivx, ivy, ivz) velocity, ipox, ipoy orientation, ivo orientation velocity, pointers to ball set and coin set
	px = ipx; py = ipy; pz=ipz;
	vx = ivx; vy = ivy; vz = ivz;
	pox=ipox; poy=ipoy; vo=ivo;
	balls = ib;
	coins = ic;
	frozen=false; ftimer=-1;
	render3d = true;
	score=INITIAL_SCORE;
}

float Pawn::x() {return px;} // return pawn x coordinate

float Pawn::y() {return py;} // return pawn y coordinate

float Pawn::z() {return pz;} // return pawn z coordinate

float Pawn::velx() {return vx;} // return pawn x velocity coordinate

float Pawn::vely() {return vy;} // return pawn y velocity coordinate

float Pawn::velz() {return vz;} // return pawn z velocity coordinate

float Pawn::ox() {return pox;} // return pawn x orientation

float Pawn::oy() {return poy;} // return pawn y orientation

int Pawn::sc() { // return pawn score
	return score;
}

void Pawn::update(float dt) { // called as often as desired. dt is the time that has passed since last call
	if(frozen) {
		ftimer-=dt;
		if(ftimer <= 0)
			frozen = false;
	}
	//if(length(nextlocx-px, nextlocy-py, 0) < COIN_DISPLAY_SIZE)  // makes if only run ai after reaching current destination
	//vx = nextlocx-px; vy = nextlocy-py; vz = nextlocz-pz;
	else {
		vx=cos(pox); vy=sin(pox); vz=0;
		normalize(vx, vy, vz);
		vx *= MAX_VEL; vy *= MAX_VEL; vz *= MAX_VEL;
		px += vx*dt; py += vy*dt; pz += vz*dt;
	}

	bool inside = false;
	for(int i=0; i<WORLD_HEIGHT; i++) {
		for(int j=0; j<WORLD_WIDTH; j++) {
			if(!BARRIER[i][j] && rooms[i][j].isIn(px, py, pz, PLAYER_RADIUS, PLAYER_HEIGHT)) {
				inside=true;
				break;
			}
		}
	}
	if(!inside) {
		float mpx, mpy, mpz;
		float dist = rooms[0][0].dist(px, py, pz, PLAYER_RADIUS, PLAYER_HEIGHT, mpx, mpy, mpz);
		for(int i=0; i<WORLD_HEIGHT; i++) {
			for(int j=0; j<WORLD_WIDTH; j++) {
				if(!BARRIER[i][j]) {
					float temppx, temppy, temppz;
					float tdist = rooms[i][j].dist(px, py, pz, PLAYER_RADIUS, PLAYER_HEIGHT, temppx, temppy, temppz);
					if(tdist < dist) {
						mpx=temppx; mpy=temppy; mpz=temppz; dist = tdist;
					}
				}
			}
		}
		mpx-=px; mpy-=py; mpz-=pz;
		normalize(mpx, mpy, mpz);
		move(dist, mpx, mpy, mpz);
	}

	if(balls->hit(px,py, pz, PLAYER_RADIUS, PLAYER_HEIGHT)) {
		freeze();
	}

	if(coins->takeCoin(px, py, pz, PLAYER_RADIUS, PLAYER_HEIGHT)) {
		score++;
	}
}

void Pawn::move(float dist, float x, float y, float z) { // move the pawn a distance of dist units in the (x, y, z) direction. needed for outside collision adjustment
	normalize(x, y, z);
	px += x*dist;
	py += y*dist;
	pz += z*dist;
}

void Pawn::shoot() { // shoots a ball
	if(score > 0) {
		balls->spawnBall(px, py, pz, pox, poy);
		score--;
	}
}

void Pawn::freeze() { // makes pawn frozen
	frozen = true;
	ftimer = FREEZE_TIME;
}

void Pawn::setRender3d(bool onoff) { // set 3d rendering to onoff
	render3d = onoff;
}

void Pawn::addScore(int num) { // add num points to the current score
	score += num;
}

Pawn::~Pawn() {}