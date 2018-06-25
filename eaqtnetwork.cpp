
  /*****************************************************************************************************************
  *  Electrochmical analyzer software EAQt to be used with 8KCA and M161
  *
  *  Copyright (C) 2017  Filip Ciepiela <filip.ciepiela@agh.edu.pl> and Małgorzata Jakubowska <jakubows@agh.edu.pl>
  *  This program is free software; you can redistribute it and/or modify 
  *  it under the terms of the GNU General Public License as published by
  *  the Free Software Foundation; either version 3 of the License, or
  *  (at your option) any later version.
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software Foundation,
  *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
  *******************************************************************************************************************/



#ifdef _WIN32
  /* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501  /* Windows XP. */
  #endif
  #include <winsock2.h>
  #include <Ws2tcpip.h>
#else
  /* Assume that any non-Windows platform uses POSIX-style sockets instead. */
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <unistd.h> /* Needed for close() */
#endif
#include <stdio.h>
#include <fcntl.h>

#include "eaqtdata.h"
#include "eaqtnetwork.h"

EAQtNetwork::EAQtNetwork(EAQtDataInterface* di) : QObject()
{


#ifdef _WIN32
    WSADATA wsa_data = {0};
    WSAStartup(MAKEWORD(1,1), &wsa_data);
#endif

    this->_pData = di;
    _rxSize = 0;
    if ((_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        qDebug() << QT_MESSAGELOG_LINE << "Socket creation error";
        throw("Could not create socket.");
    }
    qDebug() << QT_MESSAGELOG_LINE << "SOCKET ID: " << _socket;

    memset(&_serv_addr, '0', sizeof(_serv_addr));
    _serv_addr.sin_family = AF_INET;
    _serv_addr.sin_port = htons(7001);
    _serv_addr.sin_addr.s_addr = inet_addr("192.168.173.64");

    _process1_busy = false;
    _process2_busy = false;
    _network_timer = nullptr;
    this->connect(this, SIGNAL(go_process1(QByteArray)), this, SLOT(process1_ui(QByteArray)));
    this->connect(this, SIGNAL(go_process2(QByteArray)), this, SLOT(process2_no_ui(QByteArray)));
}


#ifdef _WIN32
bool EAQtNetwork::setSocketBlockingEnabled(SOCKET fd, bool blocking)
{
   unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
}
#else
bool EAQtNetwork::setSocketBlockingEnabled(int fd, bool blocking)
{
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
}
#endif

EAQtNetwork::~EAQtNetwork()
{
#ifdef _WIN32
    WSACleanup();
#endif
    ::shutdown(_socket, 2);
}

bool EAQtNetwork::connectToEA()
{
    qDebug() << QT_MESSAGELOG_LINE << "CONNECTING";
    static bool conSuccess = false;
    if ( conSuccess == true ) {
        qDebug() << QT_MESSAGELOG_LINE << "WAS CONNECTED";
        return true;
    }

    if (::connect(_socket, (struct sockaddr *)&_serv_addr, sizeof(_serv_addr)) != 0)
    {
        if (errno != EINPROGRESS) {
            qDebug() << QT_MESSAGELOG_LINE << "CONNECT FAILED: " << errno;
            return false;
        }
    }
    bool setblock = setSocketBlockingEnabled(_socket, false);
    qDebug() << QT_MESSAGELOG_LINE << "SET BLOCKING SUCCESS? " << setblock;
    if (_network_timer == nullptr) {
        _network_timer = new QTimer(this);
        connect(_network_timer, SIGNAL(timeout()), this, SLOT(processPacket()));
        _network_timer->setInterval(10);
        _network_timer->start();
    }
    conSuccess = true;
    qDebug() << QT_MESSAGELOG_LINE << "CONNECT SUCCESS";
    return true;
}


int EAQtNetwork::sendToEA(const char* TxBuf)
{
    qDebug() << QT_MESSAGELOG_LINE << "SENDING...";
    return ::send(_socket, TxBuf, NETWORK::TxBufLength, NULL);
}

void EAQtNetwork::processPacket()
{
    //qDebug() << QT_MESSAGELOG_LINE << "CHECKING SOCKET";
    int bytes_read;
    static char _RxBuf2[NETWORK::RxBufLength];
    bytes_read = ::recv(_socket, _RxBuf, NETWORK::RxBufLength, 0);
    while (bytes_read > 0) {
        qDebug() << QT_MESSAGELOG_LINE << "DATA FOUND, CHECKING NEXT DATA";
        bytes_read = ::recv(_socket, _RxBuf2, NETWORK::RxBufLength, 0);
        qDebug() << QT_MESSAGELOG_LINE << "NEXT DATA SIZE: " << bytes_read << "B, PROCESSING";
        _pData->ProcessPacketFromEA(_RxBuf, (bytes_read>0?true:false));
        qDebug() << QT_MESSAGELOG_LINE << "PROCESSED";
        memcpy(&_RxBuf, &_RxBuf2, NETWORK::RxBufLength);
    }
}


void EAQtNetwork::process1_ui(QByteArray rxdata)
{
    _process1_busy = true;
    int nextindex = 0;
    char* rx_ptr = rxdata.data();
    while(nextindex < rxdata.size()) {
        nextindex += NETWORK::RxBufLength;
        bool nextPacketReady = (nextindex < rxdata.size());
        this->_pData->ProcessPacketFromEA(rx_ptr, nextPacketReady);
        rx_ptr += NETWORK::RxBufLength;
    }
    _process1_busy = false;
}


void EAQtNetwork::process2_no_ui(QByteArray rxdata)
{
    qDebug() << "===> 2 <=====";
    _process2_busy = true;
    int nextindex = 0;
    char* rx_ptr = rxdata.data();
    while(nextindex < rxdata.size()) {
        nextindex += NETWORK::RxBufLength;
        //bool nextPacketReady = (nextindex < rxdata.size());
        this->_pData->ProcessPacketFromEA(rx_ptr, true);
        rx_ptr += NETWORK::RxBufLength;
    }
    _process2_busy = false;
}
