// The Enemy class is a type of Pawn. It is controlled by the AI, as opposed to the Player class, which is controlled by the human playing the game.
// The AI function of the Enemy class is the most complex (and messy) part of this game.

#include "enemy.h"
#include "randomc.h"
#include <string>
#include <time.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>

extern float length(float x, float y, float z);
extern void normalize(float &x, float &y, float &z);
extern int numRooms;
extern Room rooms[WORLD_HEIGHT][WORLD_WIDTH];

CRandomMersenne RNG(time(NULL)); // for random number generation

bool Enemy::inView(float x, float y) { // returns whether or not (x, y) is in the enemy's sight
	float ang = atan2(y-py,x-px);
	float aa = abs(ang-pox), ab = abs(ang-pox+2*PI), ac = abs(ang-pox-2*PI);
	float mina = aa;
	if(mina>ab)
		mina=ab;
	if(mina>ac)
		mina=ac;
	if(dist(x,y,pz) < VIEW_DIST && mina < VIEW_ANGLE/2 && !lineBlocked(px, py, x, y))
		return true;
	return false;
}

bool Enemy::inRoom(float x, float y) { // returns whether or not (x, y) in the game space
	for(int i=0; i<WORLD_HEIGHT; i++) {
		for(int j=0; j<WORLD_WIDTH; j++) {
			if(!BARRIER[i][j] && rooms[i][j].isIn(x, y, PLAYER_DEFAULT_HEIGHT, PLAYER_RADIUS, PLAYER_HEIGHT))
				return true;
		}
	}
	return false;
}

