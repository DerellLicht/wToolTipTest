//****************************************************************************
//  Copyright (c) 2008-2014  Daniel D Miller
//  common_funcs.cpp - common functions for Windows and other programs.
//  This module, which has been entirely compiled from public-domain sources, 
//  is itself declared in the public domain.
//  
//  Collected and Organized by:  Dan Miller
//****************************************************************************

#include <windows.h>
#include <tchar.h>
#include <time.h>
#include <stdio.h>   //  vsprintf
#include <math.h>    //  fabs()
#include <limits.h>
#ifdef _lint
#include <stdlib.h>
#endif

#include "common.h"

//lint -esym(526, __builtin_va_start)
//lint -esym(628, __builtin_va_start)

//lint -esym(714, TCR, TLF, TTAB)
//lint -esym(759, TCR, TLF, TTAB)
//lint -esym(765, TCR, TLF, TTAB)
const TCHAR  TCR   =  13 ;
const TCHAR  TLF   =  10 ;
const TCHAR  TTAB  =   9 ;

static char exec_fname[PATH_MAX+1] = "" ;

//****************************************************************************
//lint -esym(714, control_key_pressed)
//lint -esym(759, control_key_pressed)
//lint -esym(765, control_key_pressed)
bool control_key_pressed(void)
{
   if (GetKeyState(VK_CONTROL) & 0x8000)
      return true;
   return false;
}

//****************************************************************************
//  For some reason, on my Vista machine, I cannot access the file
//  using either FindFirstFile() or fopen().
//  It works fine on XP.
//  However, the ancient _stat() works even on Vista, so I'll use that...
//  ... until it also fails, after the third or fourth call.  duh.
//****************************************************************************
#include <sys/stat.h>

//lint -esym(714, file_exists)
//lint -esym(759, file_exists)
//lint -esym(765, file_exists)
bool file_exists(char *fefile)
{
   struct _stat st ;
   if (_stat(fefile, &st) == 0)
      return true;
   return false;
}

//lint -esym(714, drive_exists)
//lint -esym(759, drive_exists)
//lint -esym(765, drive_exists)
bool drive_exists(char *fefile)
{
   DWORD gld_return = GetLogicalDrives() ;
   char drive_letter = *fefile ;
   drive_letter |= 0x20 ;  //  convert to lower case
   uint drive_mask = 1U << (uint) (drive_letter - 'a') ; //lint !e571
   return (gld_return & drive_mask) ? true : false ;
}

//lint -esym(714, dir_exists)
//lint -esym(759, dir_exists)
//lint -esym(765, dir_exists)
bool dir_exists(char *fefile)
{
   if (strlen(fefile) == 2) {
      return drive_exists(fefile) ;
   } else {
      struct _stat st ;
      if (_stat(fefile, &st) == 0) {
         if (st.st_mode & _S_IFDIR)
            return true;
      }
   }
   return false;
}

//*****************************************************************************
//lint -esym(714, proc_time)
//lint -esym(759, proc_time)
//lint -esym(765, proc_time)
u64 proc_time(void)
{
   // return (unsigned) clock() ;
   LARGE_INTEGER ti ;
   QueryPerformanceCounter(&ti) ;
   return (u64) ti.QuadPart ;
}

//*************************************************************************
//lint -esym(714, get_clocks_per_second)
//lint -esym(759, get_clocks_per_second)
//lint -esym(765, get_clocks_per_second)
u64 get_clocks_per_second(void)
{
   static u64 clocks_per_sec64 = 0 ;
   if (clocks_per_sec64 == 0) {
      LARGE_INTEGER tfreq ;
      QueryPerformanceFrequency(&tfreq) ;
      clocks_per_sec64 = (u64) tfreq.QuadPart ;
   }
   return clocks_per_sec64 ;
}

//****************************************************************************
//lint -esym(714, swap_rgb)
//lint -esym(759, swap_rgb)
//lint -esym(765, swap_rgb)
uint swap_rgb(uint invalue)
{
   ul2uc_t uconv ;
   uconv.ul = invalue ;
   u8 utemp = uconv.uc[0] ;
   uconv.uc[0] = uconv.uc[2] ;
   uconv.uc[2] = utemp;
   return uconv.ul;
}

//*************************************************************************
//  04/23/13  NOTE
//  We *could* use GetLocalTime() here, instead of time() and localtime()
//*************************************************************************

