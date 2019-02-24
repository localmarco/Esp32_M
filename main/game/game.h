#ifndef __GAME_H
#define __GAME_H
#include <stdio.h>
#include <stdlib.h>

#define LOG(fmt, ...) ESP_LOGI("[GAME]", fmt, ##__VA_ARGS__)

#define MAP_WIDTH 32
#define MAP_HEIGHT 128
#define MAP_SIZE (MAP_WIDTH * MAP_HEIGHT / 8)
#define FRAME_FREQ 30
#define HERO_NUM 1
#define PLANE_NUM 10

typedef enum _block_type {
	TYPE_HERO = 0,
	TYPE_PLANE,
	TYPE_BULLET,
	TYPE_BOSS
}B_TYPE;

typedef enum _action {
	C_UP = 0,
	C_DOWN,
	C_LEFT,
	C_RIGHT
}C_ACTION;

typedef union game_action {
	unsigned char c;
	struct s_action {
		unsigned char type:4; /* 0: hero, 1: plane, 2: bullet */
		unsigned char speed:2;
		unsigned char action0:1;
		unsigned char action1:1;
		unsigned char action2:1;
		unsigned char action3:1;
	} s;
} G_ACTION;

typedef struct _base_block {
    unsigned int x;
    unsigned int y;
	G_ACTION a;
	struct _base_block *next;
} G_BLOCK; 

void GameStart();
void update_hero(C_ACTION a);

#endif
