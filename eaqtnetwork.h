
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

#include <sys/socket.h>
#include <netinet/in.h>

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
    int _socket;
    struct sockaddr_in _address;
    struct sockaddr_in _serv_addr;

    volatile bool _process1_busy;
    volatile bool _process2_busy;
    EAQtDataInterface *_pData;
    QTimer* _network_timer;
    int _rxSize;
    bool setSocketBlockingEnabled(int fd, bool blocking);

public slots:
    void processPacket();
    void process1_ui(QByteArray);
    void process2_no_ui(QByteArray);

signals:
    void go_process1(QByteArray);
    void go_process2(QByteArray);
};

#endif // EAQTNETWORK_H
