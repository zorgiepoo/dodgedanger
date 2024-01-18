
#include "defaults.h"
#include <Shlwapi.h>
#include <ShlObj.h>
#include <windows.h>

FILE *getUserDataFile(const char* defaultsName, const char *mode)
{
	FILE *file = NULL;
	
	TCHAR appDataPath[MAX_PATH] = { 0 };
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath)))
	{
		PathAppend(appDataPath, defaultsName);
		
		int success = CreateDirectory(appDataPath, NULL);
		if (success == 0 || success == ERROR_ALREADY_EXISTS)
		{
			const char userDataFilename[] = "user_data.txt";

			PathAppend(appDataPath, userDataFilename);
			file = fopen(appDataPath, mode);
		}
	}
	
	return file;
}