void Enemy::ai() { // called during updates to figure out what the enemy should do next
	// updates to knowledge via perception

	// update data based on what is in the visible cone
	for(int i=0; i<coins->num(); i++) {
		if(!kCoins[i] && inView(coins->giveCoin(i).x(), coins->giveCoin(i).y())) { // if visible
			kCoins[i] = true;
		}
	}

	for(int i=0; i<balls->num(); i++) {
		if(inView(balls->giveBall(i).x(), balls->giveBall(i).y())) { // if visible
			kBalls[i] = true;
		}
		else { // if no longer visible
			kBalls[i] = false;
		}
	}

	if(!player->isInvisible() && inView(player->x(), player->y())) { // if player visible
		knownPlayer[0] = player->x();
		knownPlayer[1] = player->y();
		knownPlayer[2] = player->velx();
		knownPlayer[3] = player->vely();
		knownPlayer[4] = 0;
	}
	else
		knownPlayer[4]++;
	

	/*
	// decision making
	Ball b;
	if(closeBall(&b))
		dodge(b);
	else {
		if(closePlayer()) {
			if(RNG.Random()<.5)
				flee();
			else
				attack();
		}
		else {
			Coin c;
			if(coinUp(&c))
				getCoin(c);
			else {
				explore();
			}
		}
	}
	*/

	// check for close balls
	if(!dodging && !stopped) {
		for(int i=0; i<balls->num(); i++) {
			if(kBalls[i] && dist(balls->giveBall(i).x(),balls->giveBall(i).y(),pz) < PLAYER_RADIUS*4 && abs(balls->giveBall(i).ox()-pox)<PI/6) { // if ball visible and close enough, start dodging the ball
				float dx=balls->giveBall(i).y()-py, dy=px-balls->giveBall(i).x(), dz=0;
				normalize(dx,dy,dz);
				dx*=64;
				dy*=64;
				nextlocx = px+dx;
				nextlocy = py+dy;
				dodging = true;
				traveling = true;
				fleeing = false;
				sprintf_s(status, 200, "Enemy Status: Dodging a ball by traveling to (%.2f, %.2f)", nextlocx, nextlocy);
				break;
			}
		}
	}

	// check for close player
	if(!player->isInvisible() && !fleeing && !dodging && !attacking && !stopped) {
		if(knownPlayer[4] < 20 && dist(knownPlayer[0],knownPlayer[1],pz) < 256) { // start fleeing the player
			if(RNG.Random()<.75) { // 3/4ths chance to flee, otherwise attack
				float fx=knownPlayer[0], fy=knownPlayer[1];
				float randx=RAND_WALK_RAD*(1-2*RNG.Random()), randy=RAND_WALK_RAD*(1-2*RNG.Random());
				int toc=0;
				while(!inRoom(px+randx, py+randy) || (toc<100 && (length(px+randx-fx,py+randy-fy,0) < dist(fx,fy,pz) || length(randx,randy,0) > dist(fx,fy,pz)))) {
					randx=RAND_WALK_RAD*(1-2*RNG.Random());
					randy=RAND_WALK_RAD*(1-2*RNG.Random());
					toc++;
				}
				nextlocx = px+randx;
				nextlocy = py+randy;
				fleeing = true;
				traveling = true;
				sprintf_s(status, 200, "Enemy Status: Fleeing the player by traveling to (%.2f, %.2f)", nextlocx, nextlocy);
			}
			else {
				float fx=knownPlayer[0]-px, fy=knownPlayer[1]-py, fz=0;
				normalize(fx,fy,fz);
				fx*=MAX_VEL*2;
				fy*=MAX_VEL*2;
				nextlocx = px+fx;
				nextlocy = py+fy;
				attacking = true;
				traveling = true;
				sprintf_s(status, 200, "Enemy Status: Attacking the player by traveling to (%.2f, %.2f)", nextlocx, nextlocy);
			}
		}
	}
	
	// if not fleeing or dodging, find a path to take to a coin
	if(!fleeing && !dodging && !attacking && !stopped) {
		bool kAvailCoin = false;
		for(int i=0; i<coins->num(); i++) {
			if(kCoins[i] && !coins->giveCoin(i).isTaken()) {
				kAvailCoin = true;
				if(curCoin == -1)
					curCoin = i;
				break;
			}
		}
		if(kAvailCoin) {
			if(coins->giveCoin(curCoin).isTaken() || !traveling) { // if target coin is taken or finished traveling
				int closest = 99999, key=-1;
				for(int i=0; i<coins->num(); i++) {
					if(kCoins[i] && !coins->giveCoin(i).isTaken() && closest > coins->giveCoin(i).dist(px, py, pz)) {
						key = i;
						closest = coins->giveCoin(i).dist(px, py, pz);
					}
				}
				if(key != -1) {
					destx = coins->giveCoin(key).x();
					desty = coins->giveCoin(key).y();
					curCoin = key;
					findPath(px/WORLD_BLOCK_SIZE, py/WORLD_BLOCK_SIZE, destx/WORLD_BLOCK_SIZE, desty/WORLD_BLOCK_SIZE);
					waypoint = 0;
					nextlocy = path[waypoint]/100*WORLD_BLOCK_SIZE+.5*WORLD_BLOCK_SIZE;
					nextlocx = path[waypoint]%100*WORLD_BLOCK_SIZE+.5*WORLD_BLOCK_SIZE;
					traveling = true;
					sprintf_s(status, 200, "Enemy Status: On way to coin %d by traveling to (%.2f, %.2f)", key, nextlocx, nextlocy);
				}
			}
		}
		else {	// don't know location of any coins, wander about randomly
			if(!traveling && timeout > STANDSTILL_TIMEOUT && !stopped) {
				bool exploringLeft = false;
				for(int i=0; i<WORLD_HEIGHT; i++) {
					if(!exploringLeft) {
						for(int j=0; j<WORLD_WIDTH; j++) {
							if(kExplore[i][j]) {
								exploringLeft = true;
								break;
							}
						}
					}
				}
				if(exploringLeft) { // something left to explore, pick random explore square
					int rand = floor(RNG.Random()*WORLD_HEIGHT*WORLD_WIDTH);
					while(!kExplore[rand/WORLD_WIDTH][rand%WORLD_WIDTH])
						rand=floor(RNG.Random()*WORLD_HEIGHT*WORLD_WIDTH);
					destx = WORLD_BLOCK_SIZE*(rand%WORLD_WIDTH+.5);
					desty = WORLD_BLOCK_SIZE*(rand/WORLD_WIDTH+.5);
					findPath(px/WORLD_BLOCK_SIZE, py/WORLD_BLOCK_SIZE, destx/WORLD_BLOCK_SIZE, desty/WORLD_BLOCK_SIZE);
					waypoint = 0;
					nextlocy = (path[waypoint]/100+.5)*WORLD_BLOCK_SIZE;
					nextlocx = (path[waypoint]%100+.5)*WORLD_BLOCK_SIZE;
					traveling = true;
					sprintf_s(status, 200, "Enemy Status: On way to exploration point (%.2f, %.2f) by traveling first to (%.2f, %.2f)", destx, desty, nextlocx, nextlocy);

				}
				else { // nothing left to explore, so do a true random walk
					float randx=RAND_WALK_RAD*(1-2*RNG.Random()), randy=RAND_WALK_RAD*(1-2*RNG.Random());
					int toc=0;
					while(toc<100 && length(randx,randy,pz) < RAND_WALK_RAD/2 || !inRoom(px+randx, py+randy) || lineBlocked(px,py,px+randx,py+randy)) {
						randx=RAND_WALK_RAD*(1-2*RNG.Random());
						randy=RAND_WALK_RAD*(1-2*RNG.Random());
						toc++;
					}
					//while(lineBlocked(px, py, randx, randy)) {
					//	randx=px+RAND_WALK_RAD-RNG.Random()*RAND_WALK_RAD*2;
					//	randy=py+RAND_WALK_RAD-RNG.Random()*RAND_WALK_RAD*2;
					//}
					nextlocx = px+randx;
					nextlocy = py+randy;
					destx = px+randx;
					desty = py+randy;
					sprintf_s(status, 100, "Enemy Status: Exploring via random walk, traveling to (%.2f, %.2f)", nextlocx, nextlocy);
				}

				traveling = true;
				timeout=0;
			}
			else
				timeout++;
		}
	}
	
	// ai for getting unstuck if running into a wall
	if(dist(lastlocx,lastlocy,pz)<MAX_VEL*MOVEMENT_MULTIPLIER/4 && vo<ROTATION_MULTIPLIER/2 && !stopped)
		still++;
	else
		still=0;
	if(traveling && still>10) { // stuck
		still=0;
		/*
		if(waypoint>0) {
			waypoint--;
			if(!lineBlocked(px, py, (path[waypoint]%100+.5)*WORLD_BLOCK_SIZE, (path[waypoint]/100+.5)*WORLD_BLOCK_SIZE)) {
				nextlocx = path[waypoint]%100*128+64;
				nextlocy = path[waypoint]/100*128+64;
			}
		}
		else {
			if(timeout > STANDSTILL_TIMEOUT) {
				float randx=RAND_WALK_RAD*(1-2*RNG.Random()), randy=RAND_WALK_RAD*(1-2*RNG.Random());
				while(length(randx,randy) < RAND_WALK_RAD/2 || !inRoom(px+randx, py+randy)) {
					randx=RAND_WALK_RAD*(1-2*RNG.Random());
					randy=RAND_WALK_RAD*(1-2*RNG.Random());
				}
				while(lineBlocked(px, py, randx, randy)) {
					randx=px+RAND_WALK_RAD-RNG.Random()*RAND_WALK_RAD*2;
					randy=py+RAND_WALK_RAD-RNG.Random()*RAND_WALK_RAD*2;
				}
				nextlocx = px+randx;
				nextlocy = py+randy;
				if(waypoint>0)
					waypoint--;
				traveling = true;
				timeout=0;
			}
			else
				timeout++;
		}
		*/
		findPath(px/WORLD_BLOCK_SIZE, py/WORLD_BLOCK_SIZE, destx/WORLD_BLOCK_SIZE, desty/WORLD_BLOCK_SIZE);
		waypoint = 0;
	}

	// if at current waypoint, set new destination to next waypoint
	if(dist(nextlocx,nextlocy,pz) < PLAYER_RADIUS/2 && !stopped) {
		if(attacking) {
			shoot();
		}
		waypoint++;
		dodging = false;
		fleeing = false;
		attacking = false;
		if(waypoint < pathlen) {
			nextlocx = (path[waypoint]%100+.5)*WORLD_BLOCK_SIZE;
			nextlocy = (path[waypoint]/100+.5)*WORLD_BLOCK_SIZE;
		}
		else
			traveling=false;
	}

	// if at final destination, no longer traveling
	if(dist(destx,desty,pz) < PLAYER_RADIUS/2 && !stopped) {
		pathlen = 0;
		traveling = false;
		dodging = false;
		fleeing = false;
		attacking = false;
	}
	
	if(stopped) {
		nextlocx = px;
		nextlocy = py;
		stoptimer++;
		if(stoptimer>50) {
			stoptimer=0;
			stopped = false;
		}
		sprintf_s(status, 200, "Enemy Status: Stopped");
	}

	if(everyFive%10==0) {
		lastlocx = px;
		lastlocy = py;
		everyFive=0;
	}
	everyFive++;
}

