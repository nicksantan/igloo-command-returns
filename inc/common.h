#ifndef COMMON_H
#define COMMON_H

#include <genesis.h>

// Screen constants
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   224

// Crosshair speeds
#define CROSSHAIR_SPEED_NORMAL  2
#define CROSSHAIR_SPEED_BOOST   4

// Missile constants
#define MAX_MISSILES 30
#define MISSILE_SPEED FIX16(3)
#define MISSILE_GRAVITY FIX16(0.02)
#define MISSILE_TYPE_NORMAL 0
#define MISSILE_TYPE_FAST 1

// Enemy constants
#define MAX_ENEMIES 7
#define MAX_LARGE_ENEMIES 5
#define ENEMY_SPEED FIX16(0.3)
#define LARGE_ENEMY_HURT_DURATION 10  // Frames to show hurt sprite (1/6 second at 60fps)
#define ENEMY_MIN_SPACING 40  // Minimum pixels between enemies when spawning

// Bomb constants
#define MAX_BOMBS 20
#define BOMB_INITIAL_VY FIX16(0.05)
#define BOMB_GRAVITY FIX16(0.02)
#define BOMB_MAX_VY FIX16(0.8)
#define BOMB_BLAST_RADIUS 32
#define BOMB_BLAST_FORCE FIX16(3.0)
#define BOMB_CHAIN_RADIUS 5
#define BOMB_DROP_CHANCE 1

// Igloo constants
#define NUM_IGLOOS 5

// Cannon positions
#define CANNON_LEFT_X  32
#define CANNON_RIGHT_X (SCREEN_WIDTH - 32)
#define CANNON_Y       (SCREEN_HEIGHT - 48)

// Powerup truck constants
#define TRUCK_SPEED FIX16(1.125)  // Increased by 50% again (was 0.75, now 1.125)
#define TRUCK_Y (SCREEN_HEIGHT - 27)  // Moved down 5px from -32
#define TRUCK_ARROW_VY FIX16(-1.25)  // Upward velocity when arrow is released (half speed)
#define TRUCK_ARROW_DISTANCE 30  // Distance in pixels the arrow travels upward
#define TRUCK_ARROW_HOLD_TIME 30  // Frames to hold before disappearing (0.5 seconds at 60fps)

// Polar bear constants
#define POLAR_BEAR_SPEED FIX16(0.6)  // Normal speed
#define POLAR_BEAR_SPEED_BOOST FIX16(0.6)  // Speed boost per click (same as base speed)
#define POLAR_BEAR_MAX_CLICKS 3  // Max clicks that give speed boost
#define POLAR_BEAR_Y (SCREEN_HEIGHT - 32)  // Y position
#define POLAR_BEAR_BASE_SCORE 200  // First click score

// Explosion constants
#define MAX_EXPLOSIONS 10
#define EXPLOSION_DURATION 15  // Frames to show explosion (0.25 seconds at 60fps)

// Powerup constants
#define TRIPLE_SHOT_DURATION 1800  // 30 seconds at 60fps
#define TRIPLE_SHOT_ANGLE_COS 63329  // cos(15°) ≈ 0.9659 in fix16
#define TRIPLE_SHOT_ANGLE_SIN 16965  // sin(15°) ≈ 0.2588 in fix16
#define FAST_SHOT_DURATION 1800  // 30 seconds at 60fps

// Missile structure
typedef struct {
    fix16 x, y;
    fix16 target_x, target_y;
    fix16 vx, vy;
    u8 active;
    u8 player;
    u8 type;  // MISSILE_TYPE_NORMAL or MISSILE_TYPE_FAST
    Sprite* sprite;
} Missile;

// Enemy structure
typedef struct {
    fix16 x, y;
    fix16 vx;
    u8 active;
    u8 from_left;
    s8 hp;
    u8 hurt_timer;  // Frames to show hurt sprite (for large enemies)
    Sprite* sprite;
} Enemy;

// Bomb structure
typedef struct {
    fix16 x, y;
    fix16 vx, vy;
    u8 active;
    Sprite* sprite;
} Bomb;

// Igloo structure
typedef struct {
    s16 x, y;
    u8 alive;
    Sprite* sprite;
} Igloo;

// Powerup truck structure
typedef struct {
    fix16 x, y;
    fix16 vx;
    u8 active;
    u8 from_left;
    u8 spawn_pending;  // TRUE if truck should spawn after delay
    u16 spawn_timer;   // Frames until spawn (60 fps)
    Sprite* sprite;
    Sprite* arrow_sprite;  // Arrow powerup indicator
    u8 arrow_collected;    // TRUE if arrow has been clicked/collected
    fix16 arrow_x;         // X position of arrow (when collected, stays fixed)
    fix16 arrow_y;         // Y position of arrow (when collected and moving upward)
    fix16 arrow_start_y;   // Starting Y position when arrow was collected
    fix16 arrow_vy;        // Y velocity of arrow (when collected)
    u8 arrow_hold_timer;   // Timer for holding at top before disappearing
} PowerupTruck;

// Polar bear structure
typedef struct {
    fix16 x, y;
    fix16 vx;
    u8 active;
    u8 from_left;
    u8 spawn_pending;
    u16 spawn_timer;
    u8 click_count;  // Number of times clicked this appearance
    Sprite* sprite;
} PolarBear;

// Explosion structure
typedef struct {
    s16 x, y;
    u8 active;
    u8 timer;  // Frames remaining before removal
    Sprite* sprite;
} Explosion;

// Global game state (extern declarations)
extern u8 two_player_mode;
extern u16 current_wave;
extern u8 enemies_spawned;
extern u8 large_enemies_spawned;
extern u8 wave_complete;
extern u8 game_over;
extern u8 game_paused;
extern u32 score_p1;
extern u32 score_p2;
extern u16 ammo_p1;
extern u16 ammo_p2;
extern u16 megabombs;
extern u8 bonus_igloos_queued;
extern u32 next_bonus_threshold;
extern u8 triple_shot_active_p1;
extern u16 triple_shot_timer_p1;
extern u8 triple_shot_active_p2;
extern u16 triple_shot_timer_p2;
extern u8 fast_shot_active_p1;
extern u16 fast_shot_timer_p1;
extern u8 fast_shot_active_p2;
extern u16 fast_shot_timer_p2;

// Global object pools (extern declarations)
extern Missile missiles[MAX_MISSILES];
extern Enemy enemies[MAX_ENEMIES];
extern Enemy large_enemies[MAX_LARGE_ENEMIES];
extern Bomb bombs[MAX_BOMBS];
extern Igloo igloos[NUM_IGLOOS];
extern PowerupTruck powerup_truck;
extern PolarBear polar_bear;
extern Explosion explosions[MAX_EXPLOSIONS];

#endif // COMMON_H
