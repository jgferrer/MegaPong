#include <genesis.h>
#include <resources.h>
#include <string.h>
#include <font.h>

#define ELEMENTCOUNT(x)  (sizeof(x) / sizeof(x[0]))

typedef struct
{
    u16 x;
    u16 y;
    char label[10];
} Option;

#define NUM_MAIN_MENU_OPTIONS 2
Option mainmenu_options[NUM_MAIN_MENU_OPTIONS] = {
    {8, 8, "START"},
    {8, 9, "OPTIONS"}
};

#define NUM_MENU_OPTIONS 3
Option menu_options[NUM_MENU_OPTIONS] = {
    {10, 9, "Easy"},
    {10, 10, "Medium"},
    {10, 11, "Hard"}
};

enum GAME_STATE {
    STATE_MENU,
    STATE_PLAY,
    STATE_OPTIONS
};
enum GAME_STATE currentState;

u8 currentIndex = 0;
u8 currentOptionsIndex = 0;
u8 difficulty = 0; // 0-easy, 1-medium, 2-hard
Sprite* cursor;

Sprite* ball;
int ball_pos_x = 100;
int ball_pos_y = 100;
int ball_vel_x = 1;
int ball_vel_y = 1;
int ball_width = 8;
int ball_height = 8;

Sprite* player;
int player_pos_x = 144;
const int player_pos_y = 200;
int player_vel_x = 0;
const int player_width = 32;
const int player_height = 8;

/*Score variables*/
int score = 0;
char label_score[6] = "SCORE\0";
char str_score[4] = "0";

char msg_reset[37] = "GAME OVER!";

/*The edges of the play field*/
const int LEFT_EDGE = 0;
const int RIGHT_EDGE = 320;
const int TOP_EDGE = 0;
const int BOTTOM_EDGE = 224;

// Func Prototypes
int sign(int x);
void showText(char s[]);
void endGame();
void updateScoreDisplay();
void basicInit();
void drawMenuOptions();
void updateCursorPosition();
void updateCursorOptionsPosition();
void moveUp();
void moveDown();
void joyEventHandler(u16 joy, u16 changed, u16 state);
void processStateMenu();
void startGame();
void processStatePlay();
void moveBall();
void positionPlayer();
void processStateOptions();

int main()
{    
    basicInit();
    while(1)
    {
        switch(currentState){
            case STATE_MENU:{
                processStateMenu();
                break;
            }
            case STATE_PLAY:{
                processStatePlay();
                break;
            }
            case STATE_OPTIONS:{
                processStateOptions();
            }
        }
    }
    return (0);
}

int sign(int x) {
    return (x > 0) - (x < 0);
}

/*Draws text in the center of the screen*/
void showText(char s[]){
	VDP_drawText(s, 20 - strlen(s)/2 ,15);
}

void endGame(){
	showText(msg_reset);
    u16 timer = 120;
    while (timer > 0)
    {
        timer--;
        if(timer == 0){
            currentState = STATE_MENU;
            basicInit();
        }
        SYS_doVBlankProcess();
    }    
}

void updateScoreDisplay(){
	sprintf(str_score,"%d",score);
	VDP_clearText(1,2,3);
	VDP_drawText(str_score,1,2);
}

void basicInit(){
    JOY_init();
    SPR_init();
    currentState = STATE_MENU;
    //Cleanup
    VDP_clearPlane(BG_A, TRUE);
    JOY_setEventHandler(&joyEventHandler);
    PAL_setColor(0, RGB24_TO_VDPCOLOR(0x000000));
    cursor = SPR_addSprite(&gfx_cursor, 0, 0, 0);
    //Draw options
    u16 i = 0;
    for (; i < ELEMENTCOUNT(mainmenu_options); i++) {
        Option o = mainmenu_options[i];
        VDP_drawText(o.label,o.x,o.y);
    }
}

void drawMenuOptions(){
    JOY_init();
    SPR_init();
    currentState = STATE_OPTIONS;
    //Cleanup
    VDP_clearPlane(BG_A, TRUE);
    JOY_setEventHandler(&joyEventHandler);
    PAL_setColor(0, RGB24_TO_VDPCOLOR(0x000000));
    cursor = SPR_addSprite(&gfx_cursor, 0, 0, 0);
    //Draw options
    u16 i = 0;
    for (; i < ELEMENTCOUNT(menu_options); i++) {
        Option o = menu_options[i];
        VDP_drawText(o.label,o.x,o.y);
    }
}

void updateCursorPosition(){
    SPR_setPosition(cursor, mainmenu_options[currentIndex].x*8-12, mainmenu_options[currentIndex].y*8);
}

void updateCursorOptionsPosition(){
    currentOptionsIndex = difficulty;
    SPR_setPosition(cursor, menu_options[currentOptionsIndex].x*8-12, menu_options[currentOptionsIndex].y*8);
}

void moveUp(){
    if(currentIndex > 0 && currentState == STATE_MENU){
        currentIndex--;
        updateCursorPosition();
    } else if (currentOptionsIndex > 0 && currentState == STATE_OPTIONS) {
        currentOptionsIndex--;
        difficulty--;
        updateCursorOptionsPosition();
    }
}

