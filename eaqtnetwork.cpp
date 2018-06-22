
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
#include "eaqtdata.h"
#include "eaqtnetwork.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

EAQtNetwork::EAQtNetwork(EAQtDataInterface* di) : QObject()
{

    this->_pData = di;
    //this->_pRxBuf = new char[NETWORK::RxBufLength];
    //memset(this->_pRxBuf,0,NETWORK::RxBufLength);
    _rxSize = 0;
    if ((_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        qDebug() << QT_MESSAGELOG_LINE << "Socket creation error";
        throw("Could not create socket.");
    }
    setSocketBlockingEnabled(_socket, false);
    memset(&_serv_addr, '0', sizeof(_serv_addr));
    _serv_addr.sin_family = AF_INET;
    _serv_addr.sin_port = htons(7001);
    if(inet_pton(AF_INET, "192.168.173.64", &_serv_addr.sin_addr)<=0) {
        qDebug() << QT_MESSAGELOG_LINE << "Wrong IP address.";
    }

    _process1_busy = false;
    _process2_busy = false;
    _network_timer = nullptr;
    this->connect(this, SIGNAL(go_process1(QByteArray)), this, SLOT(process1_ui(QByteArray)));
    this->connect(this, SIGNAL(go_process2(QByteArray)), this, SLOT(process2_no_ui(QByteArray)));
}


bool EAQtNetwork::setSocketBlockingEnabled(int fd, bool blocking)
{
    /** Returns true on success, or false if there was an error */
   if (fd < 0) return false;

#ifdef _WIN32
   unsigned long mode = blocking ? 0 : 1;
   return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}

EAQtNetwork::~EAQtNetwork()
{
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
    if (::connect(_socket, (struct sockaddr *)&_serv_addr, sizeof(_serv_addr)) < 0)
    {
        qDebug() << QT_MESSAGELOG_LINE << "CONNECT FAILED";
        return false;
    }
    if (_network_timer == nullptr) {
        _network_timer = new QTimer(this);
        connect(_network_timer, SIGNAL(timeout()), this, SLOT(processPacket()));
        _network_timer->setInterval(50);
        _network_timer->start();
    }
    conSuccess = true;
    qDebug() << QT_MESSAGELOG_LINE << "CONNECT SUCCESS";
    return true;
}
/*
void EAQtNetwork::connectionError(QAbstractSocket::SocketError error)
{
    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        this->_pData->NetworkError(tr("The connection was closed."));
        break;

    default:
        this->_pData->NetworkError(
             tr("The following error occurred: %1.")
                .arg(this->_socket->errorString())
         );
    }
    return;
}
*/
int EAQtNetwork::sendToEA(const char* TxBuf)
{
    qDebug() << QT_MESSAGELOG_LINE << "SENDING...";
    return ::send(_socket, TxBuf, NETWORK::TxBufLength, NULL);
}

void EAQtNetwork::processPacket()
{
    qDebug() << QT_MESSAGELOG_LINE << "CHECKING SOCKET";
    int bytes_read;
    static char _RxBuf2[NETWORK::RxBufLength];
    bytes_read = ::read(_socket, &_RxBuf, NETWORK::RxBufLength);
    while (bytes_read > 0) {
        qDebug() << QT_MESSAGELOG_LINE << "DATA FOUND, CHECKING NEXT DATA";
        bytes_read = ::read(_socket, &_RxBuf2, NETWORK::RxBufLength);
        qDebug() << QT_MESSAGELOG_LINE << "NEXT DATA SIZE: " << bytes_read;
        qDebug() << QT_MESSAGELOG_LINE << "PROCESSING";
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
