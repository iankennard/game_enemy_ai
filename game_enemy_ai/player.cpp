// The Player class is a type of Pawn. It is controlled by the human playing the game, as opposed to the Enemy class, which is controlled by the AI.

#include "player.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>

extern float length(float x, float y, float z);
extern void normalize(float &x, float &y, float &z);
extern int numRooms;
extern Room rooms[WORLD_HEIGHT][WORLD_WIDTH];

Player::Player(): Pawn() {
	v = 0; vl = 0;
	lastmove = 1; lastlmove = 1;
	invisible = false;
	render3d = false;
}

Player::Player(float ipx, float ipy, float ipz, float ivx, float ivy, float ivz, float ipox, float ipoy, BallSet *ib, CoinSet *ic): Pawn(ipx, ipy, ipz, ivx, ivy, ivz, ipox, ipoy, 0, ib, ic) { // (ipx, ipy, ipz) position, (ivx, ivy, ivz) velocity, ipox, ipoy orientation, pointers to ball set and coin set
	v = 0; vl = 0;
	lastmove = 1; lastlmove = 1;
	invisible = false;
	render3d = false;
}

bool Player::isInvisible() {return invisible;} // is the player invisible?

void Player::togInvisible() { // toggle player invisibility
	if(invisible)
		invisible = false;
	else
		invisible = true;
}

void Player::keyboard(unsigned char key, int x, int y) // process a keyboard command. (x, y) is current mouse position
{
	if(!invisible) {
		switch(key) {
			case 'w': // forward
			v = 1; // start velocity
			if(lastmove==0) // reset no action counter
				lastmove = -1;
			else
				lastmove = 0;
			break;

			case 's': // backward
			v = -1; // start velocity
			if(lastmove==0) // reset no action counter
				lastmove = -1;
			else
				lastmove = 0;
			break;

			//if(key == 'a') {
			//	vx -= sin(po);
			//	vy += cos(po);
			//	lastmove = 0;
			//}

			//if(key == 'd') {
			//	vx += sin(po);
			//	vy -= cos(po);
			//	lastmove = 0;
			//}

			case 'f': // fire
			shoot();
			break;

			//if(key == 'j') {
			//	vo = 1;
			//	lastamove = 0;
			//}

			//if(key == 'l') {
			//	vo = -1;
			//	lastamove = 0;
			//}

			case 'a': // strafe left
			vl = -1; // start velocity
			if(lastlmove==0) // reset no action counter
				lastlmove = -1;
			else
				lastlmove = 0;
			break;

			case 'd': // strafe right
			vl = 1; // start velocity
			if(lastlmove==0) // reset no action counter
				lastlmove = -1;
			else
				lastlmove = 0;
			break;

			default: // no other keys do anything
			break;
		}
		float vmag = length(v,vl,0); // scale velocity to not be above maximum allowed speed
		if(vmag > MAX_VEL) {
			float multi = MAX_VEL/vmag;
			v *= multi;
			vl *= multi;
		}
	}
}

void Player::keyboardUp(unsigned char key, int x, int y) // process a keyboard command stopping. (x, y) is current mouse position
{
	switch(key) {
		case 'w':
		case 's':
		lastmove++; // no forward/backward movement, increase counter
		break;

		case 'a':
		case 'd':
		lastlmove++; // no left/right strafing movement, increase counter
		break;

		case 32: // spacebar
		if(pz<PLAYER_HEIGHT/2+MAX_VERTICAL_COLLISION) // jump if not already in the air
			vz += JUMP_VEL;
		break;

		default: // no other keys do anything
		break;
	}
}

void Player::mouse(int x, int y) // process mouse movement. (x, y) is the amount the mouse moved
{
	pox -= MOUSE_SENSITIVITY*x;
	while(pox>=2*PI)
		pox-=2*PI;
	while(pox<0)
		pox+=2*PI;
	poy -= MOUSE_SENSITIVITY*y;
	if(poy>MAX_ANGLE) // cannot look higher than straight up or low than straight down
		poy=MAX_ANGLE;
	if(poy<-MAX_ANGLE)
		poy=-MAX_ANGLE;
}