#define  USE_SYSTIME    1
//lint -esym(714, get_dtimes_str)
//lint -esym(759, get_dtimes_str)
//lint -esym(765, get_dtimes_str)
char *get_dtimes_str(char *dest)
{
   static char ctm[GET_TIME_LEN+1] ;

   if (dest == NULL)
       dest = ctm ;

#ifdef  USE_SYSTIME
   SYSTEMTIME  stime ;
   GetLocalTime(&stime) ;

   sprintf(dest,  "%02u/%02u/%02u, %02u:%02u:%02u", 
      stime.wMonth, stime.wDay,    stime.wYear % 100,
      stime.wHour,  stime.wMinute, stime.wSecond) ;
   return dest;
#else
   time_t ttm ;
   struct tm *gtm ;
   size_t slen ;

   time(&ttm) ;
   gtm = localtime(&ttm) ;
   slen = strftime(dest, GET_TIME_LEN, "%m/%d/%y, %H:%M:%S", gtm) ;
   *(dest+slen) = 0 ;   //  strip newline from string
   return dest;
#endif   
}

//****************************************************************************
//lint -esym(714, get_file_datetime)
//lint -esym(759, get_file_datetime)
//lint -esym(765, get_file_datetime)
bool get_file_datetime(char *file_name, SYSTEMTIME *sdt, file_time_select_t time_select)
{
   WIN32_FIND_DATAA fdata ;
   HANDLE fd = FindFirstFileA(file_name, &fdata) ;
   if (fd == INVALID_HANDLE_VALUE) {
      return false;
   }
   FindClose(fd) ;

   FILETIME lft ;
   switch (time_select) {
   case FILE_DATETIME_CREATE:     FileTimeToLocalFileTime(&(fdata.ftCreationTime),   &lft);  break;
   case FILE_DATETIME_LASTACCESS: FileTimeToLocalFileTime(&(fdata.ftLastAccessTime), &lft);  break;
   case FILE_DATETIME_LASTWRITE:  FileTimeToLocalFileTime(&(fdata.ftLastWriteTime),  &lft);  break;
   default:
      return false;
   }
   FileTimeToSystemTime(&lft, sdt) ;
   return true;
}

//****************************************************************************
//  this should be called first, before other functions which use exec_fname
//****************************************************************************
//lint -esym(714, load_exec_filename)
//lint -esym(759, load_exec_filename)
//lint -esym(765, load_exec_filename)
DWORD load_exec_filename(void)
{
   //  get fully-qualified name of executable program
   DWORD result = GetModuleFileNameA(NULL, exec_fname, PATH_MAX) ;
   if (result == 0) {
      exec_fname[0] = 0 ;
      syslog("GetModuleFileName: %s\n", get_system_message()) ;
   }
   // else {
   //    syslog("exe: %s\n", exec_fname) ;
   // }
   return result ;
}

//*************************************************************
//  This appends filename to the base path previous 
//  derived by load_exec_filename()
//*************************************************************
//lint -esym(714, derive_file_path)
//lint -esym(759, derive_file_path)
//lint -esym(765, derive_file_path)
LRESULT derive_file_path(char *drvbfr, char *filename)
{
   if (exec_fname[0] == 0) {
      syslog("cannot find name of executable\n") ;
      return ERROR_FILE_NOT_FOUND ;
   }
   strncpy(drvbfr, exec_fname, PATH_MAX) ;
   //  this should never fail; failure would imply
   //  an executable with no .exe extension!
   char *sptr = strrchr(drvbfr, '\\') ;
   if (sptr == 0) {
      syslog("%s: no valid separator\n", drvbfr) ;
      return ERROR_BAD_FORMAT;
   }
   sptr++ ; //  point past the backslash
   strcpy(sptr, filename) ;
   return 0;
}

//*************************************************************
//  returns <exec_path>\\Svr2009.<new_ext>
//*************************************************************
//lint -esym(714, derive_filename_from_exec)
//lint -esym(759, derive_filename_from_exec)
//lint -esym(765, derive_filename_from_exec)
LRESULT derive_filename_from_exec(char *drvbfr, char *new_ext)
{
   if (exec_fname[0] == 0) {
      syslog("cannot find name of executable\n") ;
      return ERROR_FILE_NOT_FOUND ;
   }
   strncpy(drvbfr, exec_fname, PATH_MAX) ;
   //  this should never fail; failure would imply
   //  an executable with no .exe extension!
   char *sptr = strrchr(drvbfr, '.') ;
   if (sptr == 0) {
      syslog("%s: no valid extension\n", drvbfr) ;
      return ERROR_BAD_FORMAT;
   }
   //  if no period in new_ext, skip the one in drvbfr
   if (*new_ext != '.')
      sptr++ ;

   strcpy(sptr, new_ext) ;
   // syslog("derived [%s]\n", drvbfr) ;
   return 0;
}

