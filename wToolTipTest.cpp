//**********************************************************************
//  Copyright (c) 2018-2023  Daniel D Miller
//  
//  Written by:   Daniel D. Miller
//**********************************************************************
//  version    changes
//  =======    ======================================
//    1.00     original, derived from wFontList
//    1.02     remove application-specific data from tooltips module
//****************************************************************************

static char const * const Version = "wToolTipDemo, Version 1.02" ;

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0501
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <time.h>
#include <tchar.h>
#include <commctrl.h>

#include "resource.h"
#include "common.h"
#include "tooltips.h"

//lint -esym(715, hwnd, private_data, message, wParam, lParam)
//***********************************************************************

//lint -esym(844, hwndBitGapSpin, hwndCharGapSpin, hwndPixDiamSpin)
static HWND hwndFontName    = NULL ;
static HWND hwndPixDiam     = NULL ;
static HWND hwndBitGap      = NULL ;
static HWND hwndCharGap     = NULL ;
static HWND hwndPixDiamSpin = NULL ;
static HWND hwndBitGapSpin  = NULL ;
static HWND hwndCharGapSpin = NULL ;
static HWND hwndSetAttr     = NULL ;
static HWND hwndClearAttr   = NULL ;
static HWND hwndBgndAttr    = NULL ;

static char font_name[1024] = "script2.f19" ;

static uint const MAX_CHAR_GAP = 6 ;

#define  SQUARE_PIXELS  1

//  maximum pixel dimensions
static uint const MAX_DIAMETER = 5 ;
static uint const MAX_BIT_GAP = 3 ;

typedef struct lrender_init_s {
   char   *font_name ;        //  Name of font file to use for display
   uint   diameter ;          //  diameter of pixels in font
   uint   bit_gap ;           //  gap between pixels in character
   uint   char_gap ;          //  gap between characters
   uint   pixel_type ;        //  square or round pixels
   COLORREF bgnd ;            //  background color
   COLORREF set_bit ;         //  color of set bits
   COLORREF clear_bit ;       //  color of cleared bits
} lrender_init_t;

static lrender_init_t test_init = {
   font_name, 3, 1, 2, SQUARE_PIXELS, RGB(31, 31, 31), RGB(63, 181, 255), RGB(23, 64, 103)
} ;

//****************************************************************************
//  CommPort dialog tooltips
//****************************************************************************
static tooltip_data_t const program_tooltips[] = {
{ IDS_FONTNAME,      _T("Filename of current font")},
{ IDC_FONTNAME,      _T("Filename of current font")},
{ IDB_LOAD_FONT,     _T("Load different font file" )},
{ IDC_RB_ROUND,      _T("Change displayed font bits to filled circles" )},
{ IDC_RB_SQUARE,     _T("Change displayed font bits to filled squares" )},
{ IDS_PIXDIAM,       _T("Set diameter of displayed font bits" )},
{ IDC_PIXDIAM,       _T("Set diameter of displayed font bits" )},
{ IDS_BITGAP,        _T("Set gap between font bits (in pixels)" )},
{ IDC_BITGAP,        _T("Set gap between font bits (in pixels)" )},
{ IDS_CHARGAP,       _T("Set gap between characters (in pixels)" )},
{ IDC_CHARGAP,       _T("Set gap between characters (in pixels)" )},
{ IDB_ATTR_SET,      _T("Set color of SET (ON) font bits" )},
{ IDC_SHOW_SET,      _T("Set color of SET (ON) font bits" )},
{ IDB_ATTR_CLEAR,    _T("Set color of CLEARED (OFF) font bits" )},
{ IDC_SHOW_CLEAR,    _T("Set color of CLEARED (OFF) font bits" )},
{ IDB_ATTR_BGND,     _T("Set background color of display field" )},
{ IDC_SHOW_BGND,     _T("Set background color of display field" )},
{ IDOK,              _T("Close this program" )},

//  This is how to enter multi-line tooltips:
// { IDS_CP_SERNUM,     _T("The SEND CMD button will send COMMAND to the device with")
//                      _T("this Serial Number.  If Serial Number is 0, COMMAND is sent ")
//                      _T("to the broadcast address on the current port.") },

{ 0, NULL }} ;

/****************************************************************************
 * This hook procedure, which allows ClearIcon to position the color dialog
 * as desired, was extracted from Nancy Cluts, Chapter 06, cmndlg32.
 * The technique of setting the dialog position by trapping WM_INITDIALOG
 * in the hook procedure, was taken from comments on the web.
****************************************************************************/
#define  USE_CHOOSECOLOR_HOOK    1

#ifdef  USE_CHOOSECOLOR_HOOK
static BOOL APIENTRY ChooseColorHookProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   switch (message) {
   case WM_INITDIALOG:
      {
      DWORD UFLAGS = SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW;
      SetWindowPos(hDlg, HWND_TOP, 400, 300, 0, 0, UFLAGS);
      }   
      break;
        
   }  //lint !e744 switch statement has no default
   return (FALSE);
}
#endif