void moveDown(){
    if(currentIndex < ELEMENTCOUNT(mainmenu_options)-1 && currentState == STATE_MENU){
        currentIndex++;
        updateCursorPosition();
    } else if (currentOptionsIndex < ELEMENTCOUNT(menu_options)-1 && currentState == STATE_OPTIONS) {
        currentOptionsIndex++;
        difficulty++;
        updateCursorOptionsPosition();
    }
}

void joyEventHandler(u16 joy, u16 changed, u16 state){
    if (currentState != STATE_PLAY){
        if (changed & state & BUTTON_UP)
        {
            moveUp();
        }
        else if(changed & state & BUTTON_DOWN){
            moveDown();
        }
        if(changed & state & BUTTON_A && (currentState == STATE_MENU)){
            if(currentIndex == 0){
                currentState = STATE_PLAY;
            }else if (currentIndex == 1){
                currentState = STATE_OPTIONS;
            }
        }
        else if(changed & state & BUTTON_A && (currentState == STATE_OPTIONS))
        {
            currentState = STATE_MENU;
        }
    }
    else {
        if (joy == JOY_1)
	    {   
            /*Set player velocity if left or right are pressed;
            *set velocity to 0 if no direction is pressed */
            if (state & BUTTON_RIGHT)
            {
                player_vel_x = 3;
            }
            else if (state & BUTTON_LEFT)
            {
                player_vel_x = -3;
            } else{
                if( (changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT) ){
                    player_vel_x = 0;
                }
            }
        }
    }
}

void processStateMenu(){
    //Update
    while(currentState == STATE_MENU){
        updateCursorPosition();
        SPR_update();
        SYS_doVBlankProcess();
    }
}

void startGame(){
	score = 0;
	updateScoreDisplay();

	ball_pos_x = 10;
	ball_pos_y = 10;
	ball_vel_x = 1;
	ball_vel_y = 1;
    player_pos_x = 144;
    
	/*Clear the text from the screen*/
	VDP_clearTextArea(0,10,40,10);
}

void processStatePlay(){
    VDP_clearPlane(BG_A, TRUE);
    SPR_releaseSprite(cursor);
    SPR_clear();
    startGame();
    VDP_loadTileSet(bgtile.tileset,1,DMA);
    PAL_setPalette(1, bgtile.palette->data,DMA);
    VDP_fillTileMapRect(BG_A,TILE_ATTR_FULL(PAL1,0,FALSE,FALSE,1),0,0,40,30);
    ball = SPR_addSprite(&imgball,100,100,TILE_ATTR(PAL1,0, FALSE, FALSE));
    player = SPR_addSprite(&paddle, player_pos_x, player_pos_y, TILE_ATTR(PAL1, 0, FALSE, FALSE));

    /*Draw the texts*/
    VDP_setTextPlane(BG_A);
    VDP_drawText(label_score,1,1);
    updateScoreDisplay();

    while (currentState == STATE_PLAY)
    {
        moveBall();
        positionPlayer();
        
        SPR_update();
        SYS_doVBlankProcess();
    }
}

void moveBall(){
    //Check horizontal bounds
    if(ball_pos_x < LEFT_EDGE){
        ball_pos_x = LEFT_EDGE;
        ball_vel_x = -ball_vel_x;
    } else if(ball_pos_x + ball_width > RIGHT_EDGE){
        ball_pos_x = RIGHT_EDGE - ball_width;
        ball_vel_x = -ball_vel_x;
    }
    //Check vertical bounds
    if(ball_pos_y < TOP_EDGE){
        ball_pos_y = TOP_EDGE;
        ball_vel_y = -ball_vel_y;
    } else if(ball_pos_y + ball_height > BOTTOM_EDGE){
        //ball_pos_y = BOTTOM_EDGE - ball_height;
        //ball_vel_y = -ball_vel_y;
        endGame();
    }

    /*Check for collisions with the player paddle*/
    if(ball_pos_x < player_pos_x + player_width && ball_pos_x + ball_width > player_pos_x){
        if(ball_pos_y < player_pos_y + player_height && ball_pos_y + ball_height >= player_pos_y){
            //On collision, invert the velocity
            ball_pos_y = player_pos_y - ball_height - 1;
            ball_vel_y = -ball_vel_y;
            //Increase the score and update the HUD
            score++;
            updateScoreDisplay();
            //Make ball faster on every 10th hit
            if( score % 10 == 0){
                ball_vel_x += sign(ball_vel_x);
                ball_vel_y += sign(ball_vel_y);
            }
        }
    }

    //Ball movement goes here
    ball_pos_x += ball_vel_x;
    ball_pos_y += ball_vel_y;
    SPR_setPosition(ball,ball_pos_x,ball_pos_y);
}

void positionPlayer(){
	/*Add the player's velocity to its position*/
	player_pos_x += player_vel_x;

	/*Keep the player within the bounds of the screen*/
	if(player_pos_x < LEFT_EDGE) player_pos_x = LEFT_EDGE;
	if(player_pos_x + player_width > RIGHT_EDGE) player_pos_x = RIGHT_EDGE - player_width;

	/*Let the Sprite engine position the sprite*/
	SPR_setPosition(player,player_pos_x,player_pos_y);
}

void processStateOptions(){
    drawMenuOptions();
    VDP_drawText("DIFFICULTY", 8, 8);
    VDP_drawText("Select difficulty and press 'A'", 1, 22);
    //Update
    while(currentState == STATE_OPTIONS){
        updateCursorOptionsPosition();
        SPR_update();
        SYS_doVBlankProcess();
    }
    basicInit();
}
