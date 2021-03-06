#include "game.h"

#define DEBUG 1

#if 1 //def CONFIG_IDF_TARGET_ESP32
#include "img/logo.h"
#include "../oled/oled.h"
#define RANDOM() esp_random()
#else
#include "logo.h"
#define RANDOM() random()
#define BIT(_n) (1 << (_n))
#endif

#define GET_BIT(_r, _b) ((_r)>>(_b) & 1)
#define SET_BIT(_r, _b) ((_r) |= BIT(_b))
#define CLR_BIT(_r, _b) ((_r) &= ~BIT(_b))


#define CLEAR() printf("\33[2J")
#define MOVE_UP(x) printf("\33[%dA", (x))
#define MOVE_DOWN(x) printf("\033[%dB", (x))
#define MOVE_LEFT(y) printf("\033[%dD", (y))
#define MOVE_RIGHT(y) printf("\033[%dC",(y))
#define MOVE_TO(x,y) printf("\033[%d;%dH", (x), (y))
#define RESET_CURSOR() printf("\033[H")
#define HIDE_CURSOR() printf("\033[?25l") 
#define SHOW_CURSOR() printf("\033[?25h") 

/* plane
Hero
00100
11111
00100
01110
unsigned char hero_img[] = {0x27, 0xC8, 0xE0};

Plane
01010
11111
1110
00100
00100 
unsigned char plane[] = {0x57, 0xDC, 0x42, 0x00};

Boss
0110110
1111111
0111110
0011100
0001000
unsigned char plane[] = {0x6D, 0xFD, 0xF1, 0xC1, 0x00};
*/

struct block {
    unsigned char w;
    unsigned char h;
    unsigned char img[];
}plane = {5, 5, {0x21, 0x1D, 0xF5, 0x00}}, hero = {5, 4, {0x71, 0x3E, 0x40}}, bullet = {1, 1, {0x80}}, boss={7, 5, {0x10, 0x71, 0xF7, 0xF5, 0xC0}};
//}plane = {8, 4, {0x66, 0xFF, 0x3C, 0x18}}, hero = {8, 4, {0x18, 0xFF, 0x18, 0x3C}}, bullet = {1, 1, {0x80}} ;
//}plane = {5, 5, {0x57, 0xDC, 0x42, 0x00}}, hero = {5, 4, {0x27, 0xC8, 0xE0}}, bullet = {1, 1, {0x80}}, boss={7, 5, {0x6D, 0xFD, 0xF1, 0xC1, 0x00}};

static unsigned char *game_map = NULL;
static G_BLOCK *bl = NULL;
static unsigned int hero_x = 0;
static unsigned int hero_y = 0;

#if 0 //ndef CONFIG_IDF_TARGET_ESP32
static void OLED_fill_surface(unsigned char *p) {
	for (int w = 0; w < MAP_WIDTH; w++) {
		for (int h = 0; h < MAP_HEIGHT; h++) {
			printf("%d", GET_BIT(game_map[(w / 8) * 128 + h], w % 8));
		}
		printf("\n");
	}
	MOVE_UP(MAP_WIDTH);
	fflush(stdout);
}
#endif

static void add_block(int x, int y, B_TYPE type) {
	G_BLOCK *t = bl;
	while(NULL != t) {
		if (NULL == t->next) {
			G_BLOCK *p = (G_BLOCK*) malloc (sizeof(G_BLOCK));
			if (NULL != p) {
				p->x = x;
				p->y = y;
				p->a.s.type = type;
				p->next = NULL;
				t->next = p;
			}
			break;
		}
		t = t->next;
	}
}

static void remove_block(G_BLOCK *b) {
	G_BLOCK *t = bl;
	while(NULL != t && NULL != t->next) {
		if (t->next == b) {
			t->next = t->next->next;
			free(b);
			b = NULL;
		}
		t = t->next;
	}
}

static void free_blocks() {
	G_BLOCK *t = bl;
	/* free block list. */
	while( t ) {
		G_BLOCK *t1 = t;
		t = t->next;
		free(t1);
		t1 = NULL;
	}
	bl = NULL;
	/* free map */
	if (NULL != game_map) {
		free(game_map);
		game_map = NULL;
	}
}

