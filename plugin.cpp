#include "plugin.h"
#include "parser.h"
#include <string>
#include <iostream> 
#include <windows.h>
#include <vector>
unsigned char* snapshotContent;
unsigned char* snapshotContent2;
long long addrmainmodule;
long long sizemainmodule;


enum
{
    MENU_DISASM_PASTE,
	MENU_DISASM_PASTE_PATCH,
	MENU_DISASM_CLEAR,
    MENU_DUMP_PASTE,
	MENU_DUMP_PASTE_PATCH,
	MENU_DUMP_CLEAR,
	MENU_DISASM_ABOUT,
	MENU_DUMP_ABOUT
};


#define ABOUT_MSG "CreateBackup by Priboi\r\n\r\n" \
				  "Make Snapshot and then compare to see differences!\r\n\r\n" \
				  "compiled in: "  __DATE__ " " __TIME__


void About()
{
	MessageBoxA(hwndDlg, ABOUT_MSG, "CreateBackup", MB_ICONINFORMATION);
}
	
LPSTR GetClipboardTextData(size_t *pLength)
{
	LPSTR temp=NULL,pastedContent = NULL;
	size_t contentLength = 0;
	HANDLE cbHandle = NULL;

	if (pLength)
		*pLength = 0;

	if (!OpenClipboard(hwndDlg))
	{
		MessageBoxA(hwndDlg, "The clipboard couldn't be opened","yummyPaste",MB_ICONWARNING);
		goto oneWayExit;
	}


	if (!IsClipboardFormatAvailable(CF_TEXT))
	{
		goto oneWayExit;
	}

	cbHandle = GetClipboardData(CF_TEXT);

	if (!cbHandle)
	{
		MessageBoxA(hwndDlg, "Clipboard data couldn't readed", "yummyPaste", MB_ICONWARNING);
		goto oneWayExit;
	}



	temp = (LPSTR)GlobalLock(cbHandle);

	if (!temp)
	{
		MessageBoxA(hwndDlg, "The data couldn't be extracted from the cb object", "yummyPaste", MB_ICONSTOP);
		goto oneWayExit;
	}

	contentLength = strlen(temp);

	if (contentLength == 0)
		goto oneWayExit;

	pastedContent = (LPSTR)Malloc(contentLength + 1);


	if (!pastedContent)
	{
		goto oneWayExit;
	}

	
	strcpy_s(pastedContent, contentLength+1, temp);

	
	if (pLength)
		*pLength = contentLength;

oneWayExit:

	if (temp)
	{
		GlobalUnlock(cbHandle);
		temp = NULL;
	}

	CloseClipboard();

	return pastedContent;

	
}
		   


void MakeTomatoPaste(int window,BOOL makeSnapshot)
{		
	addrmainmodule = DbgFunctions()->ModBaseFromAddr(DbgValFromString("mod.main()"));
	sizemainmodule = DbgFunctions()->ModSizeFromAddr(addrmainmodule);

	if (makeSnapshot)
	{
		if (!snapshotContent) snapshotContent = new unsigned char[sizemainmodule];

		DbgMemRead(addrmainmodule, snapshotContent, sizemainmodule);
	}
	else
	{
		if (!snapshotContent2) snapshotContent2 = new unsigned char[sizemainmodule];
		DbgMemRead(addrmainmodule, snapshotContent2, sizemainmodule);
		for(int x = 0; x < sizemainmodule; x++)
			if (snapshotContent[x] != snapshotContent2[x])
			{
				DbgMemWrite(addrmainmodule + x, snapshotContent + x, 1);	 // this function dont change byte color and restore bytes from first snapshot I used this because below function MemPatch only change color when bytes are change. Thats trick.
				DbgFunctions()->MemPatch(addrmainmodule + x, snapshotContent2 + x, 1); // this function change byte color and restore actual bytes. Thats looks like we have actual data/code and changed bytes color where is difference between snapshots
			}
	
	}
	//GuiUpdatePatches(); // sprawdzic
	if (window == GUI_DISASSEMBLY);
	//	GuiUpdateDisassemblyView();
	else if (window == GUI_DUMP);
	//	GuiUpdateDumpView();

}
void Clear()
{
	for (int x = 0; x < sizemainmodule; x++)
		if (snapshotContent[x] != snapshotContent2[x])
		{
			DbgFunctions()->MemPatch(addrmainmodule + x, snapshotContent + x, 1);
			DbgMemWrite(addrmainmodule + x, snapshotContent2 + x, 1); // this function change byte color and restore actual bytes. Thats looks like we have actual data/code and changed bytes color where is difference between snapshots
		}

}

PLUG_EXPORT void CBINITDEBUG(CBTYPE cbType, PLUG_CB_INITDEBUG* info)
{
}

PLUG_EXPORT void CBSTOPDEBUG(CBTYPE cbType, PLUG_CB_STOPDEBUG* info)
{
}

PLUG_EXPORT void CBEXCEPTION(CBTYPE cbType, PLUG_CB_EXCEPTION* info)
{
}

PLUG_EXPORT void CBDEBUGEVENT(CBTYPE cbType, PLUG_CB_DEBUGEVENT* info)
{
}

PLUG_EXPORT void CBMENUENTRY(CBTYPE cbType, PLUG_CB_MENUENTRY* info)
{
    switch(info->hEntry)
    {

    case MENU_DISASM_PASTE:
        MakeTomatoPaste(GUI_DISASSEMBLY, FALSE);
        break;
	case MENU_DISASM_PASTE_PATCH:
		MakeTomatoPaste(GUI_DISASSEMBLY, TRUE);
		break;
	case MENU_DISASM_CLEAR:
		Clear();
		break;
    case MENU_DUMP_PASTE:
        MakeTomatoPaste(GUI_DUMP,FALSE);
        break;
	case MENU_DUMP_PASTE_PATCH:
		MakeTomatoPaste(GUI_DUMP, TRUE);
		break;
	case MENU_DUMP_CLEAR:
		Clear();
		break;
	case MENU_DISASM_ABOUT:
	case MENU_DUMP_ABOUT:
		About();
		break;
    default:
        break;
    }
}

//Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
	if (!InitBinaryObject(0xFEED))
	{
		MessageBoxA(hwndDlg, "Ups. memory?", "yummyPaste", MB_ICONSTOP);
		return false;
	}


    return true; //Return false to cancel loading the plugin.
}


void pluginStop()
{
	DestroyBinaryObject();
}

void pluginSetup()
{
    _plugin_menuaddentry(hMenuDisasm, MENU_DISASM_PASTE, "&Compare Snapshot");
	_plugin_menuaddentry(hMenuDisasm, MENU_DISASM_PASTE_PATCH, "Make Snapshot");
	_plugin_menuaddentry(hMenuDisasm, MENU_DUMP_CLEAR, "Clear Snapshot");
    _plugin_menuaddentry(hMenuDump, MENU_DISASM_CLEAR, "Clear Snapshot");
    _plugin_menuaddentry(hMenuDump, MENU_DUMP_PASTE, "&Compare Snapshot");
	_plugin_menuaddentry(hMenuDump, MENU_DUMP_PASTE_PATCH, "Make Snapshot");
	_plugin_menuaddentry(hMenuDisasm, MENU_DISASM_ABOUT, "A&bout");
	_plugin_menuaddentry(hMenuDump, MENU_DUMP_ABOUT, "A&bout");

}
