///////////////////////////////////////////////////////////////////////////////
// File: SocketServer.cpp
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

#include "SocketServer.h"

#include "JetByteTools\Win32Tools\Utils.h"
#include "JetByteTools\Win32Tools\Exception.h"

///////////////////////////////////////////////////////////////////////////////
// Lint options
//
//lint -save
//
// Member not defined
//lint -esym(1526, CSocketServer::CSocketServer)
//lint -esym(1526, CSocketServer::operator=)
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Using directives
///////////////////////////////////////////////////////////////////////////////

using JetByteTools::Win32::CIOCompletionPort;
using JetByteTools::Win32::CIOBuffer;
using JetByteTools::Win32::Output;
using JetByteTools::Win32::ToString;
using JetByteTools::Win32::_tstring;
using JetByteTools::Win32::CException;
using JetByteTools::Win32::DumpData;
using JetByteTools::Win32::GetLastErrorMessage;
using JetByteTools::Win32::ICriticalSectionFactory;

///////////////////////////////////////////////////////////////////////////////
// CSocketServer
///////////////////////////////////////////////////////////////////////////////

CSocketServer::CSocketServer(
   const ICriticalSectionFactory &lockFactory,
   JetByteTools::Win32::CSocketServer::IOPool &pool,
   const std::string &welcomeMessage,
   unsigned long addressToListenOn,
   unsigned short portToListenOn,
   size_t maxFreeSockets,
   size_t maxFreeBuffers,
   size_t bufferSize,
   bool useSequenceNumbers /* = true */)
   :  JetByteTools::Win32::CSocketServer(lockFactory, pool, addressToListenOn, portToListenOn, maxFreeSockets, maxFreeBuffers, bufferSize, useSequenceNumbers),
      m_welcomeMessage(welcomeMessage)
{

}

void CSocketServer::OnStartAcceptingConnections()
{
   Output(_T("OnStartAcceptingConnections"));
}

void CSocketServer::OnStopAcceptingConnections()
{
   Output(_T("OnStopAcceptingConnections"));
}
      
void CSocketServer::OnShutdownInitiated()
{
   Output(_T("OnShutdownInitiated"));
}
      
void CSocketServer::OnShutdownComplete()
{
   Output(_T("OnShutdownComplete"));
}

void CSocketServer::OnConnectionEstablished(
   Socket *pSocket,
   CIOBuffer * /*pAddress*/)
{
   Output(_T("OnConnectionEstablished"));

   pSocket->Write(m_welcomeMessage.c_str(), m_welcomeMessage.length());

   pSocket->Read();
}

void CSocketServer::OnConnectionClientClose(
   Socket * /*pSocket*/)
{
   Output(_T("OnConnectionClientClose"));
}

void CSocketServer::OnConnectionReset(
   Socket * /*pSocket*/,
   DWORD lastError)
{
   Output(_T("OnConnectionReset: ") + GetLastErrorMessage(lastError));
}

bool CSocketServer::OnConnectionClosing(
   Socket *pSocket)
{
   Output(_T("OnConnectionClosing"));

   // we'll handle socket closure so that we can do a lingering close
   // note that this is not ideal as this code is executed on the an
   // IO thread. If we had a thread pool we could fire this off to the
   // thread pool to handle.

   pSocket->Close();

   return true;
}

void CSocketServer::OnConnectionClosed(
   Socket * /*pSocket*/)
{
   Output(_T("OnConnectionClosed"));
}

void CSocketServer::OnConnectionCreated()
{
   Output(_T("OnConnectionCreated"));
}

void CSocketServer::OnConnectionDestroyed()
{
   Output(_T("OnConnectionDestroyed"));
}

void CSocketServer::OnConnectionError(
   ConnectionErrorSource source,
   Socket *pSocket,
   CIOBuffer *pBuffer,
   DWORD lastError)
{
   const LPCTSTR errorSource = (source == ZeroByteReadError ? _T(" Zero Byte Read Error:") : (source == ReadError ? _T(" Read Error:") : _T(" Write Error:")));

   Output(_T("OnConnectionError - Socket = ") + ToString(pSocket) + _T(" Buffer = ") + ToString(pBuffer) + errorSource + GetLastErrorMessage(lastError));
}

