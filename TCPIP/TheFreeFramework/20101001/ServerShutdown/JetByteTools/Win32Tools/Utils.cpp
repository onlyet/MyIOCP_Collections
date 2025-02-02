///////////////////////////////////////////////////////////////////////////////
// File: Utils.cpp
///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997 - 2003 JetByte Limited.
//
// JetByte Limited grants you ("Licensee") a non-exclusive, royalty free, 
// licence to use, modify and redistribute this software in source and binary 
// code form, provided that i) this copyright notice and licence appear on all 
// copies of the software; and ii) Licensee does not utilize the software in a 
// manner which is disparaging to JetByte Limited.
//
// This software is provided "as is" without a warranty of any kind. All 
// express or implied conditions, representations and warranties, including
// any implied warranty of merchantability, fitness for a particular purpose
// or non-infringement, are hereby excluded. JetByte Limited and its licensors 
// shall not be liable for any damages suffered by licensee as a result of 
// using, modifying or distributing the software or its derivatives. In no
// event will JetByte Limited be liable for any lost revenue, profit or data,
// or for direct, indirect, special, consequential, incidental or punitive
// damages, however caused and regardless of the theory of liability, arising 
// out of the use of or inability to use software, even if JetByte Limited 
// has been advised of the possibility of such damages.
//
// This software is not designed or intended for use in on-line control of 
// aircraft, air traffic, aircraft navigation or aircraft communications; or in 
// the design, construction, operation or maintenance of any nuclear 
// facility. Licensee represents and warrants that it will not use or 
// redistribute the Software for such purposes. 
//
///////////////////////////////////////////////////////////////////////////////

#include "Utils.h"
#include "Exception.h"
#include "CriticalSection.h"

#include <memory>
#include <iostream>
#include <fstream>

#include <atlbase.h>       // USES_CONVERSION

#ifdef _UNICODE
typedef std::wfstream _tfstream;
#else
typedef std::fstream _tfstream;
#endif

#include "Lmcons.h"     // UNLEN

#pragma warning(disable : 4996)     // 'function': was declared deprecated

///////////////////////////////////////////////////////////////////////////////
// Lint options
//
//lint -save
//
//lint -esym(534, swprintf, wprintf)   ignoring return value
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Using directives
///////////////////////////////////////////////////////////////////////////////

using std::auto_ptr;
using std::endl;

///////////////////////////////////////////////////////////////////////////////
// Namespace: JetByteTools::Win32
///////////////////////////////////////////////////////////////////////////////

namespace JetByteTools {
namespace Win32 {

CCriticalSection s_criticalSection;

static _tfstream s_debugOut;

static std::string s_logFileName = "\\JetByteTools.log";

void SetLogFileName(const _tstring &name)
{
   USES_CONVERSION;

   if (s_debugOut.is_open())
   {
      s_debugOut.close();
   }

   s_logFileName = T2A(const_cast<PTSTR>(name.c_str()));
}

void Output(const _tstring &message)
{
#ifdef _DEBUG
   CCriticalSection::Owner lock(s_criticalSection);

#ifdef _UNICODE
   std::wcout << ToString(GetCurrentThreadId()) << _T(": ") << message << endl;
#else
   std::cout << ToString(GetCurrentThreadId()) << _T(": ") << message << endl;
#endif

   const _tstring msg = ToString(GetCurrentThreadId()) + _T(": ") + message + _T("\n");

   OutputDebugString(msg.c_str());

   if (!s_debugOut.is_open())
   {
      s_debugOut.open(s_logFileName.c_str(), std::ios_base::out | std::ios_base::app);

      s_debugOut << _T("****************New Log*****************") << endl;
   }

   s_debugOut <<  ToString(GetCurrentThreadId()) << _T(": ") << message << endl;
#else
   //lint -e{715} symbol 'message' not referenced
#endif //_DEBUG
}

_tstring GetLastErrorMessage(DWORD last_error)
{
   static TCHAR errmsg[512];

   if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 
      0,
      last_error,
      0,
      errmsg, 
      511,
      NULL))
   {
      /* if we fail, call ourself to find out why and return that error */
      return (GetLastErrorMessage(GetLastError()));  
   }
  
   return errmsg;
}


_tstring HexToString(
   const BYTE *pBuffer, 
   size_t iBytes)
{
   _tstring result;
     
   for (size_t i = 0; i < iBytes; i++)
   {
      BYTE c ;

      BYTE b = pBuffer[i] >> 4;
         
      if (9 >= b)
      {
         c = b + '0';
      }
      else
      {
         c = (b - 10) + 'A';
      }

      result += (TCHAR)c;

      b = pBuffer[i] & 0x0f;

      if (9 >= b)
      {
         c = b + '0';
      }
      else
      {
         c = (b - 10) + 'A';
      }

      result += (TCHAR)c;
   }

   return result;
}

void StringToHex(
   const _tstring &ts, 
   BYTE *pBuffer, 
   size_t nBytes)
{
   USES_CONVERSION;

   const std::string s = T2A(const_cast<PTSTR>(ts.c_str()));

   for (size_t i = 0; i < nBytes; i++)
	{
      const size_t stringOffset = i * 2;

   	BYTE val = 0;

      const BYTE b = s[stringOffset];

      if (isdigit(b)) 
      {
         val = (BYTE)((b - '0') * 16); 
      }
      else 
      {
         val = (BYTE)(((toupper(b) - 'A') + 10) * 16); 
      }

      const BYTE b1 = s[stringOffset + 1];

      if (isdigit(b1)) 
      {
			val += b1 - '0' ; 
      }
      else 
      {
         val += (BYTE)((toupper(b1) - 'A') + 10); 
      }

      pBuffer[i] = val;
	}
}

