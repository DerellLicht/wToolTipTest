//****************************************************************************
//  Copyright (c) 2008-2014  Daniel D Miller
//  tooltips.cpp - tooltip functions/data
//
//  Written by:  Dan Miller 
//****************************************************************************
//  Usage:
//    HWND hToolTip = create_tooltips(hwnd, 150, 100, 10000) ;
//    add_program_tooltips(hwnd, hToolTip) ;
//****************************************************************************

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0501
#include <windows.h>
#include <tchar.h>
#include <commctrl.h>

#include "resource.h"
#include "common.h"

//  static tooltip-list struct
typedef struct tooltip_data_s {
   uint ControlID ;
   TCHAR *msg ;
} tooltip_data_t ;

//****************************************************************************
HWND create_tooltips(HWND hwnd, uint max_width, uint popup_msec, uint stayup_msec)
{
   HWND hToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, TTS_ALWAYSTIP,
         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, //lint !e569
         hwnd, NULL, GetModuleHandle(NULL), NULL);
   if (hToolTip == NULL) {
      syslog("ToolTip CreateWindowEx: %s\n", get_system_message()) ;
   } else {
      SendMessage(hToolTip, TTM_SETMAXTIPWIDTH, 0, max_width) ;
      SendMessage(hToolTip, TTM_SETDELAYTIME, TTDT_INITIAL, popup_msec) ;
      SendMessage(hToolTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, stayup_msec) ;
   }
   return hToolTip ;
}

//****************************************************************************
static void add_tooltip_target(HWND parent, HWND target, HWND hToolTip, TCHAR *msg)
{
   // static bool hex_dump_was_run = false ;
   TOOLINFO ti;
   ti.cbSize = sizeof(TOOLINFO);
   ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   ti.hwnd = parent;
   ti.uId = (UINT) target;
   ti.lpszText = msg ;
   LRESULT bresult = SendMessage(hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
   if (bresult == 0) {
      syslog("TTM_ADDTOOL: %u: %s\n", target, get_system_message()) ;
   }
}  //lint !e550  ti

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

void add_program_tooltips(HWND hwnd, HWND hwndToolTip)
{
   unsigned idx ;
   for (idx=0; program_tooltips[idx].ControlID != 0; idx++) {
      add_tooltip_target(hwnd, GetDlgItem(hwnd, program_tooltips[idx].ControlID),
         hwndToolTip, program_tooltips[idx].msg) ;
   }
}

