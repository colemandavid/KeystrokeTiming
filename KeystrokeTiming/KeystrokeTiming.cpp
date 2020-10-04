// KeystrokeTiming.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "KeystrokeTiming.h"
#include <string>
#include <vector>



#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

/*
The intent of this application is to read the rythm as tapped out on the keyboard
by the user and calculate the note durations.  The following note durations are implemented:
Whole
Half
Quarter
Eighth
Sixteenth

and the following combinations of note durations are implemented:
Whole + Half (aka dotted whole note)
Whole + Quarter
Whole + Eighth
Whole + Sixteenth
Half + Quarter (aka dotted half note)
Half + Eighth
Half + Sixteenth
Quarter + Eighth (aka dotted quarter note)
Quarter + Sixteenth
Eighth + Sixteenth (aka dotted eighth note)

The durations are all relative to a quarter note.  The tempo in BPM is a constant (currently,
but will eventually be user entered).  

This application works by measuring the milliseconds (ms) between keypresses.  

The milliseconds per beat (based on the tempo) is calculated at startup.

Then, the durations for all of the implemented note durations are calculated
relative to a quarter note.  The multipliers are constants (i.e. a whole note 
is 4x a quarter note).  

*/
DWORD	dwTickCounts[200] = { 0 };
int		idxTicks = 0;
int		numDurations = 0;
DWORD	dwDurations[200] = { 0 };
int		Notes[200] = { 0 };		// Array holding the indices into ImplNoteList of the notes tapped out
int		iTempoBPM = 105;	// beats per minute (quarter notes)
int		iTempoMSPB;		// milliseconds per beat (quarter note)

bool	bDone = false;

class NoteStruct
{
public:
	float	fMultiplier;
	int	iDurationMS;
	std::wstring	strName;

	NoteStruct() : fMultiplier(0.0), iDurationMS(0), strName(_T("Hi")) {};
	NoteStruct(float mult, TCHAR *name) : fMultiplier(mult), iDurationMS(0), strName(name) {};
	NoteStruct(float mult, int dur, TCHAR *name) : fMultiplier(mult), iDurationMS(dur), strName(name) {};

};

void ComputeNotes(DWORD Durations[], int numDurations, std::vector<NoteStruct> &NoteList, int ComputedNotes[]);
void TestComputeNotes();



std::vector<NoteStruct>	implNoteList = { 
	NoteStruct(6.0, (TCHAR *)_T("W+H")),
	NoteStruct(5.0, (TCHAR *)_T("W+Q")),
	NoteStruct(4.5, (TCHAR *)_T("W+E")),
	NoteStruct(4.25, (TCHAR *)_T("W+S")),
	NoteStruct(4.0, (TCHAR *)_T("W")),
	NoteStruct(3.0, (TCHAR *)_T("H+Q")),
	NoteStruct(2.5, (TCHAR *)_T("H+E")),
	NoteStruct(2.25, (TCHAR *)_T("H+S")),
	NoteStruct(2.0, (TCHAR *)_T("H")),
	NoteStruct(1.5, (TCHAR *)_T("Q+E")), 
	NoteStruct(1.25, (TCHAR *)_T("Q+S")),
	NoteStruct(1.0, (TCHAR *)_T("Q")),
	NoteStruct(0.75, (TCHAR *)_T("E+S")),
	NoteStruct(0.5, (TCHAR *)_T("E")),
	NoteStruct(0.25, (TCHAR *)_T("S"))
};

