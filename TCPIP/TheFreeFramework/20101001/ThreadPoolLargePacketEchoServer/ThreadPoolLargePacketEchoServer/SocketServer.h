#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef THREAD_POOL_LARGE_PACKET_ECHO_SERVER_SOCKET_SERVER_INCLUDED__
#define THREAD_POOL_LARGE_PACKET_ECHO_SERVER_SOCKET_SERVER_INCLUDED__
///////////////////////////////////////////////////////////////////////////////
// File: SocketServer.h
///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2002 JetByte Limited.
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
//  Class member is a reference
//lint -esym(1725, CSocketServer::m_pool)    
//lint -esym(1725, CPerConnectionData::m_allocator)    
//
// Public data member
//lint -esym(1925, CPerConnectionData::m_messageSize)
//lint -esym(1925, CPerConnectionData::m_pTransferBuffer)
//lint -esym(1925, CPerConnectionData::m_outOfSequenceReads)
//
// Private copy constructor
//lint -esym(1704, CPerConnectionData::CPerConnectionData) 
//
// No default constructor   
//lint -esym(1712, CPerConnectionData)
//
///////////////////////////////////////////////////////////////////////////////

#include "JetByteTools\Win32Tools\SocketServer.h"
#include "JetByteTools\Win32Tools\IOBuffer.h"
#include "JetByteTools\Win32Tools\tstring.h"

///////////////////////////////////////////////////////////////////////////////
// Classes defined in other files...
///////////////////////////////////////////////////////////////////////////////

namespace JetByteTools
{
   namespace Win32
   {
      class CIOCompletionPort;
   }
}

class CThreadPool;

///////////////////////////////////////////////////////////////////////////////
// CSocketServer
///////////////////////////////////////////////////////////////////////////////

class CSocketServer : public JetByteTools::Win32::CSocketServer
{
   public :

      CSocketServer(
         const JetByteTools::Win32::ICriticalSectionFactory &lockFactory,
         JetByteTools::Win32::CSocketServer::IOPool &ioPool,
         unsigned long addressToListenOn,
         unsigned short portToListenOn,
         size_t maxFreeSockets,
         size_t maxFreeBuffers,
         size_t bufferSize,
         size_t numberOfUserDataSlots,
         CThreadPool &businessLogicPool,
         size_t maxMessageSize,
         bool usingReadSequenceNumbers,
         bool usingSequenceNumbers,
         bool issueReadLate);

   private :

      virtual void OnStartAcceptingConnections();
      virtual void OnStopAcceptingConnections();
      virtual void OnShutdownInitiated();
      virtual void OnShutdownComplete();

      virtual void OnConnectionCreated();
      virtual void OnConnectionEstablished(
         Socket *pSocket,
         JetByteTools::Win32::CIOBuffer *pAddress);
      virtual void OnConnectionClientClose(
         Socket *pSocket);
      virtual void OnConnectionReset(
          Socket *pSocket);
      virtual bool OnConnectionClosing(
         Socket *pSocket);
      virtual void OnConnectionClosed(
         Socket *pSocket);
      virtual void OnConnectionDestroyed();
      virtual void OnSocketReleased(
         Socket *pSocket);

      virtual void OnConnectionError(
         ConnectionErrorSource source,
         Socket *pSocket,
         JetByteTools::Win32::CIOBuffer *pBuffer,
         DWORD lastError);

      virtual void OnError(
         const JetByteTools::Win32::_tstring &message);

      virtual void OnBufferCreated();
      virtual void OnBufferAllocated();
      virtual void OnBufferReleased();
      virtual void OnBufferDestroyed();

      virtual void ReadCompleted(
         Socket *pSocket,
         JetByteTools::Win32::CIOBuffer *pBuffer);

      size_t GetMinimumMessageSize() const;

      size_t GetMessageSize(
         const JetByteTools::Win32::CIOBuffer *pBuffer) const;

      void ProcessDataStream(
         Socket *pSocket,
         const JetByteTools::Win32::CIOBuffer *pBuffer);

      void ProcessCommand(
         Socket *pSocket,
         JetByteTools::Win32::CIOBuffer *pBuffer);

      CThreadPool &m_pool;

      JetByteTools::Win32::CIOBuffer::Allocator m_allocator;

      //lint -e{1516} Data member hides inherited data member from Alocator
      JetByteTools::Win32::CCriticalSection m_criticalSection;

      class CPerConnectionData
      {
         public :

            CPerConnectionData(
               JetByteTools::Win32::CCriticalSection &m_criticalSection,
               JetByteTools::Win32::CIOBuffer::Allocator &m_allocator);

            ~CPerConnectionData();

            bool HasBuffer() const;

            void NewBuffer();

            void AddData(
               const BYTE *pData,
               size_t dataLength);

         JetByteTools::Win32::CIOBuffer *RemoveTransferBuffer();

         bool Finished();

         void Finished(
            bool finished);

         size_t m_messageSize;
         JetByteTools::Win32::CIOBuffer *m_pTransferBuffer;
         JetByteTools::Win32::CIOBuffer::InOrderBufferList m_outOfSequenceReads;
         
         private :

            long m_transferSequenceNumber;

            long m_finished;

            JetByteTools::Win32::CIOBuffer::Allocator &m_allocator;

            // No copies, do not implement
            CPerConnectionData(const CPerConnectionData &rhs);
            CPerConnectionData &operator=(const CPerConnectionData &rhs);  
      };

      void SetPerConnectionData(
         Socket *pSocket, 
         CPerConnectionData *pData);
      
      CPerConnectionData *GetPerConnectionData(
         const Socket *pSocket);

      const size_t m_maxMessageSize;

      const bool m_usingSequenceNumbers;

      const bool m_issueEarlyRead;

      const size_t m_userDataIndex;

      // No copies do not implement
      CSocketServer(const CSocketServer &rhs);            
      CSocketServer &operator=(const CSocketServer &rhs); 
};

///////////////////////////////////////////////////////////////////////////////
// Lint options
//
//lint -restore
//
///////////////////////////////////////////////////////////////////////////////


#endif // THREAD_POOL_LARGE_PACKET_ECHO_SERVER_SOCKET_SERVER_INCLUDED__

///////////////////////////////////////////////////////////////////////////////
// End of file: SocketServer.h
///////////////////////////////////////////////////////////////////////////////
