#include <string>
#include <utility>
#include <windows.h>
#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include "constants.h"

/////////////////////////////////////////////////
//////////////VARS FOR WINDOW+FONT///////////////
float x=0,y=1.75,z=5;
float lx=0,ly=0,lz=-1;

int h,w; // for window resolution
float ratio; // w/h
enum CamFollow {PLAYER, ENEMY};
CamFollow cf = PLAYER; // which one the camera will follow around
bool paused=false; // pause the game
bool help=false; // show help
bool debug=false; // show debug info for enemy
bool map=false; // show the map large

int font = (int)GLUT_BITMAP_HELVETICA_12; // for text
int bitmapHeight=12; // text size
/////////////////////////////////////////////////
/////////////////////////////////////////////////



/////////////////////////////////////////////////
/////////////////UTILITY FUNCS///////////////////
float length(float ax, float ay, float az) {
	return sqrt(ax*ax+ay*ay+az*az);
}

void normalize(float &ax, float &ay, float &az) {
	if(ax==0 && ay==0 && az==0)
		return;
	float mag = sqrt(ax*ax+ay*ay+az*az);
	ax /= mag;
	ay /= mag;
	az /= mag;
}
/////////////////////////////////////////////////
/////////////////////////////////////////////////



/////////////////////////////////////////////////
/////////////OTHER CLASSES AND VARS//////////////
int numRooms = 0;

#include "room.h"

Room rooms[WORLD_HEIGHT][WORLD_WIDTH];

#include "ball.h"
#include "coin.h"
#include "enemy.h"
#include "player.h"

GLuint room;

BallSet balls = BallSet();

int ccoords[4][3] = {  // to set coin locations
	{WORLD_BLOCK_SIZE*(.5),WORLD_BLOCK_SIZE*(.5),COIN_DEFAULT_HEIGHT},
	{WORLD_BLOCK_SIZE*(WORLD_WIDTH-.5),WORLD_BLOCK_SIZE*(.5),COIN_DEFAULT_HEIGHT},
	{WORLD_BLOCK_SIZE*(.5),WORLD_BLOCK_SIZE*(WORLD_HEIGHT-.5),COIN_DEFAULT_HEIGHT},
	{WORLD_BLOCK_SIZE*(WORLD_WIDTH-.5),WORLD_BLOCK_SIZE*(WORLD_HEIGHT-.5),COIN_DEFAULT_HEIGHT}
};

CoinSet coins = CoinSet(ccoords, 4);

Player *player;
Enemy *enemy;
/////////////////////////////////////////////////
/////////////////////////////////////////////////



/////////////////////////////////////////////////
//////////////////FUNC FOR TEXT//////////////////
void renderSpacedBitmapString(float x, float y, int spacing, void *font, char *string) { // display the string at (x,y) with spacing pixels in between letters
	char *c;
	int x1=x;
	glDisable(GL_TEXTURE_2D);
	glScalef(.1,.1,.1);
	glLineWidth(.5);
	glPushMatrix();
	for(c=string; *c!='\0'; c++) {
		if(*c=='\n') {
			x1 = x;
			y = y-(bitmapHeight*1.25);
		}
		//glRasterPos2f(x1,y);
		//glutBitmapCharacter(font,*c);
		glutStrokeCharacter((void *)(int)GLUT_STROKE_ROMAN,*c);
		//x1 += glutBitmapWidth(font,*c) + spacing;
	}
	glPopMatrix();
	glScalef(10,10,10);
	glEnable(GL_TEXTURE_2D);
}
/////////////////////////////////////////////////
/////////////////////////////////////////////////