float Enemy::hfunc(int x1, int y1, int x2, int y2) { // heuristic function for A* search
	return sqrt((float)((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1)));
}

bool Enemy::nodeincs(int x, int y) { // is (x, y) in the closed set for A*?
	int num = y*100+x;
	for(int i=0; i<csnodeleft; i++) {
		if(closedset[i] == num)
			return true;
	}
	return false;
}

bool Enemy::nodeinos(int x, int y) { // is (x, y) in the open set for A*?
	int num = y*100+x;
	for(int i=0; i<osnodeleft; i++) {
		if(openset[i] == num)
			return true;
	}
	return false;
}

void Enemy::expandNodes(int x, int y, int nx, int ny, int destx, int desty) { // try expanding the path out to (nx, ny) from (x, y) given destination (destx, desty) with A*
	float tentg = g_score[y][x]+1;
	bool tentbetter = false;
	if(!nodeincs(nx, ny) && !nodeinos(nx, ny)) {
		openset[osnodeleft] = ny*100+nx;
		osnodeleft++;
		h_score[ny][nx] = hfunc(nx, ny, destx, desty);
		tentbetter = true;
	}
	else
		if(tentg < g_score[ny][nx])
			tentbetter = true;
	if(tentbetter) {
		camefrom[ny][nx] = y*100+x;
		g_score[ny][nx] = tentg;
		f_score[ny][nx] = g_score[ny][nx] + h_score[ny][nx];
	}
}