//****************************************************************
static COLORREF select_color(HWND hwnd, COLORREF old_attr)
{
   static CHOOSECOLOR cc ;
   static COLORREF    crCustColors[16] ;

   ZeroMemory(&cc, sizeof(cc));
   cc.lStructSize    = sizeof (CHOOSECOLOR) ;
   cc.rgbResult      = old_attr ;
   cc.lpCustColors   = crCustColors ;
#ifdef  USE_CHOOSECOLOR_HOOK
   cc.Flags          = CC_RGBINIT | CC_FULLOPEN | CC_ENABLEHOOK;
   cc.lpfnHook = (LPCCHOOKPROC) ChooseColorHookProc;
   // cc.lpTemplateName = (LPTSTR) NULL;  //  not needed, struct was 0'd out
#else
   cc.Flags          = CC_RGBINIT | CC_FULLOPEN ;
#endif
   cc.hwndOwner      = hwnd ;

   if (ChooseColor(&cc) == TRUE) {
      return cc.rgbResult ;
   } else {
      return old_attr ;
   }
}  //lint !e715

//*********************************************************************
//  clear entire background of current field
//*********************************************************************
static void fill_color_field(HWND hwnd, COLORREF attr)
{
   RECT m_rect ;
   GetClientRect(hwnd, &m_rect);

   HDC hdc = GetDC(hwnd) ;
   HBRUSH hbBkBrush = CreateSolidBrush(attr);
   FillRect(hdc, &m_rect, hbBkBrush);
   DeleteObject(hbBkBrush);
   ReleaseDC(hwnd, hdc) ;
}

//*******************************************************************
static bool do_init_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   // RECT myRect ;
   char msgstr[81] ;
   wsprintfA(msgstr, "%s", Version) ;
   SetWindowTextA(hwnd, msgstr) ;

   //  Add program icons to title bar
   SetClassLongA(hwnd, GCL_HICON,   (LONG) LoadIcon(GetModuleHandle(NULL), (LPCSTR) LCDTICO));
   SetClassLongA(hwnd, GCL_HICONSM, (LONG) LoadIcon(GetModuleHandle(NULL), (LPCSTR) LCDTICO));

   //*****************************************
   //  initialize controls
   //*****************************************
   CheckRadioButton (hwnd, IDC_RB_ROUND, IDC_RB_SQUARE, IDC_RB_ROUND + test_init.pixel_type) ;

   hwndFontName = GetDlgItem(hwnd, IDC_FONTNAME) ;
   wsprintf(msgstr, "%s", test_init.font_name) ;
   SetWindowText(hwndFontName, msgstr) ;

   //  manage pixel-diameter field
   hwndPixDiam = GetDlgItem(hwnd, IDC_PIXDIAM) ;
   hwndPixDiamSpin = CreateUpDownControl(
      WS_CHILD|WS_VISIBLE|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|WS_BORDER,
      // spin button control style
      // UDS_SETBUDDYINT   Causes the control to set the text of the buddy window
      // UDS_ALIGNRIGHT or UDS_ALIGNLEFT  must be set or else updown will not place beside the edit
      0, 0, 0, 0,          // size and position
      hwnd,                // parent handle
      IDC_PIXDIAMSPIN,     // updown ID
      GetModuleHandle(NULL),             // instance handle
      hwndPixDiam,         // associated field, or NULL
      MAX_DIAMETER,        // max value
      1,                   // min value
      test_init.diameter); // initial value

   //  manage bit-gap field
   hwndBitGap  = GetDlgItem(hwnd, IDC_BITGAP) ;
   hwndBitGapSpin = CreateUpDownControl(
      WS_CHILD|WS_VISIBLE|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|WS_BORDER,
      // spin button control style
      // UDS_SETBUDDYINT   Causes the control to set the text of the buddy window
      // UDS_ALIGNRIGHT or UDS_ALIGNLEFT  must be set or else updown will not place beside the edit
      0, 0, 0, 0,          // size and position
      hwnd,                // parent handle
      IDC_BITGAPSPIN,      // updown ID
      GetModuleHandle(NULL),             // instance handle
      hwndBitGap,          // associated field, or NULL
      MAX_BIT_GAP,         // max value
      0,                   // min value
      test_init.bit_gap);  // initial value

   //  manage char-gap field
   hwndCharGap = GetDlgItem(hwnd, IDC_CHARGAP) ;
   hwndCharGapSpin = CreateUpDownControl(
      WS_CHILD|WS_VISIBLE|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|WS_BORDER,
      // spin button control style
      // UDS_SETBUDDYINT   Causes the control to set the text of the buddy window
      // UDS_ALIGNRIGHT or UDS_ALIGNLEFT  must be set or else updown will not place beside the edit
      0, 0, 0, 0,          // size and position
      hwnd,                // parent handle
      IDC_CHARGAPSPIN,     // updown ID
      GetModuleHandle(NULL),             // instance handle
      hwndCharGap,         // associated field, or NULL
      MAX_CHAR_GAP,        // max value
      0,                   // min value
      test_init.char_gap); // initial value

   //  fill in color fields
   hwndSetAttr   = GetDlgItem(hwnd, IDC_SHOW_SET) ;
   hwndClearAttr = GetDlgItem(hwnd, IDC_SHOW_CLEAR) ;
   hwndBgndAttr  = GetDlgItem(hwnd, IDC_SHOW_BGND) ;

   //  set up the tooltips
   HWND hToolTip = create_tooltips(hwnd, 150, 100, 10000) ;
   add_tooltips(hwnd, hToolTip, program_tooltips);
   return true ;
}

