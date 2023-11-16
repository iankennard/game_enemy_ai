// The Room class is the basic building block for creating the game space.
// Currently, only level worlds that can be divided into rectangular prisms can be created.

#include "constants.h"
#include "room.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>

extern float length(float x, float y);
extern void normalize(float &x, float &y);

Room::Room() {
	minx=0; miny=0; minz=0; lenx=0; leny=0; lenz=0;
	dminx=0; dminy=0; dminz=0; dlenx=0; dleny=0; dlenz=0;
	roomabove=false; roomleft=false; roomright=false; roombelow=false;
	roomtl=false; roomtr=false; roombl=false; roombr=false;
	explore=false; world=false; view=false;
}

Room::Room(int mx, int my, int mz, int lx, int ly, int lz, bool ra, bool rl, bool rr, bool rb, bool rtl, bool rtr, bool rbl, bool rbr) { // (mx, my, mz) minimum coordinates, (lx, ly, lz) dimensions, ra, rl, rr, rb, rtl, rtr, rbl, rbr whether or not there are rooms there
	minx=mx-(int)rl*(PLAYER_RADIUS+2); miny=my-(int)rb*(PLAYER_RADIUS+2); minz=mz; lenx=lx+(int)rl*(PLAYER_RADIUS+2)+(int)rr*(PLAYER_RADIUS+2); leny=ly+(int)ra*(PLAYER_RADIUS+2)+(int)rb*(PLAYER_RADIUS+2); lenz=lz;
	dminx=mx; dminy=my; dminz=mz; dlenx=lx; dleny=ly; dlenz=lz;
	roomabove=ra; roomleft=rl; roomright=rr; roombelow=rb;
	roomtl=rtl; roomtr=rtr; roombl=rbl; roombr=rbr;
	explore=false; world=false; view=false;
}

bool Room::isIn(float x, float y, float z, float rad, float height) { // is a cylinder of radius rad and height height centered at (x, y, z) inside this room?
	return x>=minx+rad && y>=miny+rad && z>=height/2  &&  x<=minx+lenx-rad && y<=miny+leny-rad && z<=320-height/2;
}

float Room::dist(float x, float y, float z, float rad, float height, float &cpx, float &cpy, float &cpz) { // distance and direction (cpx, cpy, cpz) that cylinder of radius rad and height height centered at (x, y, z) would need to be moved to be inside this room
	cpx=x; cpy=y; cpz=z;
	if(x<minx+rad)
		cpx = minx+rad;
	else
		if(x>minx+lenx-rad)
			cpx = minx+lenx-rad;
	if(y<miny+rad)
		cpy = miny+rad;
	else
		if(y>miny+leny-rad)
			cpy = miny+leny-rad;
	if(z<height/2)
		cpz = height/2+PLAYER_FLOAT_HEIGHT;
	else
		if(z>WORLD_UP_HEIGHT-height/2)
			cpz = WORLD_UP_HEIGHT-height/2;
	return sqrt((x-cpx)*(x-cpx)+(y-cpy)*(y-cpy)+(z-cpz)*(z-cpz));
}

void Room::setExplore(bool tog) {explore=tog;} // toggle explore variable for enemy debugging

void Room::setWorld(bool tog) {world=tog;} // toggle world variable for enemy debugging

void Room::setView(bool tog) {view=tog;} // toggle view variable for enemy debugging

void Room::draw2d(bool debug) { // draw the room 2d (for the map)
	glPushMatrix();

	if(debug) { // special coloring for debug mode
		//if(view)
		//	glColor3f(.75, .75, .75); // currently in view
		//else {
			if(explore)
				glColor3f(.5, 1, .5); // to be explored
			else {
				if(world)
					glColor3f(.5, .5, 1); // explored
				else
					glColor3f(1, .5, .5); // unknown
			}
		//}
	}
	else
		glColor3f(1, .5, .5);

	glBegin(GL_QUADS);
	glVertex2f(dminx, dminy);
	glVertex2f(dminx+dlenx, dminy);
	glVertex2f(dminx+dlenx, dminy+dleny);
	glVertex2f(dminx, dminy+dleny);
	glEnd();

	glColor3f(0,0,0);
	glLineWidth(4);
	glBegin(GL_LINES);
	if(!roomabove) {
		glVertex2f(dminx, dminy+dleny+2);
		glVertex2f(dminx+dlenx, dminy+dleny+2);
	}
	if(!roomleft) {
		glVertex2f(dminx-2, dminy);
		glVertex2f(dminx-2, dminy+dleny);
	}
	if(!roomright) {
		glVertex2f(dminx+dlenx+2, dminy);
		glVertex2f(dminx+dlenx+2, dminy+dleny);
	}
	if(!roombelow) {
		glVertex2f(dminx, dminy-2);
		glVertex2f(dminx+dlenx, dminy-2);
	}
	glEnd();

	glColor3f(0,0,0);
	glPointSize(4);
	glBegin(GL_POINTS);
	if(!roomabove && !roomleft)
		glVertex2f(dminx-2, dminy+dleny+2);
	if(!roomabove && !roomright)
		glVertex2f(dminx+dlenx+2, dminy+dleny+2);
	if(!roombelow && !roomleft)
		glVertex2f(dminx-2, dminy-2);
	if(!roombelow && !roomright)
		glVertex2f(dminx+dlenx+2, dminy-2);
	glEnd();

	glPopMatrix();
}