void Enemy::buildPath(int origx, int origy, int destx, int desty) { // create the waypoints involved in getting from (origx, origy) to (destx, desty)
	pathlen = 0;
	int cur = desty*100+destx;
	while(cur != origy*100+origx) {
		path[pathlen] = cur;
		pathlen++;
		cur = camefrom[cur/100][cur%100];
	}
	for(int i=0; i<pathlen/2; i++) {
		int temp = path[i];
		path[i] = path[pathlen-i-1];
		path[pathlen-i-1] = temp;
	}

	// trying to optimize pathing to go along diagonals when possible
	int newpath[100];
	newpath[0] = path[0];
	int newpathlen = 1;
	int cury = py/WORLD_BLOCK_SIZE, curx = px/WORLD_BLOCK_SIZE;
	for(int node=0; node<pathlen; node++) {
		// drawing line from current pos to to a waypoint, seeing if there's anything in the way
		if(lineBlocked((curx+.5)*WORLD_BLOCK_SIZE, (cury+.5)*WORLD_BLOCK_SIZE, (path[node]%100+.5)*WORLD_BLOCK_SIZE, (path[node]/100+.5))*WORLD_BLOCK_SIZE) {
			if(node==0) {
				newpath[newpathlen] = path[0];
				curx = path[0]%100;
				cury = path[0]/100;
			}
			else {
				newpath[newpathlen] = path[node-1];
				curx = path[node-1]%100;
				cury = path[node-1]/100;
			}
			newpathlen++;
		}
	}
	newpath[newpathlen] = path[pathlen-1];
	newpathlen++;
	for(int i=0; i<pathlen; i++)
		oldpath[i] = path[i];
	oldpathlen = pathlen;
	/*for(int i=0; i<newpathlen; i++)
		path[i] = newpath[i];
	pathlen = newpathlen;*/
	oldlocx=(floor(px/WORLD_BLOCK_SIZE)+.5)*WORLD_BLOCK_SIZE; oldlocy=(floor(py/WORLD_BLOCK_SIZE)+.5)*WORLD_BLOCK_SIZE;
}

void Enemy::findPath(int origx, int origy, int destx, int desty) { // determine the best path from (origx, origy) to (destx, desty) with A*
	openset[0] = origy*100+origx;
	csnodeleft = 0;
	osnodeleft = 1;
	g_score[origy][origx] = 0;
	h_score[origy][origx] = hfunc(origx, origy, destx, desty);
	f_score[origy][origx] = h_score[origy][origx];

	while(osnodeleft>0) {
		int minf = 0;
		for(int i=1; i<osnodeleft; i++) {
			if(f_score[openset[minf]/100][openset[minf]%100] > f_score[openset[i]/100][openset[i]%100])
				minf = i;
		}
		if(desty == openset[minf]/100 && destx == openset[minf]%100) {
			buildPath(origx, origy, destx, desty);
			return;
		}
		int cury = openset[minf]/100, curx = openset[minf]%100;
		if(!nodeincs(curx, cury)) {
			closedset[csnodeleft] = openset[minf];
			csnodeleft++;
		}
		for(int i=minf; i<osnodeleft-1; i++)
			openset[i] = openset[i+1];
		osnodeleft--;
		if(curx>0 && !BARRIER[cury][curx-1] && (kWorld[cury][curx-1] || kExplore[cury][curx-1]))
			expandNodes(curx, cury, curx-1, cury, destx, desty);
		if(curx<WORLD_WIDTH-1 && !BARRIER[cury][curx+1] && (kWorld[cury][curx+1] || kExplore[cury][curx+1]))
			expandNodes(curx, cury, curx+1, cury, destx, desty);
		if(cury>0 && !BARRIER[cury-1][curx] && (kWorld[cury-1][curx] || kExplore[cury-1][curx]))
			expandNodes(curx, cury, curx, cury-1, destx, desty);
		if(cury<WORLD_HEIGHT-1 && !BARRIER[cury+1][curx] && (kWorld[cury+1][curx] || kExplore[cury+1][curx]))
			expandNodes(curx, cury, curx, cury+1, destx, desty);
		minf = 0;
	}
}