//*************************************************************
//  returns <exec_path>\\Svr2009
//*************************************************************
//lint -esym(714, get_base_filename)
//lint -esym(759, get_base_filename)
//lint -esym(765, get_base_filename)
LRESULT get_base_filename(char *drvbfr)
{
   if (exec_fname[0] == 0) {
      syslog("cannot find name of executable\n") ;
      return ERROR_FILE_NOT_FOUND ;
   }
   strncpy(drvbfr, exec_fname, PATH_MAX) ;
   //  this should never fail; failure would imply
   //  an executable with no .exe extension!
   char *sptr = strrchr(drvbfr, '.') ;
   if (sptr == 0) {
      syslog("%s: no valid extension\n", drvbfr) ;
      return ERROR_BAD_FORMAT;
   }
   *sptr = 0 ; //  strip extension
   return 0;
}

//*************************************************************
//  returns <exec_path>\\            .
//*************************************************************
//lint -esym(714, get_base_path)
//lint -esym(759, get_base_path)
//lint -esym(765, get_base_path)
LRESULT get_base_path(char *drvbfr)
{
   if (exec_fname[0] == 0) {
      syslog("cannot find name of executable\n") ;
      return ERROR_FILE_NOT_FOUND ;
   }
   strncpy(drvbfr, exec_fname, PATH_MAX) ;
   //  this should never fail; failure would imply
   //  an executable with no .exe extension!
   char *sptr = strrchr(drvbfr, '\\') ;
   if (sptr == 0) {
      syslog("%s: no valid appname\n", drvbfr) ;
      return ERROR_BAD_FORMAT;
   }
   sptr++ ; //  retain backslash
   *sptr = 0 ; //  strip extension
   return 0;
}

//*************************************************************
//lint -esym(714, get_base_path_wide)
//lint -esym(759, get_base_path_wide)
//lint -esym(765, get_base_path_wide)
LRESULT get_base_path_wide(TCHAR *drvbfr)
{
   //  get fully-qualified name of executable program
   DWORD result = GetModuleFileName(NULL, drvbfr, PATH_MAX) ;
   if (result == 0) {
      *drvbfr = 0 ;
      syslog("GetModuleFileName: %s\n", get_system_message()) ;
   }
   TCHAR *sptr = _tcsrchr(drvbfr, '\\') ;
   if (sptr == 0) {
      syslog("%s: unexpected file format\n", drvbfr) ;
      return ERROR_BAD_FORMAT;
   }
   sptr++ ; //  preserve the backslash
   *sptr = 0 ; //  strip extension
   return 0;
}

//*************************************************************
//  each subsequent call to this function overwrites
//  the previous report.
//*************************************************************
//lint -esym(714, get_system_message)
//lint -esym(759, get_system_message)
//lint -esym(765, get_system_message)
char *get_system_message(DWORD errcode)
{
   static char msg[261] ;
   // int slen ;
   int result = (int) errcode ;
   if (result < 0) {
      result = -result ;
      errcode = (DWORD) result ;
      // wsprintfA(msg, "Win32: unknown error code %d", result) ;
      // return msg;
   }
   LPVOID lpMsgBuf;
   FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      errcode,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPSTR) &lpMsgBuf,
      0, 0);
   // Process any inserts in lpMsgBuf.
   // ...
   // Display the string.
   strncpy(msg, (char *) lpMsgBuf, 260) ;

   // Free the buffer.
   LocalFree( lpMsgBuf );

   //  trim the newline off the message before copying it...
   strip_newlines(msg) ;

   return msg;
}

//*************************************************************
//  each subsequent call to this function overwrites
//  the previous report.
//*************************************************************
char *get_system_message(void)
{
   return get_system_message(GetLastError());
}

//********************************************************************
//  On Windows platform, try to redefine printf/fprintf
//  so we can output code to a debug window.
//  Also, shadow syslog() within OutputDebugStringA()
//  Note: printf() remapping was unreliable,
//  but syslog worked great.
//********************************************************************
//lint -esym(714, syslog)
//lint -esym(759, syslog)
//lint -esym(765, syslog)
int syslog(const char *fmt, ...)
{
   char consoleBuffer[3000] ;
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530
   vsprintf(consoleBuffer, fmt, al);   //lint !e64
   // if (common_logging_enabled)
   //    fprintf(cmlogfd, "%s", consoleBuffer) ;
   OutputDebugStringA(consoleBuffer) ;
   va_end(al);
   return 1;
}