void CSocketServer::OnError(
   const JetByteTools::Win32::_tstring &message)
{
   Output(_T("OnError - ") + message);
}

void CSocketServer::OnBufferCreated()
{
   Output(_T("OnBufferCreated"));
}

void CSocketServer::OnBufferAllocated()
{
   Output(_T("OnBufferAllocated"));
}

void CSocketServer::OnBufferReleased()
{
   Output(_T("OnBufferReleased"));
}

void CSocketServer::OnBufferDestroyed()
{
   Output(_T("OnBufferDestroyed"));
}

void CSocketServer::ReadCompleted(
   Socket *pSocket,
   CIOBuffer *pBuffer)
{
   try
   {
      pBuffer = ProcessDataStream(pSocket, pBuffer);

      pSocket->Read(pBuffer);
   }
   catch(const CException &e)
   {
      Output(_T("ReadCompleted - Exception - ") + e.GetWhere() + _T(" - ") + e.GetMessage());
      pSocket->Shutdown();
   }
   catch(...)
   {
      Output(_T("ReadCompleted - Unexpected exception"));
      pSocket->Shutdown();
   }
}

void CSocketServer::EchoMessage(
   Socket *pSocket,
   CIOBuffer *pBuffer) const
{
   DEBUG_ONLY(Output(_T("EchoMessage\n") + DumpData(pBuffer->GetBuffer(), pBuffer->GetUsed(), 40)));

   pSocket->Write(pBuffer);
   //const size_t bytesToEchoInPacket1 = pBuffer->GetUsed()/2;

   //pSocket->Write(pBuffer->GetBuffer(), bytesToEchoInPacket1);
   //pSocket->Write(pBuffer->GetBuffer() + bytesToEchoInPacket1, pBuffer->GetUsed() - pBuffer->GetUsed());
}

size_t CSocketServer::GetMinimumMessageSize() const
{
   return 1;
}

size_t CSocketServer::GetMessageSize(const CIOBuffer *pBuffer) const
{
   size_t messageSize = *pBuffer->GetBuffer();
   
   return messageSize;
}

CIOBuffer *CSocketServer::ProcessDataStream(
   Socket *pSocket,
   CIOBuffer *pBuffer) const
{
   bool done;

   do
   {
      done = true;

      const size_t used = pBuffer->GetUsed();

      if (used >= GetMinimumMessageSize())
      {
         const size_t messageSize = GetMessageSize(pBuffer);

         if (used == messageSize)
         {
            Output(_T("Got complete, distinct, message"));
            // we have a whole, distinct, message

            EchoMessage(pSocket, pBuffer);

            pBuffer = 0;

            done = true;
         }
         else if (used > messageSize)
         {
            Output(_T("Got message plus extra data"));
            // we have a message, plus some more data

            // allocate a new buffer, copy the extra data into it and try again...

            CIOBuffer *pMessage = pBuffer->SplitBuffer(messageSize);
            
            EchoMessage(pSocket, pMessage);

            pMessage->Release();

            // loop again, we may have another complete message in there...
      
            done = false;
         }
         else if (messageSize > pBuffer->GetSize())
         {
            Output(_T("Error: Buffer too small\nExpecting: ") + ToString(messageSize) +
               _T("Got: ") + ToString(pBuffer->GetUsed()) + _T("\nBuffer size = ") + 
               ToString(pBuffer->GetSize()) + _T("\nData = \n") + 
               DumpData(pBuffer->GetBuffer(), pBuffer->GetUsed(), 40));

            pSocket->Shutdown();

            // throw the rubbish away
            pBuffer->Empty();
            
            done = true;
         }
      }
   }
   while (!done);

   // not enough data in the buffer, reissue a read into the same buffer to collect more data
   return pBuffer;
}

///////////////////////////////////////////////////////////////////////////////
// Lint options
//
//lint -restore
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// End of file: SocketServer.cpp
///////////////////////////////////////////////////////////////////////////////