bool Enemy::lineBlocked(float origx, float origy, float destx, float desty) { // returns whether or not a line from (origx, origy) to (destx, desty) is block by a wall
	/*
	if(origx==destx)
		destx += .01;
	float x0 = (origy*destx-origx*desty)/(destx-origx); // draw line from origin to destination
	float slope = (desty-origy)/(destx-origx);
	int dirx, diry;
	if(origx <= destx) // is the line going right or left
		dirx = 1;
	else
		dirx = -1;
	if(origy <= desty) // is the line going up or down
		diry = 1;
	else
		diry = -1;
	float curx = origx, cury = origy;
	while((destx-curx)*dirx >= 0 && (desty-cury)*diry >= 0) { // keep going until destination reached
		float dif = (((int)cury+(diry+1)*.5)-(slope*((int)curx+(dirx+1)*.5)+x0))*diry;
		if(dif > 0) { // line passes through the horizontal border
			curx += dirx;
			if(BARRIER[(int)(cury/WORLD_BLOCK_SIZE)][(int)(curx/WORLD_BLOCK_SIZE)])
				return true;
		}
		else {
			if(dif < 0) { // line passes through the vertical border
				cury += diry;
				if(BARRIER[(int)(cury/WORLD_BLOCK_SIZE)][(int)(curx/WORLD_BLOCK_SIZE)])

					return true;
			}
			else { // line passes right through the corner of the grid
				curx += dirx;
				cury += diry;
				if(BARRIER[(int)(cury/WORLD_BLOCK_SIZE)][(int)(curx/WORLD_BLOCK_SIZE)])
					return true;
				if(BARRIER[(int)(cury/WORLD_BLOCK_SIZE)-diry][(int)(curx/WORLD_BLOCK_SIZE)])
					return true;
				if(BARRIER[(int)(cury/WORLD_BLOCK_SIZE)][(int)(curx/WORLD_BLOCK_SIZE)-dirx])
					return true;
			}
		
		//alternate
		float dx, dy;
		if(dirx<0 && floor(curx)==curx)
			dx=-1; // special case
		else
			dx = floor(curx)-curx+(dirx+1)/2; // change in x to next vertical border
		if(diry<0 && floor(cury)==cury)
			dy=-1; // special case
		else
			dy = (floor(cury)-cury+(diry+1)/2); // change in y to next horizontal border

		if(abs(dx*slope)<abs(dy)) { // line passes through the vertical border
			if(BARRIER[(int)floor(cury/WORLD_BLOCK_SIZE)][(int)floor(curx/WORLD_BLOCK_SIZE)+dirx])
				return true;
			curx += dx;
			cury += dx*slope;
		}
		if(abs(dx*slope)>abs(dy)) { // line passes through the horizontal border
			if(BARRIER[(int)floor(cury/WORLD_BLOCK_SIZE)+diry][(int)floor(curx/WORLD_BLOCK_SIZE)])
				return true;
			curx += dy/slope;
			cury += dy;
		}
		if(abs(dx*slope)==abs(dy)) { // line passes through the intersection of borders
			if(BARRIER[(int)floor(cury/WORLD_BLOCK_SIZE)+diry][(int)floor(curx/WORLD_BLOCK_SIZE)])
				return true;
			if(BARRIER[(int)floor(cury/WORLD_BLOCK_SIZE)][(int)floor(curx/WORLD_BLOCK_SIZE)+dirx])
				return true;
			if(BARRIER[(int)floor(cury/WORLD_BLOCK_SIZE)+diry][(int)floor(curx/WORLD_BLOCK_SIZE)+dirx])
				return true;
			curx += dx;
			cury += dy;
		}
	}
	return false;
	*/

	// slow
	float x,y;
	bool blocked=false;
	char buf[10];
	for(float t=0; t<=1; t+=.1) {
		x = destx*t + origx*(1-t);
		y = desty*t + origy*(1-t);
		if(BARRIER[(int)(y/WORLD_BLOCK_SIZE)][(int)(x/WORLD_BLOCK_SIZE)]) {
			blocked = true;
			sprintf_s(debugstatus, 200, "t = %.2f, coords (%.2f, %.2f), unit coords(%d, %d), blocked = true", t, x, y, (int)(x/WORLD_BLOCK_SIZE), (int)(y/WORLD_BLOCK_SIZE));
		}
		else {
			sprintf_s(debugstatus, 200, "t = %.2f, coords (%.2f, %.2f), unit coords(%d, %d), blocked = false", t, x, y, (int)(x/WORLD_BLOCK_SIZE), (int)(y/WORLD_BLOCK_SIZE));
		}
	}
	return blocked;
	

	/*
	// parametric version
	if(origx==destx)
		destx+=1;
	if(origy==desty)
		desty+=1;
	float curx=origx, cury=origy, t=0;

	if(destx>origx && desty>origy) {
		while(t<=1) {
			if(((floor(x)+dirx)-origx)/(destx-origx) < ())

			curx += destx*t + origx*(1-t);
			y = desty*t + origy*(1-t);
		}
	}
	*/
}