/////////////////////////////////////////////////
/////////////////GL PASSED FUNCS/////////////////
void display(void)
{
	//clock_t endtime = clock() + SLEEP_TIME_MILI/1000 * CLOCKS_PER_SEC;
	glViewport(0,0,w,h); // x,y,width,height
	glClearColor(0.0,0.0,0.0,1.0); //clear the color of the window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the Color Buffer

	// draw 3d stuff
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(60,ratio,1,2560); // fovy, aspect, near, far  (set the clipping volume)

	//------------------3D------------------//
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glViewport(0,0,w,h); // x,y,width,height

	if(!player->isInvisible() && cf==PLAYER) {
		glRotatef(-90,1,0,0);
		glRotatef(-180*player->oy()/PI,1,0,0);
		glRotatef(90-180*player->ox()/PI,0,0,1);
		glTranslatef(-player->x(),-player->y(),-player->z()); // move the camera to follow the player around
	}
	else {
		glRotatef(-90,1,0,0);
		glRotatef(-180*enemy->oy()/PI,1,0,0);
		glRotatef(90-180*enemy->ox()/PI,0,0,1);
		glTranslatef(-enemy->x(),-enemy->y(),-enemy->z()); // move the camera to follow the enemy around
	}

	// start drawing stuff
	for(int i=0; i<WORLD_HEIGHT; i++) {
		for(int j=0; j<WORLD_WIDTH; j++) {
			if(!BARRIER[i][j])
				rooms[i][j].draw3d(debug);
		}
	}
	coins.draw3d(debug);
	enemy->draw3d(debug);
	balls.draw3d(debug);

	glPopMatrix();
	//----------------End 3D----------------//

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// draw 2d stuff
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0,w,0,h); // left, right, bottom, top  (set a 2D orthographic projection)

	//-----------------Map------------------//
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if(map) // if large map is enabled
		glViewport(w*(1-MAP_SIZE_RATIO_LARGE),h*(1-MAP_SIZE_RATIO_LARGE),w*(2*MAP_SIZE_RATIO_LARGE-1),h*(2*MAP_SIZE_RATIO_LARGE-1)); // x,y,width,height
	else
		glViewport(w*(1-MAP_SIZE_RATIO),0,w*MAP_SIZE_RATIO,h*MAP_SIZE_RATIO); // x,y,width,height

	if(cf==PLAYER)
		glTranslatef(-player->x()+w/2,-player->y()+h/2,0); // move the camera to follow the player around
	else
		glTranslatef(-enemy->x()+w/2,-enemy->y()+h/2,0); // otherwise follow enemy

	glColor3f(.25,1,.25); // background
	glBegin(GL_QUADS);
	glVertex2f(-1024,-1024);
	glVertex2f(3072,-1024);
	glVertex2f(3072,2048);
	glVertex2f(-1024,2048);
	glEnd();

	// start drawing 2d stuff
	for(int i=0; i<WORLD_HEIGHT; i++) {
		for(int j=0; j<WORLD_WIDTH; j++) {
			if(!BARRIER[i][j])
				rooms[i][j].draw2d(debug);
		}
	}

	coins.draw2d(debug);
	enemy->draw2d(debug);
	player->draw2d(debug);
	balls.draw2d(debug);

	glLoadIdentity();
	glColor3f(1,1,1);
	glLineWidth(4);
	glBegin(GL_LINE_LOOP); // white outline of the map
	glVertex2f(0,0);
	glVertex2f(w,0);
	glVertex2f(w,h);
	glVertex2f(0,h);
	glEnd();

	glPopMatrix();
	//---------------End Map----------------//

	//-----------------Text-----------------//
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glViewport(0,0,w,h); // x,y,width,height

	// text for score, debugging, etc.
	glTranslatef(12, h-24, 0);

	glColor3f(.25,.25,.25);
	char scoretext[200];
	sprintf_s(scoretext, 200, "Player Score: %d       Enemy Score: %d       Enemy Orientation: (%.2f, %.2f)", player->sc(), enemy->sc(), cos(enemy->ox()), sin(enemy->ox()));
	glColor3f(1,1,1);
	renderSpacedBitmapString(0,0,0,(void *)font,scoretext);
	glTranslatef(-2, 2, 0);
	glColor3f(.25,.25,.25);
	renderSpacedBitmapString(0,0,0,(void *)font,scoretext);
	glTranslatef(2, -18, 0);
	glColor3f(1,1,1);
	renderSpacedBitmapString(0,0,0,(void *)font,enemy->giveStatus());
	glTranslatef(-2, 2, 0);
	glColor3f(.25,.25,.25);
	renderSpacedBitmapString(0,0,0,(void *)font,enemy->giveStatus());
	glTranslatef(2, -18, 0);
	glColor3f(1,1,1);
	renderSpacedBitmapString(0,0,0,(void *)font,enemy->giveDebugStatus());
	glTranslatef(-2, 2, 0);
	glColor3f(.25,.25,.25);
	renderSpacedBitmapString(0,0,0,(void *)font,enemy->giveDebugStatus());
	if(help) {
		glTranslatef(66, -66, 0);

		//char helptext[] = "Keys:\n  Esc - Exit\n  F1 - Toggle this help\n  1 - Toggle player on/off\n  2 - Toggle view to player/enemy\n  3 - Toggle pause\n  4 - Toggle debug info\n  W/S - Move forward/backward\n  A/D - Turn left/right\n  F - Fire ball";
		//renderSpacedBitmapString(0,0,0,(void *)font,helptext);
		
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"Keys:"); // display text twice for each to get a shadow underneath, making it more readable
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"Keys:");
		glTranslatef(2, -18, 0);
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"  Esc - Exit");
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"  Esc - Exit");
		glTranslatef(2, -18, 0);
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"  F1 - Toggle this help");
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"  F1 - Toggle this help");
		glTranslatef(2, -18, 0);
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"  1 - Toggle player on/off");
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"  1 - Toggle player on/off");
		glTranslatef(2, -18, 0);
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"  2 - Toggle view to player/enemy");
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"  2 - Toggle view to player/enemy");
		glTranslatef(2, -18, 0);
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"  3 - Toggle pause");
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"  3 - Toggle pause");
		glTranslatef(2, -18, 0);
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"  4 - Toggle debug info");
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"  4 - Toggle debug info");
		glTranslatef(2, -18, 0);
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"  W/S - Move forward/backward");
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"  W/S - Move forward/backward");
		glTranslatef(2, -18, 0);
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"  A/D - Move left/right");
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"  A/D - Move left/right");
		glTranslatef(2, -18, 0);
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"  Spacebar - Jump");
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"  Spacebar - Jump");
		glTranslatef(2, -18, 0);
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"  F - Fire ball");
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"  F - Fire ball");
		glTranslatef(2, -18, 0);
		glColor3f(1,1,1);
		renderSpacedBitmapString(0,0,0,(void *)font,"  M - Show map");
		glTranslatef(-2, 2, 0);
		glColor3f(.25,.25,.25);
		renderSpacedBitmapString(0,0,0,(void *)font,"  M - Show map");
	}

	glPopMatrix();
	//---------------End Text---------------//

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutSwapBuffers();
}