void Player::update(float dt) { // called as often as desired. dt is the time that has passed since last call
	if(!invisible) {
		if(frozen) {
			ftimer-=dt;
			if(ftimer <= 0)
				frozen = false;
		}
		else {
			// update position
			vx = v*cos(pox)*MOVEMENT_MULTIPLIER*dt + vl*sin(pox)*MOVEMENT_MULTIPLIER; // update actual velocity according to smooth input
			vy = v*sin(pox)*MOVEMENT_MULTIPLIER*dt - vl*cos(pox)*MOVEMENT_MULTIPLIER;
			vz += GRAVZ*dt;
			if(abs(vz) > TERMINAL_VELOCITY) { // if falling at terminal velocity, no more acceleration
				if(vz>0)
					vz = TERMINAL_VELOCITY;
				if(vz<0)
					vz = -TERMINAL_VELOCITY;
			}
			px += vx; // update position according to velocity
			py += vy;
			pz += vz;
			
			//// update orientation
			//if(vo != 0) {
			//	po += vo*ROTATION_MULTIPLIER*dt;
			//}

			// when no input for a certain time, smoothly deccelerate to 0
			if(lastmove>0 && lastmove<=2)
				lastmove++;
			if(lastlmove>0 && lastlmove<=2)
				lastlmove++;
			if(lastmove > 2) {
				v /= 16;
				v /= 16;
			}
			if(lastlmove > 2) {
				vl /= 16;
				vl /= 16;
			}
		}

		// check collision; if no longer in a room move player back to valid location
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
			px=mpx; py=mpy; pz=mpz;
		}

		if(balls->hit(px, py, pz, PLAYER_RADIUS, PLAYER_HEIGHT)) { // check if a ball hit
			freeze();
		}

		if(coins->takeCoin(px, py, pz, PLAYER_RADIUS, PLAYER_HEIGHT)) { // check if a coin was taken
			score++;
		}
	}
}

void Player::draw2d(bool debug) { // draw the player 2d (for the map)
	if(!invisible) {
		glPushMatrix();

		glTranslatef(px, py, 0);
		glRotatef(180*pox/PI, 0, 0, 1);

		glColor3f(0,0,0);
		glLineWidth(4);
		glBegin(GL_LINE_STRIP);
		glVertex2f(0, 0);
		glVertex2f(60, 0);
		glEnd();

		if(frozen)
			glColor3f(0,0,0);
		else
			glColor3f(.5,1,1);
		
		glBegin(GL_TRIANGLES);
		glVertex2f(PLAYER_RADIUS, 0);
		glVertex2f(PLAYER_RADIUS*COS_2PI_3, PLAYER_RADIUS*SIN_2PI_3);
		glVertex2f(PLAYER_RADIUS*COS_4PI_3, PLAYER_RADIUS*SIN_4PI_3);
		glEnd();

		glColor3f(0,0,0);
		glLineWidth(4);
		glBegin(GL_LINE_LOOP);
		glVertex2f(PLAYER_RADIUS, 0);
		glVertex2f(PLAYER_RADIUS*COS_2PI_3, PLAYER_RADIUS*SIN_2PI_3);
		glVertex2f(PLAYER_RADIUS*COS_4PI_3, PLAYER_RADIUS*SIN_4PI_3);
		glEnd();

		glPopMatrix();
	}
}