Enemy::Enemy(): Pawn() {
	nextlocx=px; nextlocy=py;
	lastlocx=px; lastlocy=py;
	still=0;
	destx=px; desty=py;
	waypoint=0;
	curCoin=-1;
	traveling=false;
	dodging=false;
	fleeing=false;
	attacking=false;
	stopped=true;
	stoptimer=0;

	// vars for known world, squares to explore next
	for(int i=0; i<WORLD_HEIGHT; i++) {
		for(int j=0; j<WORLD_WIDTH; j++) {
			kWorld[i][j] = false;
		}
	}
	for(int i=0; i<WORLD_HEIGHT; i++) {
		for(int j=0; j<WORLD_WIDTH; j++) {
			kExplore[i][j] = false;
		}
	}
	int bpx = floor(px/WORLD_BLOCK_SIZE), bpy = floor(py/WORLD_BLOCK_SIZE);
	kWorld[bpy][bpx] = true;
	rooms[bpy][bpx].setWorld(true);

	/*
	if(bpy>=1 && !BARRIER[bpy-1][bpx]) {
		kExplore[bpy-1][bpx] = true;
		rooms[bpy-1][bpx].setExplore(true);
	}
	if(bpx>=1 && !BARRIER[bpy][bpx-1]) {
		kExplore[bpy][bpx-1] = true;
		rooms[bpy][bpx-1].setExplore(true);
	}
	if(bpy<WORLD_HEIGHT-1 && !BARRIER[bpy+1][bpx]) {
		kExplore[bpy+1][bpx] = true;
		rooms[bpy+1][bpx].setExplore(true);
	}
	if(bpx<WORLD_WIDTH-1 && !BARRIER[bpy][bpx+1]) {
		kExplore[bpy][bpx+1] = true;
		rooms[bpy][bpx+1].setExplore(true);
	}
	*/

	timeout=0;
	everyFive=0;
	osnodeleft=1;
	csnodeleft=0;
	knownPlayer[0]=0; knownPlayer[1]=0; knownPlayer[2]=0; knownPlayer[3]=0; knownPlayer[4]=21;
	for(int i=0; i<MAX_COINS; i++)
		kCoins[i] = false;
	for(int i=0; i<MAX_BALLS; i++)
		kBalls[i] = false;
	sprintf_s(status, 200, "Enemy Status: Unknown");
	oldlocx=px; oldlocy=py;
	blink=0;
	ai();
}

Enemy::Enemy(float ipx, float ipy, float ipz, float ivx, float ivy, float ivz, float ipox, float ipoy, float ivo, BallSet *ib, CoinSet *ic, Player *iplayer): Pawn(ipx, ipy, ipz, ivx, ivy, ivz, ipox, ipoy, ivo, ib, ic) { // (ipx, ipy, ipz) position, (ivx, ivy, ivz) velocity, ipox, ipoy orientations, ivo orientation velocity, pointers to the ball set, coin set, and player
	nextlocx=px; nextlocy=py;
	lastlocx=px; lastlocy=py;
	still=0;
	destx=px; desty=py;
	waypoint=0;
	curCoin=-1;
	traveling=false;
	dodging=false;
	fleeing=false;
	attacking=false;
	stopped=true;
	stoptimer=0;
	
	// vars for known world, squares to explore next
	for(int i=0; i<WORLD_HEIGHT; i++) {
		for(int j=0; j<WORLD_WIDTH; j++) {
			kWorld[i][j] = false;
		}
	}
	for(int i=0; i<WORLD_HEIGHT; i++) {
		for(int j=0; j<WORLD_WIDTH; j++) {
			kExplore[i][j] = false;
		}
	}
	int bpx = floor(px/WORLD_BLOCK_SIZE), bpy = floor(py/WORLD_BLOCK_SIZE);
	kWorld[bpy][bpx] = true;
	rooms[bpy][bpx].setWorld(true);

	if(bpy>=1 && !BARRIER[bpy-1][bpx]) {
		kExplore[bpy-1][bpx] = true;
		rooms[bpy-1][bpx].setExplore(true);
	}
	if(bpx>=1 && !BARRIER[bpy][bpx-1]) {
		kExplore[bpy][bpx-1] = true;
		rooms[bpy][bpx-1].setExplore(true);
	}
	if(bpy<WORLD_HEIGHT-1 && !BARRIER[bpy+1][bpx]) {
		kExplore[bpy+1][bpx] = true;
		rooms[bpy+1][bpx].setExplore(true);
	}
	if(bpx<WORLD_WIDTH-1 && !BARRIER[bpy][bpx+1]) {
		kExplore[bpy][bpx+1] = true;
		rooms[bpy][bpx+1].setExplore(true);
	}

	timeout=0;
	everyFive=0;
	osnodeleft=1;
	csnodeleft=0;
	player = iplayer;
	knownPlayer[0]=0; knownPlayer[1]=0; knownPlayer[2]=0; knownPlayer[3]=0; knownPlayer[4]=21;
	for(int i=0; i<MAX_COINS; i++)
		kCoins[i] = false;
	for(int i=0; i<MAX_BALLS; i++)
		kBalls[i] = false;
	sprintf_s(status, 200, "Enemy Status: Unknown");
	oldlocx=px; oldlocy=py;
	blink=0;
	
	pathlen=0;
	oldpathlen=0;

	ai();
}