//#ifdef _WIN32
//	static DWORD last_idle_time;
//#else
clock_t last_idle_time;//static struct timeval last_idle_time;
//#endif

void collision() { // this function moves the player and enemy apart equally if they have bumped into each other
	if(abs(player->z()-enemy->z()) <= PLAYER_HEIGHT-MAX_VERTICAL_COLLISION) { // if z values are close enough for horizontal collision
		float tempx = player->x()-enemy->x(), tempy = player->y()-enemy->y(), tempz = 0;
		float tempdist = length(tempx, tempy, 0);
		if(tempdist < 2*PLAYER_RADIUS) {
			normalize(tempx, tempy, tempz);
			enemy->move(PLAYER_RADIUS-tempdist/2, -tempx, -tempy, 0);
			player->move(PLAYER_RADIUS-tempdist/2, tempx, tempy, 0);
		}
	}
	else { // if z values are different enough so one is actually on top of the other
		if(abs(player->z()-enemy->z()) < PLAYER_HEIGHT) {
			player->move((PLAYER_HEIGHT-abs(player->z()+enemy->z()))/2,0,0,player->z()-enemy->z());
			enemy->move((PLAYER_HEIGHT-abs(player->z()+enemy->z()))/2,0,0,enemy->z()-player->z());
			if(player->z()<PLAYER_HEIGHT/2) {
				player->move(PLAYER_HEIGHT/2-player->z(),0,0,1);
				enemy->move(PLAYER_HEIGHT/2-player->z(),0,0,1);
			}
			if(enemy->z()<PLAYER_HEIGHT/2) {
				player->move(PLAYER_HEIGHT/2-enemy->z(),0,0,1);
				enemy->move(PLAYER_HEIGHT/2-enemy->z(),0,0,1);
			}
		}
	}
}

