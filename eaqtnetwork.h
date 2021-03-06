
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
#ifndef EAQTNETWORK_H
#define EAQTNETWORK_H

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
#include "eaqtdatainterface.h"

class EAQtNetwork : public QObject
{
    Q_OBJECT

public:
    EAQtNetwork(EAQtDataInterface*);
    ~EAQtNetwork();

    bool connectToEA();
    int sendToEA(const char* TxBuf);

    char _RxBuf[NETWORK::RxBufLength];
    //static void paralelProcess(EAQtDataInterface *pd, char* buf, int num);
private:
#ifdef _WIN32
    SOCKET _socket;
    bool setSocketBlockingEnabled(SOCKET fd, bool blocking);
#else
    int _socket;
    bool setSocketBlockingEnabled(int fd, bool blocking);
#endif
    struct sockaddr_in _address;
    struct sockaddr_in _serv_addr;

    volatile bool _process_busy;
    EAQtDataInterface *_pData;
    QTimer* _network_timer;
    int _rxSize;


public slots:
    void checkSocket();
    void startProcessing();


signals:
    void start_processing();
};

#endif // EAQTNETWORK_H