void Player::draw3d(bool debug) { // draw the player 3d
	if(render3d) {
		glPushMatrix();

		glTranslatef(px, py, pz);

		glPushMatrix();
		glRotatef(90, 1, 0, 0);
		glRotatef(180*pox/PI, 0, 1, 0);

		if(frozen)
			glColor3f(0,0,0);
		else
			glColor3f(.5,1,1);
	
		glBegin(GL_TRIANGLES);
		glVertex3f(PLAYER_RADIUS, PLAYER_HEIGHT/2, 0);
		glVertex3f(PLAYER_RADIUS*COS_2PI_3, PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_2PI_3);
		glVertex3f(PLAYER_RADIUS*COS_4PI_3, PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_4PI_3);

		glVertex3f(PLAYER_RADIUS, -PLAYER_HEIGHT/2, 0);
		glVertex3f(PLAYER_RADIUS*COS_4PI_3, -PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_4PI_3);
		glVertex3f(PLAYER_RADIUS*COS_2PI_3, -PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_2PI_3);
		glEnd();

		glBegin(GL_QUADS);
		glVertex3f(PLAYER_RADIUS, PLAYER_HEIGHT/2, 0);
		glVertex3f(PLAYER_RADIUS, -PLAYER_HEIGHT/2, 0);
		glVertex3f(PLAYER_RADIUS*COS_2PI_3, -PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_2PI_3);
		glVertex3f(PLAYER_RADIUS*COS_2PI_3, PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_2PI_3);

		glVertex3f(PLAYER_RADIUS*COS_2PI_3, PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_2PI_3);
		glVertex3f(PLAYER_RADIUS*COS_2PI_3, -PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_2PI_3);
		glVertex3f(PLAYER_RADIUS*COS_4PI_3, -PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_4PI_3);
		glVertex3f(PLAYER_RADIUS*COS_4PI_3, PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_4PI_3);

		glVertex3f(PLAYER_RADIUS*COS_4PI_3, PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_4PI_3);
		glVertex3f(PLAYER_RADIUS*COS_4PI_3, -PLAYER_HEIGHT/2, PLAYER_RADIUS*SIN_4PI_3);
		glVertex3f(PLAYER_RADIUS, -PLAYER_HEIGHT/2, 0);
		glVertex3f(PLAYER_RADIUS, PLAYER_HEIGHT/2, 0);
		glEnd();

		glColor3f(0,0,0);
		glLineWidth(4);
		glBegin(GL_LINE_STRIP);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET), PLAYER_HEIGHT/2+OUTLINE_OFFSET, 0);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET)*COS_2PI_3, PLAYER_HEIGHT/2+OUTLINE_OFFSET, (PLAYER_RADIUS+OUTLINE_OFFSET)*SIN_2PI_3);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET)*COS_4PI_3, PLAYER_HEIGHT/2+OUTLINE_OFFSET, (PLAYER_RADIUS+OUTLINE_OFFSET)*SIN_4PI_3);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET), PLAYER_HEIGHT/2+OUTLINE_OFFSET, 0);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET), -PLAYER_HEIGHT/2-OUTLINE_OFFSET, 0);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET)*COS_4PI_3, -PLAYER_HEIGHT/2-OUTLINE_OFFSET, (PLAYER_RADIUS+OUTLINE_OFFSET)*SIN_4PI_3);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET)*COS_2PI_3, -PLAYER_HEIGHT/2-OUTLINE_OFFSET, (PLAYER_RADIUS+OUTLINE_OFFSET)*SIN_2PI_3);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET), -PLAYER_HEIGHT/2-OUTLINE_OFFSET, 0);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET)*COS_2PI_3, PLAYER_HEIGHT/2+OUTLINE_OFFSET, (PLAYER_RADIUS+OUTLINE_OFFSET)*SIN_2PI_3);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET)*COS_2PI_3, -PLAYER_HEIGHT/2-OUTLINE_OFFSET, (PLAYER_RADIUS+OUTLINE_OFFSET)*SIN_2PI_3);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET)*COS_4PI_3, PLAYER_HEIGHT/2+OUTLINE_OFFSET, (PLAYER_RADIUS+OUTLINE_OFFSET)*SIN_4PI_3);
		glVertex3f((PLAYER_RADIUS+OUTLINE_OFFSET)*COS_4PI_3, -PLAYER_HEIGHT/2-OUTLINE_OFFSET, (PLAYER_RADIUS+OUTLINE_OFFSET)*SIN_4PI_3);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex3f(0, 0, 0);
		glVertex3f(60, 0, 0);
		glEnd();

		glPopMatrix();

		glPopMatrix();
	}
}

Player::~Player() {}