char *Enemy::giveStatus() { // return a string of text describing what the enemy is doing
	return status;
}

char *Enemy::giveDebugStatus() { // return a debug version of a string of text describing what the enemy is doing
	return debugstatus;
}

void Enemy::update(float dt) { // called as often as desired. dt is the time that has passed since last call
	if(frozen) {
		ftimer-=dt; // if frozen, can't move, use countdown timer
		if(ftimer <= 0)
			frozen = false;
	}
	else {
		ai(); // first run AI alg

		while(pox>PI)
			pox-=2*PI;
		while(pox<=-PI)
			pox+=2*PI;
		float ang = atan2(nextlocy-py, nextlocx-px); // where should the enemy try to turn?
		if(abs(ang-pox) > MAX_ROT_VEL*ROTATION_MULTIPLIER*dt)
			vo = MAX_ROT_VEL;
		else {
			if(dt > 0)
				vo = abs(ang-pox)/ROTATION_MULTIPLIER/dt;
			else
				vo=0;
		}
		if((ang-pox>PI || ang-pox<0) && pox-ang<=PI)
			vo*=-1;
		pox += vo*ROTATION_MULTIPLIER*dt;

		if((pox>=ang && pox-ang<PI/2) || (pox<ang && ang-pox<PI/2)) {
			vx=cos(pox); vy=sin(pox); vz=0;
			normalize(vx, vy, vz);
			vx *= MAX_VEL; vy *= MAX_VEL; vz *= MAX_VEL;
			px += vx*MOVEMENT_MULTIPLIER*dt; py += vy*MOVEMENT_MULTIPLIER*dt; pz += vz*MOVEMENT_MULTIPLIER*dt;
		}
		else {
			vx=0; vy=0; vz=0;
		}
	}

	bool inside = false; // check collision; if no longer in a room move player back to valid location
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
		mpx-=px; mpy-=py; mpz-=pz; // less efficient than player version? check to see if equivalent
		normalize(mpx, mpy, mpz);
		Enemy::move(dist, mpx, mpy, mpz);
	}
	
	if(balls->hit(px, py, pz, PLAYER_RADIUS, PLAYER_HEIGHT)) { // check if a ball hit
		freeze();
	}

	if(coins->takeCoin(px, py, pz, PLAYER_RADIUS, PLAYER_HEIGHT)) { // check if a coin was taken
		score++;
	}

	/*bool cw;
	if(vx==0) {
		if((vy>0&&ox<0) || (vy<0&&ox>0))
			cw=true;
		else
			cw=false;
	}
	else {
		if(ox==0) {
			if((oy>0&&vx>0) || (oy<0&&vx<0))
				cw=true;
			else
				cw=false;
		}
		else {
			if(vy/vx>oy/ox) {
				if(ox>0)
					cw=false;
				else
					cw=true;
			}
			else {
				if(oy>0)
					cw=true;
				else
					cw=false;
			}
		}
	}*/

	int bpx = floor(px/WORLD_BLOCK_SIZE), bpy = floor(py/WORLD_BLOCK_SIZE); // update known information about the world
	kWorld[bpy][bpx] = true; // is in the square, so it knows it now
	rooms[bpy][bpx].setWorld(true);
	kExplore[bpy][bpx] = false;
	rooms[bpy][bpx].setExplore(false);

	/*
	if(kExplore[bpy][bpx]) {
		kExplore[bpy][bpx] = false; // is in the square, so it no longer needs to explore it
		rooms[bpy][bpx].setExplore(false);

		// if adjacent squares are new, add them to explore list
		if(bpy>=1 && !BARRIER[bpy-1][bpx] && !kWorld[bpy-1][bpx]) {
			kExplore[bpy-1][bpx] = true;
			rooms[bpy-1][bpx].setExplore(true);
		}
		if(bpx>=1 && !BARRIER[bpy][bpx-1] && !kWorld[bpy][bpx-1]) {
			kExplore[bpy][bpx-1] = true;
			rooms[bpy][bpx-1].setExplore(true);
		}
		if(bpy<WORLD_HEIGHT-1 && !BARRIER[bpy+1][bpx] && !kWorld[bpy+1][bpx]) {
			kExplore[bpy+1][bpx] = true;
			rooms[bpy+1][bpx].setExplore(true);
		}
		if(bpx<WORLD_WIDTH-1 && !BARRIER[bpy][bpx+1] && !kWorld[bpy][bpx+1]) {
			kExplore[bpy][bpx+1] = true;
			rooms[bpy][bpx+1].setExplore(true);
		}
	}
	*/

	for(int i=bpy-2; i<=bpy+2; i++) { // update debugging information
		for(int j=bpx-2; j<=bpx+2; j++) {
			if(i>=0 && i<WORLD_HEIGHT && j>=0 && j<WORLD_WIDTH && !BARRIER[i][j] && inView(j*(WORLD_BLOCK_SIZE+.5), i*(WORLD_BLOCK_SIZE+.5))) {
				if(!kWorld[i][j]) {
					kExplore[i][j] = true;
					rooms[i][j].setExplore(true);
				}
				rooms[i][j].setView(true);
			}
			else
				rooms[i][j].setView(false);
		}
	}

	blink+=.008; // blink objective point
	if(blink>=1)
		blink-=1;
}

