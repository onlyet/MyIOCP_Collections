#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef JETBYTE_TOOLS_WIN32_SOCKET_INCLUDED__
#define JETBYTE_TOOLS_WIN32_SOCKET_INCLUDED__
///////////////////////////////////////////////////////////////////////////////
// File: Socket.h
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

///////////////////////////////////////////////////////////////////////////////
// Lint options
//
//lint -save
//
// Private constructor
//lint -esym(1704, CSocket::CSocket)
//lint -esym(1704, Exception::Exception)
//
// No default constructor
//lint -esym(1712, CSocket)
//lint -esym(1712, InternetAddress)
//lint -esym(1712, Exception)
//
//lint -e537 repeated include winsock2.h
//
///////////////////////////////////////////////////////////////////////////////

#include "WinsockWrapper.h"
#include "Win32Exception.h"
#include "tstring.h"

///////////////////////////////////////////////////////////////////////////////
// Namespace: JetByteTools::Win32
///////////////////////////////////////////////////////////////////////////////

namespace JetByteTools {
namespace Win32 {

///////////////////////////////////////////////////////////////////////////////
// CSocket
///////////////////////////////////////////////////////////////////////////////

class CSocket  
{
   public:

      class InternetAddress;
      class Exception;

      CSocket();

      explicit CSocket(
         SOCKET theSocket);
      
      ~CSocket();

      void Attach(
         SOCKET theSocket);

      SOCKET Detatch();

      void Close();

      void AbortiveClose();

      void Shutdown(
         int how);

      void Listen(
         int backlog);

      void Bind(
         const SOCKADDR_IN &address);

      void Bind(
         const struct sockaddr &address,   
         size_t addressLength);

      void Connect(
         const SOCKADDR_IN &address);

      void Connect(
         const struct sockaddr &address,   
         size_t addressLength);

   private :

      SOCKET m_socket;

      // No copies do not implement
      CSocket(const CSocket &rhs);
      CSocket &operator=(const CSocket &rhs);
};

///////////////////////////////////////////////////////////////////////////////
// CSocket::InternetAddress
///////////////////////////////////////////////////////////////////////////////

class CSocket::InternetAddress : public sockaddr_in
{
   public :
      
      InternetAddress(
         const unsigned long address,
         const unsigned short port);

      InternetAddress(
         const _tstring &address,
         const unsigned short port);
};

///////////////////////////////////////////////////////////////////////////////
// CSocket::Exception
///////////////////////////////////////////////////////////////////////////////

class CSocket::Exception : public CWin32Exception
{
   friend class CSocket;
   friend class CSocket::InternetAddress;

   private :

      Exception(
         const _tstring &where, 
         DWORD error);
};

///////////////////////////////////////////////////////////////////////////////
// Namespace: JetByteTools::Win32
///////////////////////////////////////////////////////////////////////////////

} // End of namespace Win32
} // End of namespace JetByteTools 

#endif // JETBYTE_TOOLS_WIN32_SOCKET_INCLUDED__

///////////////////////////////////////////////////////////////////////////////
// Lint options
//
//lint -restore
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// End of file: Socket.h
///////////////////////////////////////////////////////////////////////////////