//********************************************************************
//  On Windows platform, try to redefine printf/fprintf
//  so we can output code to a debug window.
//  Also, shadow syslog() within OutputDebugStringA()
//  Note: printf() remapping was unreliable,
//  but syslog worked great.
//********************************************************************
//lint -esym(714, syslogW)
//lint -esym(759, syslogW)
//lint -esym(765, syslogW)
int syslogW(const TCHAR *fmt, ...)
{
   TCHAR consoleBuffer[3000] ;
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530 !e516
   _vstprintf(consoleBuffer, fmt, al);   //lint !e64
   // if (common_logging_enabled)
   //    fprintf(cmlogfd, "%s", consoleBuffer) ;
   OutputDebugString(consoleBuffer) ;
   va_end(al);
   return 1;
}

//**********************************************************************
//lint -esym(714, show_error)
//lint -esym(759, show_error)
//lint -esym(765, show_error)
char *show_error(int error_code)
{
   static char *message0 = "no response from ODU" ;
   uint ecode = (uint) (error_code < 0) ? -error_code : error_code ; //lint !e732
   if (ecode == 0)
      return message0 ;
   else
      return get_system_message(ecode) ;
}  //lint !e843

//**********************************************************************
//lint -esym(714, IsCharNum)
//lint -esym(759, IsCharNum)
//lint -esym(765, IsCharNum)
bool IsCharNum(char inchr)
{
   // if (inchr >= '0'  &&  inchr <= '9')
   if (inchr >= '0'  &&  inchr <= '9')
      return true ;
   return false;
}

//**********************************************************************
//lint -esym(714, next_field)
//lint -esym(759, next_field)
//lint -esym(765, next_field)
char *next_field(char *q)
{
   while (*q != ' '  &&  *q != HTAB  &&  *q != 0)
      q++ ; //  walk past non-spaces
   while (*q == ' '  ||  *q == HTAB)
      q++ ; //  walk past all spaces
   return q;
}

//********************************************************************
//  this function searches input string for CR/LF chars.
//  If any are found, it will replace ALL CR/LF with 0,
//  then return pointer to next non-CR/LF char.
//  If NO CR/LF are found, it returns NULL
//********************************************************************
//lint -esym(714, find_newlines)
//lint -esym(759, find_newlines)
//lint -esym(765, find_newlines)
char *find_newlines(char *hd)
{
   char *tl = hd ;
   while (1) {
      if (*tl == 0)
         return 0;
      if (*tl == CR  ||  *tl == LF) {
         while (*tl == CR  ||  *tl == LF) 
            *tl++ = 0 ;
         return tl;
      }
      tl++ ;
   }
}

//**********************************************************************
//lint -esym(714, strip_newlines)
//lint -esym(759, strip_newlines)
//lint -esym(765, strip_newlines)
void strip_newlines(char *rstr)
{
   int slen = (int) strlen(rstr) ;
   while (1) {
      if (slen == 0)
         break;
      if (*(rstr+slen-1) == '\n'  ||  *(rstr+slen-1) == '\r') {
         slen-- ;
         *(rstr+slen) = 0 ;
      } else {
         break;
      }
   }
}

//**********************************************************************
//lint -esym(714, strip_leading_spaces)
//lint -esym(759, strip_leading_spaces)
//lint -esym(765, strip_leading_spaces)
char *strip_leading_spaces(char *str)
{
   if (str == 0)
      return 0;
   char *tptr = str ;
   while (1) {
      if (*tptr == 0)
         return tptr;
      if (*tptr != ' '  &&  *tptr != HTAB)
         return tptr;
      tptr++ ;
   }
}

//**********************************************************************
//lint -esym(714, strip_trailing_spaces)
//lint -esym(759, strip_trailing_spaces)
//lint -esym(765, strip_trailing_spaces)
void strip_trailing_spaces(char *rstr)
{
   unsigned slen = strlen(rstr) ;
   while (LOOP_FOREVER) {
      if (slen == 0)
         break;
      slen-- ; //  point to last character
      if (*(rstr+slen) != ' ') 
         break;
      *(rstr+slen) = 0 ;
   }
}