void Enemy::draw2d(bool debug) { // draw the enemy 2d (for the map)
	if(debug) {
		glPushMatrix();

		glTranslatef(nextlocx, nextlocy, 0);

		glColor3f(sin(PI*blink),0,0);
		glPointSize(4);
		glBegin(GL_POINTS);
		glVertex2f(0,0);
		glEnd();

		glPopMatrix();
	}

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
		glColor3f(1,.5,1);
	
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

	if(debug) {
		GLubyte halftone[] = {0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55};
		glEnable(GL_POLYGON_STIPPLE);
		glPolygonStipple(halftone);
		glColor3f(1,1,1);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(0, 0);
		float ang=-VIEW_ANGLE/2;
		glVertex2f(VIEW_DIST*cos(ang), VIEW_DIST*sin(ang));
		for(int i=0; i<16; i++) {
			ang+=VIEW_ANGLE/16;
			glVertex2f(VIEW_DIST*cos(ang), VIEW_DIST*sin(ang));
		}
		glEnd();
		glDisable(GL_POLYGON_STIPPLE);
	}
	glPopMatrix();

	if(debug) {
		glPushMatrix();

		glColor3f(0,.5,.5);
		glBegin(GL_LINE_STRIP);
		glVertex2f(oldlocx, oldlocy);
		for(int i=0; i<oldpathlen; i++)
			glVertex2f((oldpath[i]%100+.5)*WORLD_BLOCK_SIZE, (oldpath[i]/100+.5)*WORLD_BLOCK_SIZE);
		glEnd();
		glPopMatrix();
	}
}

void Enemy::draw3d(bool debug) { // draw the enemy 3d
	if(debug) {
		glPushMatrix();

		glTranslatef(nextlocx, nextlocy, -pz+OUTLINE_OFFSET);

		glColor3f(sin(PI*blink),0,0);
		glPointSize(4);
		glBegin(GL_POINTS);
		glVertex3f(0,0,0);
		glEnd();

		glPopMatrix();
	}

	if(render3d) {
		glPushMatrix();

		glTranslatef(px, py, pz);

		glPushMatrix();
		glRotatef(90, 1, 0, 0);
		glRotatef(180*pox/PI, 0, 1, 0);

		if(frozen)
			glColor3f(0,0,0);
		else
			glColor3f(1,.5,1);
	
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

	if(debug) {
		glPushMatrix();

		glColor3f(0,.5,.5);
		glBegin(GL_LINE_STRIP);
		glVertex3f(oldlocx, oldlocy, OUTLINE_OFFSET);
		for(int i=0; i<oldpathlen; i++)
			glVertex3f((oldpath[i]%100+.5)*WORLD_BLOCK_SIZE, (oldpath[i]/100+.5)*WORLD_BLOCK_SIZE, OUTLINE_OFFSET);
		glEnd();
		glPopMatrix();

		glPushMatrix();

		glTranslatef(px, py, pz-12);

		glPushMatrix();
		glRotatef(90, 1, 0, 0);
		glRotatef(180*pox/PI, 0, 1, 0);

		GLubyte halftone[] = {0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
                              0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55};
		glEnable(GL_POLYGON_STIPPLE);
		glPolygonStipple(halftone);
		glColor3f(1,1,1);
		glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0,0,0);
		float ang=-VIEW_ANGLE/2;
		glVertex3f(VIEW_DIST*cos(ang), 0, VIEW_DIST*sin(ang));
		for(int i=0; i<16; i++) {
			ang+=VIEW_ANGLE/16;
			glVertex3f(VIEW_DIST*cos(ang), 0, VIEW_DIST*sin(ang));
		}
		glEnd();
		glDisable(GL_POLYGON_STIPPLE);

		glPopMatrix();

		glPopMatrix();
	}
}

Enemy::~Enemy() {}