/*
void BuildNoteList(int TempoMSPB);

void BuildNoteList(int TempoMSPB)
{
	NoteStruct	ns;

}
*/

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	// Calculate the number of milliseconds for a quarter note at the defined
	// tempo
	iTempoMSPB = (int)((60.0 * 1000.0) / (float)iTempoBPM);  

	for (std::vector<NoteStruct>::iterator it = implNoteList.begin(); it != implNoteList.end(); ++it)
	{
		it->iDurationMS = (int)(it->fMultiplier * iTempoMSPB);
	}


	TestComputeNotes();

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_KEYSTROKETIMING, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KEYSTROKETIMING));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEYSTROKETIMING));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_KEYSTROKETIMING);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void TestComputeNotes()
{
	DWORD	NoteDurations[200] = { 0 };
	int		FoundNotes[200]= { 0 };	
	int		count;
	int		Error = 0;

	// Perfect case
	count = 0;
	for (size_t i = 0; i < implNoteList.size(); i++)
	{
		NoteDurations[count] = implNoteList[i].iDurationMS;
		count++;
	}
	ComputeNotes(NoteDurations, count, implNoteList, FoundNotes);

	for (int i = 0; i < count; i++)
	{
		if (FoundNotes[i] != i)
		{
			// Error
			Error = 1;
		}
	}
	int x = 0;
	x++;

	// Everything longer by 10ms
	for (int i = 0; i < 200; i++)
	{
		NoteDurations[i] = 0;
		FoundNotes[i] = 0;
	}

	count = 0;
	for (size_t i = 0; i < implNoteList.size(); i++)
	{
		NoteDurations[count] = implNoteList[i].iDurationMS + 10;
		count++;
	}
	ComputeNotes(NoteDurations, count, implNoteList, FoundNotes);
	for (int i = 0; i < count; i++)
	{
		if (FoundNotes[i] != i)
		{
			// Error
			Error = 1;
		}
	}

	// Everything longer by almost a 1/16 (shortest value - 1)
	// This took some thought to figure out what the right answers were supposed to be
	// Because it's adding essentially a 1/16, it shifts some down
	// (i.e. W+S + (S-1) is closest to W+E, W + (S-1) is closest to W+S (skipping W), etc.)
	for (int i = 0; i < 200; i++)
	{
		NoteDurations[i] = 0;
		FoundNotes[i] = 0;
	}
	int	ExpectedNotes[200] = { 0, 1, 2, 2, 3, 5, 6, 6, 7, 9, 9, 10 , 11, 12, 13, 0 };
	count = 0;
	for (size_t i = 0; i < implNoteList.size(); i++)
	{
		int	shortestDuration = implNoteList[implNoteList.size() - 1].iDurationMS - 1;
		NoteDurations[count] = implNoteList[i].iDurationMS + shortestDuration;
		count++;
	}
	ComputeNotes(NoteDurations, count, implNoteList, FoundNotes);
	for (int i = 0; i < count; i++)
	{
		if (FoundNotes[i] != ExpectedNotes[i])
		{
			// Error
			Error = 1;
		}
	}

	// Corner Case: no durations passed in
	for (int i = 0; i < 200; i++)
	{
		NoteDurations[i] = 0;
		FoundNotes[i] = 0;
	}
	count = 0;
	ComputeNotes(NoteDurations, count, implNoteList, FoundNotes);

	if (FoundNotes[0] != 0)
	{
		// Error
		Error = 1;
	}

	// Durations list 2x as long as implNoteList
	for (int i = 0; i < 200; i++)
	{
		NoteDurations[i] = 0;
		FoundNotes[i] = 0;
	}

	count = 0;
	for (size_t i = 0; i < implNoteList.size(); i++)
	{
		NoteDurations[count] = implNoteList[i].iDurationMS;
		ExpectedNotes[count] = i;
		count++;
		NoteDurations[count] = implNoteList[i].iDurationMS;
		ExpectedNotes[count] = i;
		count++;
	}

	ComputeNotes(NoteDurations, count, implNoteList, FoundNotes);
	for (int i = 0; i < count; i = i+2)
	{
		if (FoundNotes[i] != ExpectedNotes[i])
		{
			// Error
			Error = 1;
		}
	}


}
void ComputeNotes(DWORD Durations[], int numDurations, std::vector<NoteStruct> &NoteList, int ComputedNotes[])
{
	int	idxCurrDuration;

	for (idxCurrDuration = 0; idxCurrDuration < numDurations; idxCurrDuration++)
	{
		DWORD dwMinDelta = 99999999;
		int idxMinIndex = -1;
		DWORD	dwCurrDuration = Durations[idxCurrDuration];

		for (size_t i = 0; i < NoteList.size(); i++)
		{
			DWORD	dwCurrDelta;
			if (NoteList[i].iDurationMS > dwCurrDuration)
			{ 
				dwCurrDelta = (DWORD)NoteList[i].iDurationMS - dwCurrDuration;
			}
			else
			{
				dwCurrDelta = dwCurrDuration - NoteList[i].iDurationMS;
			}

			if (dwCurrDelta < dwMinDelta)
			{
				dwMinDelta = dwCurrDelta;
				idxMinIndex = i;
			}
		}
		ComputedNotes[idxCurrDuration] = idxMinIndex;
	}


} // ComputeNotes

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
			TEXTMETRIC	tm;

            HDC hdc = BeginPaint(hWnd, &ps);
			GetTextMetrics(hdc, &tm);
			int	cy = tm.tmHeight;
			int cx = tm.tmAveCharWidth;