//**********************************************************************
//lint -esym(714, get_hex8)
//lint -esym(759, get_hex8)
//lint -esym(765, get_hex8)
u8 get_hex8(char *ptr)
{
   char hex[3] ;
   hex[0] = *(ptr) ;
   hex[1] = *(ptr+1) ;
   hex[2] = 0 ;
   return (u8) strtoul(hex, 0, 16);
}

//**********************************************************************
//lint -esym(714, get_hex16)
//lint -esym(759, get_hex16)
//lint -esym(765, get_hex16)
u16 get_hex16(char *ptr)
{
   char hex[5] ;
   hex[0] = *(ptr) ;
   hex[1] = *(ptr+1) ;
   hex[2] = *(ptr+2) ;
   hex[3] = *(ptr+3) ;
   hex[4] = 0 ;
   return (u16) strtoul(hex, 0, 16);
}

//**********************************************************************
// :08000003ECC6030024CE00004E
//lint -esym(714, get_hex32)
//lint -esym(759, get_hex32)
//lint -esym(765, get_hex32)
u32 get_hex32(char *ptr)
{
   char hex[9] ;
   hex[0] = *(ptr) ;
   hex[1] = *(ptr+1) ;
   hex[2] = *(ptr+2) ;
   hex[3] = *(ptr+3) ;
   hex[4] = *(ptr+4) ;
   hex[5] = *(ptr+5) ;
   hex[6] = *(ptr+6) ;
   hex[7] = *(ptr+7) ;
   hex[8] = 0 ;
   return (u32) strtoul(hex, 0, 16);
}

//**********************************************************
//lint -esym(714, uabs)
//lint -esym(759, uabs)
//lint -esym(765, uabs)
uint uabs(uint uvalue1, uint uvalue2)
{
   return (uvalue1 > uvalue2)
        ? (uvalue1 - uvalue2)
        : (uvalue2 - uvalue1) ;
}

//**********************************************************
//lint -esym(714, dabs)
//lint -esym(759, dabs)
//lint -esym(765, dabs)
double dabs(double dvalue1, double dvalue2)
{
   return (dvalue1 > dvalue2)
        ? (dvalue1 - dvalue2)
        : (dvalue2 - dvalue1) ;
}

//**********************************************************************
//  Modify this to build entire string and print once.
//  This command has several forms:
//
//  - the basic form has too many arguments!!
//    bfr - data buffer to display
//    bytes - number of bytes (of bfr) to display
//    addr  - base address to display at beginning of line.
//            This helps with repeated calls to this function.
//    mode  - 0=output to printf, 1=output to syslog
//
//  - The other two forms take only buffer and length args,
//    and implicitly print to either printf or syslog.
//**********************************************************************
//lint -esym(714, hex_dump)
//lint -esym(759, hex_dump)
//lint -esym(765, hex_dump)
int hex_dump(u8 *bfr, int bytes, unsigned addr)
{
   int j, len ;
   char tail[40] ;
   char pstr[81] ;

   tail[0] = 0 ;
   int idx = 0 ;
   int plen = 0 ;
   while (1) {
      int leftovers = bytes - idx ;
      if (leftovers > 16)
          leftovers = 16 ;

      plen = wsprintfA(pstr, "%05X:  ", addr+idx) ;  //lint !e737
      len = 0 ;
      for (j=0; j<leftovers; j++) {
         u8 chr = bfr[idx+j] ;
         plen += wsprintfA(&pstr[plen], "%02X ", chr) ;
         if (chr < 32) {
            len += wsprintfA(tail+len, ".") ;
         } else if (chr < 127) {
            len += wsprintfA(tail+len, "%c", chr) ;
         } else {
            len += wsprintfA(tail+len, "?") ;
         }
      }
      //  last, print fill spaces
      for (; j<16; j++) {
         plen += wsprintfA(&pstr[plen], "   ") ;
         len += wsprintfA(tail+len, " ") ;
      }

      // printf(" | %s |\n", tail) ;
      strcat(pstr, " | ") ;
      strcat(pstr, tail) ;
      strcat(pstr, " |") ;
      // printf("%s\n", pstr) ;
      syslog("%s\n", pstr) ;

      idx += leftovers ;
      if (idx >= bytes)
         break;
   }
   return 0;
}

//**************************************************************************
int hex_dump(u8 *bfr, int bytes)
{
   return hex_dump(bfr, bytes, 0) ;
}