void updateworld(void) // runs when the GL engine isn't doing anything else
{
	float dt; // used to measure time so that faster cpus don't run the game at a faster speed
	/*#ifdef _WIN32
		DWORD time_now;
		time_now = GetTickCount();
		dt = (float) (time_now - last_idle_time) / 1000.0;
	#else*/
		// Figure out time elapsed since last call to idle function
		clock_t time_now = clock();/*struct timeval time_now;
		gettimeofday(&time_now, NULL);
		dt = (float)(time_now.tv_sec  - last_idle_time.tv_sec) +
		1.0e-6*(time_now.tv_usec - last_idle_time.tv_usec);*/
		dt = SPEED_MODIFIER*(time_now-last_idle_time)/CLOCKS_PER_SEC;
	//#endif

	if(!paused) {
		balls.update(dt); // start updating actual state (not graphics)
		coins.update(dt);

		enemy->update(dt);
		
		player->update(dt);

		if(!player->isInvisible()) // if player is invisible, can't bump into him
			collision();
	}

	glutWarpPointer(w/2,h/2); // move cursor back to the middle of the screen so that we can measure mouse movement accurately
	last_idle_time = time_now;
	// Force redraw
	glutPostRedisplay();
}

void reshape(int w1, int h1)
{
	// Prevent a divide by zero, when window is too short (you cant make a window of zero width)
	if(h1==0)
		h1=1;
	w=w1;
	h=h1;
	ratio = 1.0*w/h;
	glViewport(0,0,w,h); // x, y, width, height  (set the viewport to be the entire window)
	glMatrixMode(GL_PROJECTION); // Reset the coordinate system before modifying
	glLoadIdentity();
	gluPerspective(90,ratio,.01,2560); // fovy, aspect, near, far  (set the clipping volume)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void primitives(void) // not in use right now
{
	room = glGenLists(1);
	glNewList(room, GL_COMPILE); //compile the new list
	
	//glScalef(.2, .2, .2);

	//---------------------
	glPushMatrix();

	// x and y sides plus lines on edges
	for(int i=0; i<WORLD_HEIGHT; i++) {
		for(int j=0; j<WORLD_WIDTH; j++) {
			if(!BARRIER[i][j])
				rooms[i][j].draw2d(debug);
		}
	}
	glPopMatrix();
	
	glEndList();
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key) {
		case 27: // escape key
		//glutLeaveGameMode(); //set the resolution how it was
		exit(0); //quit the program
		break;

		case '1': // 1 is to toggle invisibility
		player->togInvisible();
		if(player->isInvisible()) { // if invisibility on, can't focus camera on player, focus on enemy
			cf=ENEMY;
			player->setRender3d(false); // player is invisible
			enemy->setRender3d(false); // looking from enemy's POV, so shouldn't render the enemy
		}
		else { // if player isn't invisible
			if(cf==PLAYER) {
				player->setRender3d(false); // player POV, stop rendering player
				enemy->setRender3d(true); // enemy should be rendered
			}
			else {
				player->setRender3d(true); // not player POV, show player
				enemy->setRender3d(false); // but not enemy
			}
		}
		break;

		case '2': // toggle enemy view
		if(cf==PLAYER)
			cf=ENEMY;
		else
			cf=PLAYER;
		if(player->isInvisible()) { // if player is invisible, can't focus on him
			cf=ENEMY;
			player->setRender3d(false); // player invisible
			enemy->setRender3d(false); // focusing on enemy, don't render
		}
		else { // player isn't invisible
			if(cf==PLAYER) {
				player->setRender3d(false); // don't render player
				enemy->setRender3d(true); // render enemy
			}
			else {
				player->setRender3d(true); // enemy POV, show player
				enemy->setRender3d(false); // but not enemy
			}
		}
		break;
	
		case '3': // toggle pause
		paused = !paused;
		break;

		case '4': // toggle debug mode (shows enemy information graphically)
		debug = !debug;
		break;

		case 'm': // toggle large map
		map = !map;
		break;
	
		default: // any other key is to control player movement (or doesn't do anything)
		player->keyboard(key,x,y);
		break;
	}
}