void Room::draw3d(bool debug) { // draw the room 3d
	int offtl=0, offtr=0, offbl=0, offbr=0, offlt=0, offlb=0, offrt=0, offrb=0;

	// very ugly and convoluted way of determining where to draw outline lines
	glPushMatrix();

	glColor3f(1, .75, .75);
	glBegin(GL_QUADS);
	if(!roomabove) {
		offlt=-1;
		offrt=-1;
		glVertex3f(dminx, dminy+dleny, dminz+dlenz);
		glVertex3f(dminx, dminy+dleny, dminz);
		glVertex3f(dminx+dlenx, dminy+dleny, dminz);
		glVertex3f(dminx+dlenx, dminy+dleny, dminz+dlenz);
	}
	else {
		if(roomtl)
			offlt=1;
		if(roomtr)
			offrt=1;
	}
	if(!roombelow) {
		offlb=1;
		offrb=1;
		glVertex3f(dminx, dminy, dminz+dlenz);
		glVertex3f(dminx, dminy, dminz);
		glVertex3f(dminx+dlenx, dminy, dminz);
		glVertex3f(dminx+dlenx, dminy, dminz+dlenz);
	}
	else {
		if(roombl)
			offlb=-1;
		if(roombr)
			offrb=-1;
	}
	if(!roomleft) {
		offtl=1;
		offbl=1;
		glVertex3f(dminx, dminy, dminz+dlenz);
		glVertex3f(dminx, dminy, dminz);
		glVertex3f(dminx, dminy+dleny, dminz);
		glVertex3f(dminx, dminy+dleny, dminz+dlenz);
	}
	else {
		if(roomtl)
			offtl=-1;
		if(roombl)
			offbl=-1;
	}
	if(!roomright) {
		offtr=-1;
		offbr=-1;
		glVertex3f(dminx+dlenx, dminy, dminz+dlenz);
		glVertex3f(dminx+dlenx, dminy, dminz);
		glVertex3f(dminx+dlenx, dminy+dleny, dminz);
		glVertex3f(dminx+dlenx, dminy+dleny, dminz+dlenz);
	}
	else {
		if(roomtr)
			offtr=1;
		if(roombr)
			offbr=1;
	}

	// ceiling
	glColor3f(.75,1,.75);
	glVertex3f(dminx, dminy, dminz+dlenz);
	glVertex3f(dminx, dminy+dleny, dminz+dlenz);
	glVertex3f(dminx+dlenx, dminy+dleny, dminz+dlenz);
	glVertex3f(dminx+dlenx, dminy, dminz+dlenz);

	// floor
	if(debug) { // special colors for debug mode
		/*if(view)
			glColor3f(.5, .5, .5); // currently in view
		else {*/
			if(explore)
				glColor3f(.5, 1, .5); // to be explored
			else {
				if(world)
					glColor3f(.5, .5, 1); // explored
				else
					glColor3f(1, .5, .5); // unknown
			}
		/*}*/
	}
	else
		glColor3f(.75, .75, 1);
	glVertex3f(dminx, dminy, dminz);
	glVertex3f(dminx+dlenx, dminy, dminz);
	glVertex3f(dminx+dlenx, dminy+dleny, dminz);
	glVertex3f(dminx, dminy+dleny, dminz);
	glEnd();

	glColor3f(0,0,0);
	glLineWidth(6);
	glBegin(GL_LINES);
	if(!roomabove) {
		glVertex3f(dminx+offtl*OUTLINE_OFFSET, dminy+dleny-OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+dlenx+offtr*OUTLINE_OFFSET, dminy+dleny-OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+offtl*OUTLINE_OFFSET, dminy+dleny-OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
		glVertex3f(dminx+dlenx+offtr*OUTLINE_OFFSET, dminy+dleny-OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
	}
	if(!roombelow) {
		glVertex3f(dminx+offbl*OUTLINE_OFFSET, dminy+OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+dlenx+offbr*OUTLINE_OFFSET, dminy+OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+offbl*OUTLINE_OFFSET, dminy+OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
		glVertex3f(dminx+dlenx+offbr*OUTLINE_OFFSET, dminy+OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
	}
	if(!roomleft) {
		glVertex3f(dminx+OUTLINE_OFFSET, dminy+offlb*OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+OUTLINE_OFFSET, dminy+dleny+offlt*OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+OUTLINE_OFFSET, dminy+offlb*OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
		glVertex3f(dminx+OUTLINE_OFFSET, dminy+dleny+offlt*OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
	}
	if(!roomright) {
		glVertex3f(dminx+dlenx-OUTLINE_OFFSET, dminy+offrb*OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+dlenx-OUTLINE_OFFSET, dminy+dleny+offrt*OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+dlenx-OUTLINE_OFFSET, dminy+offrb*OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
		glVertex3f(dminx+dlenx-OUTLINE_OFFSET, dminy+dleny+offrt*OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
	}
	if((!roomtl && roomabove && roomleft) || (!roomabove && !roomleft)) {
		glVertex3f(dminx+OUTLINE_OFFSET, dminy+dleny-OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+OUTLINE_OFFSET, dminy+dleny-OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
	}
	if((!roomtr && roomabove && roomright) || (!roomabove && !roomright)) {
		glVertex3f(dminx+dlenx-OUTLINE_OFFSET, dminy+dleny-OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+dlenx-OUTLINE_OFFSET, dminy+dleny-OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
	}
	if((!roombl && roombelow && roomleft) || (!roombelow && !roomleft)) {
		glVertex3f(dminx+OUTLINE_OFFSET, dminy+OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+OUTLINE_OFFSET, dminy+OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
	}
	if((!roombr && roombelow && roomright) || (!roombelow && !roomright)) {
		glVertex3f(dminx+dlenx-OUTLINE_OFFSET, dminy+OUTLINE_OFFSET, dminz+OUTLINE_OFFSET);
		glVertex3f(dminx+dlenx-OUTLINE_OFFSET, dminy+OUTLINE_OFFSET, dminz+dlenz-OUTLINE_OFFSET);
	}
	glEnd();

	glPopMatrix();
}

Room::~Room() {}