#include "game.h"

#include "img/logo.h"
#include "../oled/oled.h"

#define DEBUG 1

//#define BIT(_n) (1 << (_n))
#define GET_BIT(_r, _b) ((_r) & (_b))
#define SET_BIT(_r, _b) ((_r) |= (_b))
#define CLR_BIT(_r, _b) ((_r) &= ~(_b))

/* plane
Hero
00100
11111   01110001
00100   00111110
01110   0100   
unsigned char hero_img[] = {0x71, 0x3E, 0x40};

Plane
01010
11111
01110
00100  
00100 = 00100001 00011101 11110101 0
unsigned char plane[] = {0x21, 0x1D, 0xF5, 0x00};

Boss
0110110
1111111
0111110
0011100
0001000  = 00010000 01110001 11110111 11110110 110
unsigned char plane[] = {0x10, 0x71, 0xF7, 0xF5, 0xC0};
*/

struct block {
    unsigned char w;
    unsigned char h;
    unsigned char img[];
}plane = {5, 5, {0x21, 0x1D, 0xF5, 0x00}}, hero = {5, 4, {0x71, 0x3E, 0x40}}, bullet = {1, 1, {0x80}}, boss={7, 5, {0x10, 0x71, 0xF7, 0xF5, 0xC0}};

static unsigned char game_map[ MAP_SIZE ] = {0x00};
static G_BLOCK *bl = NULL;

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
	while( t ) {
		G_BLOCK *t1 = t;
		t = t->next;
		free(t1);
		t1 = NULL;
	}
	bl = NULL;
}

static void GameInit() {
	/* malloc hero.*/
	bl = (G_BLOCK*) malloc (sizeof(G_BLOCK));
	if (NULL == bl) {
		return;
	}
#ifdef DEBUG
	bl->x = esp_random() & 0x1F;
	bl->y = esp_random() & 0x7F;
#else
	bl->x = 0;
	bl->y = 0;
#endif
	bl->a.s.type = TYPE_HERO;
	bl->next = NULL;
	/* malloc plane.*/
	for (int i = 0; i < 5; i++) {
		add_block(esp_random() & 0x1F, 127, TYPE_PLANE);
	}
	for (int i = 0; i < 1; i++) {
		add_block(esp_random() & 0x1F, 127, TYPE_BOSS);
	}
	for (int i = 0; i < 10; i++) {
		add_block(esp_random() & 0x1F, 0, TYPE_BULLET);
	}
}

static void update_blocks() {
	G_BLOCK *t = bl;
	while( NULL != t ) {
		switch (t->a.s.type)
		{
			case TYPE_BULLET:
				if ( 127 < (t->y + 3)) {
					t->y = 0;
				} else {
					t->y += 3;
				}
				break;
			case TYPE_BOSS:
			case TYPE_PLANE:
				if (0 >= (t->y - 3)) {
					t->y = 127;
				}else{
					t->y -= esp_random() & 0x03;
				}
				break;
			case TYPE_HERO:
				/* Hero conrtol by user.*/
				break;
			default:
				LOG("--- Type Undefined... ---");
				break;
		}
		t = t->next;
	}
}

static void draw_block(unsigned int x, unsigned int y, struct block *b) {
	for(int y1 = 0; y1< b->h; y1++) {
		for (int x1 = 0; x1< b->w; x1++) {
			if ((((x+x1)/8)*128 + y + y1) < 512) {
				GET_BIT(b->img[(y1 * b->w + x1) / 8], BIT(7 - (y1 * b->w + x1) % 8))?  
					SET_BIT(game_map[((x+x1)/8)*128 + y + y1], BIT((x + x1)% 8)) : 
					CLR_BIT(game_map[((x+x1)/8)*128 + y + y1], BIT((x + x1)% 8)) ;
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
				draw_block(t->x, t->y, &boss);
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