//			cx = tm.tmMaxCharWidth;

			int x = 0;
			int y = 0;

            // TODO: Add any drawing code that uses hdc here...
			TCHAR buffer[250];
			HRESULT hr;
			hr = StringCchPrintf(buffer, 249, _T("Tempo (BPM) %d "), iTempoBPM);
			TextOut(hdc, x, y, buffer, lstrlen(buffer));
			SIZE	stringSize;
			GetTextExtentPoint(hdc, buffer, lstrlen(buffer), &stringSize);
			x = 0;
			y = y + stringSize.cy;
			hr = StringCchPrintf(buffer, 249, _T("Tempo(MSPB) %d "), iTempoMSPB);
			TextOut(hdc, x, y, buffer, lstrlen(buffer));
			GetTextExtentPoint(hdc, buffer, lstrlen(buffer), &stringSize);
			x = 0;
			y = y + stringSize.cy;

/*
			hr = StringCchPrintf(buffer, 249, _T("Durations(MS) %f %f %f %f %f"), 
				noteDurations[0],
				noteDurations[1],
				noteDurations[2],
				noteDurations[3],
				noteDurations[4]);
			TextOut(hdc, x, y, buffer, lstrlen(buffer));
			y = y + cy;
*/
			TCHAR	*label = (TCHAR *)_T("Durations(MS) ");

			TextOut(hdc, x, y, label, lstrlen(label));
			GetTextExtentPoint(hdc, label, lstrlen(label), &stringSize);
			x = stringSize.cx;


			for (size_t i = 0; i < implNoteList.size(); i++)
			{
				int len = implNoteList[i].strName.length();
				TextOut(hdc, x, y, implNoteList[i].strName.c_str(), len);
				GetTextExtentPoint(hdc, implNoteList[i].strName.c_str(), len, &stringSize);
				x = x + stringSize.cx;

				hr = StringCchPrintf(buffer, 249, _T("=%d "), implNoteList[i].iDurationMS);
				TextOut(hdc, x, y, buffer, lstrlen(buffer));
				GetTextExtentPoint(hdc, buffer, lstrlen(buffer), &stringSize);
				x = x + stringSize.cx;
			} 

			x = 0;
			y = y + stringSize.cy;

			if (bDone)
			{
				numDurations = idxTicks - 1;
				for (int i = 0; i < numDurations; i++)
				{
//					hr = StringCchPrintf(buffer, 249, _T("%d "), dwDurations[i]);
					TCHAR *pstr = (TCHAR *)implNoteList[Notes[i]].strName.c_str();
					int len = implNoteList[Notes[i]].strName.length();
					TextOut(hdc, x, y, pstr, len);
					GetTextExtentPoint(hdc, pstr, len, &stringSize);
					x = x + stringSize.cx + (cx*2);
				}
			}
            EndPaint(hWnd, &ps);
        }
        break;

	case WM_KEYDOWN:
	{
		if (wParam == VK_TAB)
		{
			numDurations = idxTicks - 1;
			for (int i = 0; i < numDurations; i++)
			{
				dwDurations[i] = dwTickCounts[i + 1] - dwTickCounts[i];
			}
			ComputeNotes(dwDurations, numDurations, implNoteList, Notes);
			bDone = true;
			InvalidateRect(hWnd, NULL, TRUE);

		}
		else
		{
			if (idxTicks < 200)
			{
				dwTickCounts[idxTicks++] = GetTickCount();
			}

		}
	}
		break;
	case WM_CHAR:
		break;

/*
		{
		if (wParam == _T('\t'))
		{
			numDurations = idxTicks - 1;
			for (int i = 0; i < (idxTicks-1); i++)
			{
				dwDurations[i] = dwTickCounts[i + 1] - dwTickCounts[i];
			}
			ComputeNotes(dwDurations, numDurations, implNoteList, Notes);
			bDone = true;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else
		{
			if (idxTicks < 200)
			{
				dwTickCounts[idxTicks++] = GetTickCount();
			}
		}
		}
*/
		break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
