
#import "gamepad.h"
#import "platforms.h"
#import <stdbool.h>

#if PLATFORM_IOS
#define GC_KEYBOARD 1
#else
#define GC_KEYBOARD 0
#endif

typedef struct _Gamepad
{
	void *controller;
#if GC_KEYBOARD
	void *keyboard;
#endif
	GamepadState lastStates[GAMEPAD_BUTTON_MAX];
	char name[GAMEPAD_NAME_SIZE];
	GamepadIndex index;
	uint8_t rank;
} Gamepad;

struct _GamepadManager
{
	Gamepad gamepads[MAX_GAMEPADS];
	GamepadEvent eventsBuffer[GAMEPAD_EVENT_BUFFER_CAPACITY];
	GamepadCallback addedCallback;
	GamepadCallback removalCallback;
	void *context;
	GamepadIndex nextGamepadIndex;
};

struct _GamepadManager *initGamepadManager(const char *databasePath, GamepadCallback addedCallback, GamepadCallback removalCallback, void *context);

GamepadEvent *pollGamepadEvents(struct _GamepadManager *gamepadManager, const void *systemEvent, uint16_t *eventCount);

const char *gamepadName(struct _GamepadManager *gamepadManager, GamepadIndex index);

uint8_t gamepadRank(struct _GamepadManager *gamepadManager, GamepadIndex index);

void setPlayerIndex(struct _GamepadManager *gamepadManager, GamepadIndex index, int64_t playerIndex);