_tstring GetCurrentDirectory()
{
   DWORD size = ::GetCurrentDirectory(0, 0);

   auto_ptr<TCHAR> spBuf(new TCHAR[size]);

   if (0 == ::GetCurrentDirectory(size, spBuf.get()))
   {
      throw CException(_T("GetCurrentDirectory()"), _T("Failed to get current directory"));
   }

   return _tstring(spBuf.get());
}

_tstring GetDateStamp()
{
   SYSTEMTIME systime;
   GetSystemTime(&systime);

   static TCHAR buffer[7];

   _stprintf(buffer, _T("%02d%02d%02d"),
                     systime.wDay,
                     systime.wMonth,
                     ( 1900 + systime.wYear) % 100);

   return buffer;
}

_tstring ToHex(BYTE c)
{
   TCHAR hex[3];
	
   const int val = c;

	_stprintf(hex, _T("%02X"), val);

   return hex;
}

_tstring DumpData(
   const BYTE * const pData, 
   size_t dataLength, 
   size_t lineLength /* = 0 */)
{
   const size_t bytesPerLine = lineLength != 0 ? (lineLength - 1) / 3 : 0;

   _tstring result;

   _tstring hexDisplay;
   _tstring display;

   size_t i = 0;

	while (i < dataLength)
	{
      const BYTE c = pData[i++];

      hexDisplay += ToHex(c) + _T(" ");

      if (isprint(c))
      {
         display += (TCHAR)c;
      }
      else
		{
			display += _T('.');
		}

      if ((bytesPerLine && (i % bytesPerLine == 0 && i != 0)) || i == dataLength)
      {
         if (i == dataLength && (bytesPerLine && (i % bytesPerLine != 0)))
         {
            for (size_t pad = i % bytesPerLine; pad < bytesPerLine; pad++)
            {
               hexDisplay += _T("   ");
            }
         }
         result += hexDisplay + _T(" - ") + display + _T("\n");

         hexDisplay = _T("");
         display = _T("");
      }
	}

   return result;
}

_tstring GetComputerName()
{
   static bool gotName = false;

   static _tstring name = _T("UNAVAILABLE");

   if (!gotName)
   {
      TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1] ;
	   DWORD computerNameLen = MAX_COMPUTERNAME_LENGTH ;

      if (::GetComputerName(computerName, &computerNameLen))
      {
         name = computerName;
      }

      gotName = true;
   }

   return name;
}

_tstring GetModuleFileName(
   HINSTANCE hModule /* = 0 */)
{
   static bool gotName = false;

   static _tstring name = _T("UNAVAILABLE");

   if (!gotName)
   {
      TCHAR moduleFileName[MAX_PATH + 1] ;
	   DWORD moduleFileNameLen = MAX_PATH ;

      if (::GetModuleFileName(hModule, moduleFileName, moduleFileNameLen))
      {
         name = moduleFileName;
      }

      gotName = true;
   }

   return name;
}

_tstring GetUserName()
{
   static bool gotName = false;

   static _tstring name = _T("UNAVAILABLE");

   if (!gotName)
   {
      TCHAR userName[UNLEN + 1] ;
	   DWORD userNameLen = UNLEN;

      if (::GetUserName(userName, &userNameLen))
      {
         name = userName;
      }

      gotName = true;
   }

   return name;
}

_tstring StripLeading(
   const _tstring &source, 
   const char toStrip)
{
   const TCHAR *pSrc = source.c_str();

   while (pSrc && *pSrc == toStrip)
   {
      ++pSrc;
   }

   return pSrc;
}

_tstring StripTrailing(
   const _tstring &source, 
   const char toStrip)
{
   size_t i = source.length();
   const _TCHAR *pSrc = source.c_str() + i;
   
   --pSrc;

   while (i && *pSrc == toStrip)
   {
      --pSrc;
      --i;
   }
   
   return source.substr(0, i);
}

#pragma comment(lib, "Version.lib")

_tstring GetFileVersion()
{
   _tstring version;

   const _tstring moduleFileName = GetModuleFileName(NULL);

   LPTSTR pModuleFileName = const_cast<LPTSTR>(moduleFileName.c_str());

   DWORD zero = 0;

   DWORD verSize = ::GetFileVersionInfoSize(pModuleFileName, &zero);

   if (verSize != 0)
   {
      auto_ptr<BYTE> spBuffer(new BYTE[verSize]);

      if (::GetFileVersionInfo(pModuleFileName, 0, verSize, spBuffer.get()))
      {
         LPTSTR pVersion = 0;
         UINT verLen = 0;

         if (::VerQueryValue(spBuffer.get(), 
              const_cast<LPTSTR>(_T("\\StringFileInfo\\080904b0\\ProductVersion")), 
              (void**)&pVersion, 
              &verLen))
         {
            version = pVersion;
         }
      }
   }

   return version;
}

///////////////////////////////////////////////////////////////////////////////
// Namespace: JetByteTools::Win32
///////////////////////////////////////////////////////////////////////////////

} // End of namespace Win32
} // End of namespace JetByteTools 

///////////////////////////////////////////////////////////////////////////////
// Lint options
//
//lint -restore
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// End of file: Utils.cpp
///////////////////////////////////////////////////////////////////////////////