//*******************************************************************
//  handle inputs to up/down controls
//*******************************************************************
static bool do_vscroll(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
// #define SB_THUMBPOSITION   4
// #define SB_ENDSCROLL       8
// [3496] scroll code=4, nPos=2
// [3496] scroll code=8, nPos=2
//    syslog("scroll code=%u, nPos=%u\n", LOWORD(wParam), HIWORD(wParam)) ;
   if (LOWORD(wParam) != SB_ENDSCROLL)
      return false;
   if (hwndPixDiamSpin == (HWND) lParam) {
      return true;
   } 
   if (hwndBitGapSpin == (HWND) lParam) {
      return true;
   } 
   if (hwndCharGapSpin == (HWND) lParam) {
      return true;
   } 
   return false ;
}

//*******************************************************************
static bool do_command(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   // TCHAR msgstr[81] ;
   int result ;

   DWORD cmd = HIWORD (wParam) ;
   DWORD target = LOWORD(wParam) ;
   // termout(&this_term, "WM_COMMAND: cmd=%u, target=%u", cmd, target) ;
   if (cmd == BN_CLICKED) {
      switch (target) {
      case IDC_RB_SQUARE:
      case IDC_RB_ROUND:
         test_init.pixel_type = target - IDC_RB_ROUND ;
         CheckRadioButton (hwnd, IDC_RB_ROUND, IDC_RB_SQUARE, IDC_RB_ROUND + test_init.pixel_type) ;
         return true;

      case IDB_LOAD_FONT:
         return true;

      case IDB_ATTR_SET:
         result = select_color(hwnd, test_init.set_bit) ;
         if (result >= 0) {
            test_init.set_bit = (COLORREF) result ;
            fill_color_field(hwndSetAttr,   test_init.set_bit) ;
         }
         return true;

      case IDB_ATTR_CLEAR:
         result = select_color(hwnd, test_init.clear_bit) ;
         if (result >= 0) {
            test_init.clear_bit = (COLORREF) result ;
            fill_color_field(hwndClearAttr, test_init.clear_bit) ;
         }
         return true;

      case IDB_ATTR_BGND:
         result = select_color(hwnd, test_init.bgnd) ;
         if (result >= 0) {
            test_init.bgnd = (COLORREF) result ;
            fill_color_field(hwndBgndAttr,  test_init.bgnd) ;
         }
         return true;
         
      case IDOK:
         PostMessage(hwnd, WM_CLOSE, 0, 0);
         return true;

      default:
         break;
      }  //lint !e744
   } 
   return false ;
}

//*******************************************************************
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef  USE_WINMSGS
   switch (message) {
   //  list messages to be ignored
   case WM_MOUSEMOVE:
   case WM_NCMOUSEMOVE:
   case WM_NCHITTEST:
   case WM_SETCURSOR:
   case WM_TIMER:
   case WM_NOTIFY:
   case WM_COMMAND:  //  prints its own msgs below
      break;
   default:
      syslog("TOP [%u]\n", message) ;
      break;
   }
#endif
   switch (message) {
   case WM_INITDIALOG:
      do_init_dialog(hwnd, message, wParam, lParam, NULL);
      break;

   case WM_COMMAND:
      do_command(hwnd, message, wParam, lParam, NULL);
      break;

   case WM_VSCROLL:
      return do_vscroll(hwnd, message, wParam, lParam, NULL);

   case WM_CLOSE:
      DestroyWindow(hwnd);
      return true ;

   case WM_DESTROY:
      PostQuitMessage(0);  //  this closes parent as well
      return true ;

   default:
      return FALSE;
   }

   return false;
}  //lint !e715

//***********************************************************************
//lint -esym(1784, WinMain)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
   {
   load_exec_filename() ;     //  get our executable name

   HWND hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_SF_DIALOG), NULL, (DLGPROC) WndProc) ;
   if (hwnd == NULL) {
      syslog("CreateDialog: %s\n", get_system_message()) ;
      return 0;
   }

   MSG Msg;
   while(GetMessage(&Msg, NULL,0,0)) {
      if(!IsDialogMessage(hwnd, &Msg)) {
          TranslateMessage(&Msg);
          DispatchMessage(&Msg);
      }
   }

   return (int) Msg.wParam ;
}  //lint !e715

