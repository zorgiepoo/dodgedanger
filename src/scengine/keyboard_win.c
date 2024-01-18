
#include "keyboard.h"
#include <Windows.h>

#include <stdio.h>

bool ZGTestReturnKeyCode(uint16_t keyCode)
{
	return (keyCode == VK_RETURN);
}

bool ZGTestMetaModifier(uint64_t modifier)
{
	return (modifier == VK_CONTROL);
}

const char* ZGGetKeyCodeName(uint16_t keyCode)
{
	switch (keyCode)
	{
	case ZG_KEYCODE_RIGHT:
		return "right";
	case ZG_KEYCODE_LEFT:
		return "left";
	case ZG_KEYCODE_DOWN:
		return "down";
	case ZG_KEYCODE_UP:
		return "up";
	case ZG_KEYCODE_SPACE:
		return "space";
	}

	static char mappingBuffer[2];

	UINT mapping = MapVirtualKeyA(keyCode, MAPVK_VK_TO_CHAR);
	if (mapping != 0)
	{
		ZeroMemory(&mappingBuffer, sizeof(mappingBuffer));

		char characterValue = (char)(mapping & 0x7FFF);
		mappingBuffer[0] = characterValue;

		return mappingBuffer;
	}

	return NULL;
}

char* ZGGetClipboardText(void)
{
	if (!OpenClipboard(NULL))
	{
		fprintf(stderr, "Error: failed to open clipboard: %d\n", GetLastError());
		return NULL;
	}

	HANDLE handle = GetClipboardData(CF_TEXT);
	if (handle == NULL)
	{
		fprintf(stderr, "Error: failed to get clipboard data: %d\n", GetLastError());

		CloseClipboard();
		return NULL;
	}

	size_t bufferSize = 128;
	char *buffer = calloc(bufferSize, sizeof(*buffer));

	char *text = GlobalLock(handle);
	strncpy(buffer, text, bufferSize - 1);

	GlobalUnlock(handle);

	CloseClipboard();
	return buffer;
}

void ZGFreeClipboardText(char* clipboardText)
{
	free(clipboardText);
}