static void GameInit() {
	game_map = (unsigned char*) malloc(MAP_SIZE * sizeof (unsigned char));
	if (NULL == game_map) {
		return;
	}
	/* malloc hear.*/
	bl = (G_BLOCK*) malloc (sizeof(G_BLOCK));
	if (NULL == bl) {
		return;
	}
	bl->x = 0;
	bl->y = 64;
	bl->a.s.type = TYPE_HERO;
	bl->next = NULL;
	/* malloc plane.*/
	for (int i = 0; i < 5; i++) {
		add_block(RANDOM() & 0x1F, 127, TYPE_PLANE);
	}
}

static void update_blocks() {
	G_BLOCK *t = bl;
	while( NULL != t ) {
		switch (t->a.s.type)
		{
			case TYPE_BULLET:
				t->y += 3;
				if (MAP_HEIGHT < t->y) 
					t->y = 0;
				break;
			case TYPE_BOSS:
			case TYPE_PLANE:
				t->y -= 3;
				if (MAP_HEIGHT < t->y) 
					t->y = 127;
				break;
			case TYPE_HERO:
				t->x = hero_x;
				t->y = hero_y;
				/* Hero conrtol by user.*/
				break;
			default:
				LOG("--- Type Undefined... ---");
				break;
		}
		t = t->next;
	}
}

void update_hero(C_ACTION a) {
	const int step = 2;
	switch(a) {
		case C_UP:
			if (hero_y < MAP_HEIGHT - step) 
				hero_y += step;
			break;
		case C_DOWN:
			if (hero_y > step) 
				hero_y -= step;
			break;
		case C_LEFT:
			if (hero_x > step) 
				hero_x -= step;
			break;
		case C_RIGHT:
			if (hero_x < MAP_WIDTH ) 
				hero_x += step;
			break;
		default:
			break;
	}
}

static void draw_block(unsigned int x, unsigned int y, struct block *b) {
	for(int y1 = 0; y1< b->h; y1++) {
		for (int x1 = 0; x1< b->w; x1++) {
			if ( (y1 + y) < MAP_HEIGHT && (x1 + x) < MAP_WIDTH) {
				GET_BIT(b->img[(y1 * b->w + x1) / 8], (7 - (y1 * b->w + x1) % 8))?  
					SET_BIT(game_map[((x+x1)/8)*128 + y + y1], (x + x1) % 8) : 
					CLR_BIT(game_map[((x+x1)/8)*128 + y + y1], (x + x1) % 8) ;
			}
		}
	}
}

static void update_surface() {
	G_BLOCK *t = bl;
	memset(game_map, 0x00, MAP_SIZE);
	while (NULL != t) {
		switch (t->a.s.type)
		{
			case TYPE_HERO:
				draw_block(t->x, t->y, &hero);
				break;
			case TYPE_PLANE:
				draw_block(t->x, t->y, &plane);
				break;
			case TYPE_BULLET:
				draw_block(t->x, t->y, &bullet);
				break;
			case TYPE_BOSS:
				//draw_block(t->x, t->y, &boss);
				break;
			default:
				LOG("--- Type Undefined... ---");
				break;
		}
		t = t->next;
	}
}

static void GameLoop(void *arg) {
    while (true) {
		/* TODO: get control. */
		/* TODO: check action. */
		/* TODO: fill surface. */
		update_blocks();
		update_surface();
        OLED_fill_surface(game_map);
		// vTaskDelay(500 / portTICK_PERIOD_MS);
    }
	free_blocks();
    vTaskDelete(NULL);
	LOG("---Thread Done---");
}

void GameStart() {
    LOG("---Game Start---");
	GameInit();
	LOG("-- Update surface ---");
	memcpy(game_map, logo, MAP_SIZE);
	LOG("---FILL SURFACE---");
	OLED_fill_surface(game_map);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
    xTaskCreate( GameLoop, "Game Loop", 1024 * 2,  NULL, 10, NULL);
	LOG("--- Game start done---");
	/* Why need delay.? */
	sleep(1);
	return;
}
