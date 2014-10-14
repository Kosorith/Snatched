 /***********************************************
 *          EMBEDDED SYSTEMS
 * 
 * Project: Pacman
 * Device: LPC2148v3 + Expansion Board
 *
 * Group B09 (tuesday, 12:15)
 * Members:
 *    - Piotr Grzelak
 *    - Andrzej Kurczewski
 *    - Michał Sośnicki
 *    - Wojciech Szałapski (team leader)
 *
 ***********************************************/

// main.c
// Reponsible for initialization and creating application processes.

/************/
/* Includes */
/************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"

#include "lcd.h"
#include "key.h"
#include "display.h"
#include "game.h"

/***********/
/* Defines */
/***********/

// stack size for processes:
#define INIT_STACK_SIZE  	400
#define GAME_STACK_SIZE    1200

 // LCD contrast value
 #define LCD_CONTRAST		 50

/*************/
/* Variables */
/*************/

// stacks for processes
static tU8 initStack[INIT_STACK_SIZE];
static tU8 gameStack[GAME_STACK_SIZE];

/*************/
/* Functions */
/*************/

static void initializationProcess(void* arg);
static void gameProcess(void *arg);

// The first function to be executed.
// Creates the initialization process and then starts the OS.
// Initialization process is executed with the highest priority to make sure no other processes start before it ends.
//
// Parameters:
//   [in] arg - parameters passed to the function (not used)
int main(void) {

  tU8 initProcError;
  tU8 initProcPid;

  // osInit() is mandatory before any other call to OS
  osInit();
  osCreateProcess(initializationProcess, initStack, INIT_STACK_SIZE, &initProcPid, 1, NULL, &initProcError);
  osStartProcess(initProcPid, &initProcError);
  
  // starts the operating system
  // before calling this function at least one process must be created and started
  osStart();
  return 0;
}

// Initialization process is responsible for creating application processes.
//
// Parameters:
//   [in] arg - parameters passed to the function (not used)
static void initializationProcess(void* arg) {

	tU8 gameProcError;
	tU8 gameProcPid;
	osCreateProcess(gameProcess, gameStack, GAME_STACK_SIZE, &gameProcPid, 2, NULL, &gameProcError);
  	osStartProcess(gameProcPid, &gameProcError);

	// releases the process control block (PCB)
	osDeleteProcess();
}

// Process reponsible for starting the game.
//
// Parameters:
//   [in] arg - parameters passed to the function (not used)
static void gameProcess(void* arg) {

	// initializes LCD
	lcdInit();
	lcdContrast(LCD_CONTRAST);
	// initializes joystick
	initKeyProc();
	
	displayMenu();

	while (1) {
		tU8 keyPressed = checkKey();

		if (keyPressed == KEY_CENTER) {
			startGame();
			displayMenu();
		}

		osSleep(20);
	}

	// releases the process control block (PCB)
	osDeleteProcess();
}

 // This function is called once for each timer tick interrupt in OS.
 // It should be kept as short as possible because it runs in interrupt context.
 //
 // Parameters:
 //   [in] elapsedTime - Number of elapsed milliseconds since the last call of this function.
void appTick(tU32 elapsedTime) {
}