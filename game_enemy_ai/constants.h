// This is where most constants that are used globally are defined.

#ifndef IANKENNARD_CONSTANTS_H
#define IANKENNARD_CONSTANTS_H

#include <math.h>
#include "room.h"

const float PI = 3.141592654; // define pi so we don't have to keep calculating it

const int INITIAL_SCORE = 10; // the score that each player starts out with

const float SPEED_MODIFIER = 80; // adjust this number to increase/decrease the rate at which the game runs

const float MOVEMENT_MULTIPLIER = 2; // adjust this number to increase/decrease player movement speed
const float ROTATION_MULTIPLIER = PI/60; // *DEPRECATED* adjust this number to increase/decrease rotational sensitivity
const float FLOAT_ERROR = .0001; // *DEPRECATED* defines the minimum difference between two numbers to be considered equal
const float ANGLE_MULTIPLIER = PI/90; // *DEPRECATED* also used for rotational sensitivity
const float COS_ANGLE_MULTIPLIER = cos(ANGLE_MULTIPLIER); // *DEPRECATED* define so we don't have to keep calculating it
const float SIN_ANGLE_MULTIPLIER = sin(ANGLE_MULTIPLIER); // *DEPRECATED* define so we don't have to keep calculating it
const float MAX_ANGLE = PI/2-PI/180; // the maximum angle a player can look up or down (used so that the player view can't turn upside down
const float COS_MAX_ANGLE = cos(MAX_ANGLE); // define so we don't have to keep calculating it
const float SIN_MAX_ANGLE = sin(MAX_ANGLE); // define so we don't have to keep calculating it
const float MOUSE_SENSITIVITY = .005; // adjust this number to increase/decrease mouse sensitivity
const float BALL_VEL = 12; // adjust this number to increase/decrease ball velocity
const float BALL_OFF = 50; // how far away from the player a ball spawns (to prevent an instant hit with own ball)
const float JUMP_VEL = 10; // adjust this number to increase/decrease jump velocity

// these constants define the layout of the game space
const int WORLD_HEIGHT = 8;
const int WORLD_WIDTH = 16;
const int WORLD_BLOCK_SIZE = 128;
const bool BARRIER[WORLD_HEIGHT][WORLD_WIDTH] = {{false, true,  false, false, true,  true,  false, false, false, false, false, false, true,  false, false, false},
												 {false, true,  false, false, true,  false, false, false, false, false, false, false, true,  false, false, false},
												 {false, false, false, false, false, false, false, false, false, true,  false, true,  true,  false, false, false},
												 {false, false, false, false, false, false, true,  false, true,  true,  false, false, false, false, false, false},
												 {true,  true,  true,  false, false, false, true,  false, true,  true,  false, false, false, false, false, false},
												 {false, false, true,  false, false, true,  true,  false, false, false, false, true,  true,  false, false, false},
												 {false, false, false, false, false, true,  false, false, false, false, false, false, true,  false, false, false},
												 {false, false, false, false, false, true,  false, false, false, false, false, false, true,  false, false, false}};

const float OUTLINE_OFFSET = 1; // adjust this number to increase/decrease the distance of black line outlines of 3d structures in the world

const float MAP_SIZE_RATIO = .33; // size of the map compared to the entire screen
const float MAP_SIZE_RATIO_LARGE = .9; // size of the map (when large) compared to the entire screen

const float GRAVX = 0, GRAVY = 0, GRAVZ = -.1; // adjust these numbers to increase/decrease the acceleration due to gravity
const float TERMINAL_VELOCITY = 50; // objects will not accelerate to a velocity higher than this number
const float BALL_DISPLAY_SIZE = 25; // this is how large a ball is _displayed_ (ball collision is calculated as if the ball were a point)
const float BALL_ROT = PI/90; // adjust this number to increase/decrease the speed of display rotation of balls
const float PLAYER_HEIGHT = 90; // the player's height
const float PLAYER_RADIUS = 30; // the player's radius (cylindrical collision hull)
const float PLAYER_DEFAULT_HEIGHT = 50; // how high above the ground the player's center starts out
const float PLAYER_FLOAT_HEIGHT = 4; // adjust the player to never fall below this many units above the ground
const float MAX_VERTICAL_COLLISION = 12; // the maximum vertical overlap objects can have before a collision adjustment is made
const float MAX_VEL = 1; // maximum player velocity
const float MAX_ROT_VEL = 1; // maximum player rotational velocity

const float COIN_ROT = PI/90; // adjust this number to increase/decrease the speed of display rotation of coins
const float COIN_DISPLAY_SIZE = 48; // this is how large a coin is _displayed_ (coin collision is calculated as if the coin were a point)
const float COIN_DISPLAY_WIDTH = 8; // this is how thick a coin is _displayed_
const float COIN_DEFAULT_HEIGHT = 48; // this is how tall a coin is _displayed_
const float COIN_RESPAWN_TIME = 3000*4; // this is the amount of time before a coin respawns

const float COS_2PI_3 = cos(2*PI/3); // define so we don't have to keep calculating it
const float COS_4PI_3 = cos(4*PI/3); // define so we don't have to keep calculating it
const float SIN_2PI_3 = sin(2*PI/3); // define so we don't have to keep calculating it
const float SIN_4PI_3 = sin(4*PI/3); // define so we don't have to keep calculating it

const int FREEZE_TIME = 500; // how long a player is frozen still after being hit by a ball
const float ORIENTATION_CONST = .05; // *DEPRECATED* adjust this number to increase/decrease enemy rotational sensitivity
const float RAND_WALK_RAD = 256; // the distance that an enemy should randomly run to dodge a ball
const float VIEW_ANGLE = 2*PI/3; // angle from straight forward forming a cone in which enemy has line of sight
const float VIEW_DIST = 256; // distance away that enemy can see objects
const int STANDSTILL_TIMEOUT = 5; // if the enemy hasn't moved in this long, he is probably stuck, so give him something new to do

const int SLEEP_TIME_MILI = 30; // *DEPRECATED* used to make faster cpus run at the same speed as slower ones by doing work for one tick then sleeping for the remainder

const int WORLD_UP_HEIGHT = 320; // the game space height (ceiling) in actual units, not blocks

const int MAX_BALLS = 128; // maximum number of balls that can be spawned at any one time
const int MAX_COINS = 32; // maximum number of coins that can exist

#endif