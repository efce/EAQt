
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

#include <stdio.h>
#include <fcntl.h>

#include "eaqtdata.h"
#include "eaqtnetwork.h"


EAQtNetwork::EAQtNetwork(EAQtDataInterface* di) : QObject()
{
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(1,1), &wsa_data);
#endif

    this->_pData = di;
    _rxSize = 0;
    if ((_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0) {
        qDebug() << QT_MESSAGELOG_LINE << "Socket creation error";
        throw("Could not create socket.");
    }
    qDebug() << QT_MESSAGELOG_LINE << "SOCKET ID: " << _socket;

    memset(&_serv_addr, '0', sizeof(_serv_addr));
    _serv_addr.sin_family = AF_INET;
    _serv_addr.sin_port = htons(7001);
    _serv_addr.sin_addr.s_addr = inet_addr("192.168.173.64");

    _process_busy = false;
    _network_timer = nullptr;
    this->connect(this, SIGNAL(start_processing()), this, SLOT(startProcessing()));
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
        connect(_network_timer, SIGNAL(timeout()), this, SLOT(checkSocket()));
        _network_timer->setInterval(20);
        _network_timer->start();
    }
    conSuccess = true;
    qDebug() << QT_MESSAGELOG_LINE << "CONNECT SUCCESS";
    return true;
}


int EAQtNetwork::sendToEA(const char* TxBuf)
{
    qDebug() << QT_MESSAGELOG_LINE << "SENDING...";
    return ::send(_socket, TxBuf, NETWORK::TxBufLength, 0);
}


void EAQtNetwork::checkSocket()
{
    //qDebug() << QT_MESSAGELOG_LINE << "CHECKING SOCKET";
    if (_process_busy) {
        return;
    }
    int bytes_read = ::recv(_socket, _RxBuf, NETWORK::RxBufLength, 0);
    if (bytes_read > 0) {
        emit start_processing();
    }
}


void EAQtNetwork::startProcessing()
{
    _process_busy = true;
    static char RxBuf2[NETWORK::RxBufLength];
    int bytes_read;

    qDebug() << QT_MESSAGELOG_LINE << "PROCESSING ...";
    while ((bytes_read = ::recv(_socket, RxBuf2, NETWORK::RxBufLength, 0)) > 0) {
        qDebug() << QT_MESSAGELOG_LINE << "NEXT DATA SIZE: " << bytes_read << "B";
        _pData->ProcessPacketFromEA(_RxBuf, true);
        qDebug() << QT_MESSAGELOG_LINE << "PACKET PROCESSED";
        memcpy(_RxBuf, RxBuf2, NETWORK::RxBufLength);
    }
    qDebug() << QT_MESSAGELOG_LINE << "FINAL PROCESSING, UPDATING UI";
    _pData->ProcessPacketFromEA(_RxBuf, false);
    qDebug() << QT_MESSAGELOG_LINE << "PROCESSED, UI UPDATED";

    _process_busy = false;
}

