// The Room class is the basic building block for creating the game space.
// Currently, only level worlds that can be divided into rectangular prisms can be created.

#ifndef IANKENNARD_ROOM_H
#define IANKENNARD_ROOM_H

class Room
{
private:
	int minx, miny, minz, lenx, leny, lenz; // (minx, miny, minz) minimum coordinates, (lenx, leny, lenz) dimensions of room
	int dminx, dminy, dminz, dlenx, dleny, dlenz; // (dminx, dminy, dminz) _displayed_ minimum coordinates, (dlenx, dleny, dlenz) _displayed_ dimensions of room
	bool roomabove, roomleft, roomright, roombelow; // whether or not there are rooms "above," "left" of, "right" of, or "below" this one
	bool roomtl, roomtr, roombl, roombr; // whether or not there are rooms "top left," "top right," "bottom left," or "bottom right" of this one
	bool explore, world, view; // variables used for enemy debugging info

public:
	Room();
	Room(int mx, int my, int mz, int lx, int ly, int lz, bool ra, bool rl, bool rr, bool rb, bool rtl, bool rtr, bool rbl, bool rbr); // (mx, my, mz) minimum coordinates, (lx, ly, lz) dimensions, ra, rl, rr, rb, rtl, rtr, rbl, rbr whether or not there are rooms there
	bool isIn(float x, float y, float z, float rad, float height); // is a cylinder of radius rad and height height centered at (x, y, z) inside this room?
	float dist(float x, float y, float z, float rad, float height, float &cpx, float &cpy, float &cpz); // distance and direction (cpx, cpy, cpz) that cylinder of radius rad and height height centered at (x, y, z) would need to be moved to be inside this room
	void setExplore(bool tog); // toggle explore variable for enemy debugging
	void setWorld(bool tog); // toggle world variable for enemy debugging
	void setView(bool tog); // toggle view variable for enemy debugging
	void draw2d(bool debug); // draw the room 2d (for the map)
	void draw3d(bool debug); // draw the room 3d
	~Room();
};

#endif