void keyboardUp(unsigned char key, int x, int y) // used to make it so holding down a key immediately starts an interrupt and continues signal, needed for player movement
{
	player->keyboardUp(key,x,y);
}

void keyboardSpecialKeys(int key, int x, int y) // F1 is a key that can only be accessed through this GL function
{
	if(key == GLUT_KEY_F1) { // toggles help
		help=!help;
	}
}

void keyboardSpecialKeysUp(int key, int x, int y) // don't need immediate interrupts for "special" keys (yet)
{
	// nothing for now
}

void mouse(int x, int y) // send mouse movement information to player instance
{
	player->mouse(x-w/2,y-h/2); // we warp the cursor back to (w/2,h/2) periodically, so we an measure the distance from there to get total mouse movement
}
/////////////////////////////////////////////////
/////////////////////////////////////////////////



/////////////////////////////////////////////////
///////////////////MAIN FUNC/////////////////////
//int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
int main(int argc, char **argv)
{
	for(int i=0; i<WORLD_HEIGHT; i++) { // actually construct the level
		for(int j=0; j<WORLD_WIDTH; j++) {
			if(!BARRIER[i][j]) {
				rooms[i][j] = Room(j*WORLD_BLOCK_SIZE, i*WORLD_BLOCK_SIZE, 0, WORLD_BLOCK_SIZE, WORLD_BLOCK_SIZE, WORLD_UP_HEIGHT, i!=WORLD_HEIGHT-1 && !BARRIER[i+1][j], j!=0 && !BARRIER[i][j-1], j!=WORLD_WIDTH-1 && !BARRIER[i][j+1], i!=0 && !BARRIER[i-1][j], i!=WORLD_HEIGHT-1 && j!=0 && !BARRIER[i+1][j-1], i!=WORLD_HEIGHT-1 && j!=WORLD_WIDTH-1 && !BARRIER[i+1][j+1], i!=0 && j!=0 && !BARRIER[i-1][j-1], i!=0 && j!=WORLD_WIDTH-1 && !BARRIER[i-1][j+1]);
				numRooms++;
			}
		}
	}
	player = &Player(64, 320, PLAYER_DEFAULT_HEIGHT, 0, 0, 0, 0, 0, &balls, &coins); // instantiate player and enemy
	enemy = &Enemy(1088, 960, PLAYER_DEFAULT_HEIGHT, 0, 0, 0, 0, 0, 0, &balls, &coins, player);
	//#ifdef _WIN32
	//	last_idle_time = GetTickCount();
	//#else
		//gettimeofday (&last_idle_time, NULL);
	last_idle_time = clock();
	//#endif

	//char *argv[] = {"iankennard.exe"};
	//int argc = 1; // must/should match the number of strings in argv

	glutInit(&argc, argv); //initialize the program.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 600); //set the window size
	w=800; h=600;
	glutInitWindowPosition(100, 100); //set the position of the window
	glutCreateWindow("Ian Kennard"); //the caption of the window
	//glutGameModeString("640x480:32@75"); //the settings for fullscreen mode
	//glutEnterGameMode(); //set glut to fullscreen using the settings in the line above

	glEnable(GL_DEPTH_TEST); //enable the depth testing

	glClearDepth(1);
	glClearColor(0.0,0.0,0.0,1.0); //clear the color of the window

	// for text
	//glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//primitives();

	glutDisplayFunc(display); //call the display function to draw our world
	glutIdleFunc(updateworld); //update any variables in display, display can be changed to anything, as long as you move the variables to be updated, in this case, angle++;
	glutReshapeFunc(reshape);

	// (these are needed for player movement)
	glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);
	glutIgnoreKeyRepeat(1); // don't call keyboard functions for each autorepeat
	glutKeyboardFunc(keyboard); //the call for the keyboard function.
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(keyboardSpecialKeys);
	glutSpecialUpFunc(keyboardSpecialKeysUp);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutPassiveMotionFunc(mouse);

	glutMainLoop(); //initialize the OpenGL loop cycle

	// after exiting the main loop, put settings back to default
	glutIgnoreKeyRepeat(0);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	return 0;
}
/////////////////////////////////////////////////
/////////////////////////////////////////////////