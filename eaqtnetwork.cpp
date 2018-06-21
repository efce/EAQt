
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

EAQtNetwork::EAQtNetwork(EAQtDataInterface* di) : QObject()
{
    this->_EA_IP = "192.168.173.64";
    this->_EA_Port = 7001;
    this->_socket = new QTcpSocket();
    _socket->moveToThread(this->thread());
    this->connect( this->_socket, SIGNAL(error(QAbstractSocket::SocketError)),
                   this,         SLOT(connectionError(QAbstractSocket::SocketError)));
    //this->_socket->setReadBufferSize(10000*NETWORK::RxBufLength); // 600 kB -- it can have backlog of 10000 unprocessed packets.
    this->_socket->setReadBufferSize(0); // UNLIMITED buffer size.
    this->_pData = di;
    //this->_pRxBuf = new char[NETWORK::RxBufLength];
    //memset(this->_pRxBuf,0,NETWORK::RxBufLength);
    _rxSize = 0;

    _process1_busy = false;
    _process2_busy = false;
    this->connect(this, SIGNAL(go_process1(QByteArray)), this, SLOT(process1_ui(QByteArray)));
    this->connect(this, SIGNAL(go_process2(QByteArray)), this, SLOT(process2_no_ui(QByteArray)));
}

EAQtNetwork::~EAQtNetwork()
{
    this->_socket->close();
    delete this->_socket;
}

bool EAQtNetwork::connectToEA()
{
    static bool conSuccess = false;
    if ( conSuccess == true ) {
        return true;
    }
    _socket->setSocketOption(QAbstractSocket::LowDelayOption,true);
    _socket->setSocketOption(QAbstractSocket::KeepAliveOption, true);
    this->_socket->connectToHost(this->_EA_IP, this->_EA_Port);
    this->_socket->waitForConnected();
    conSuccess=(this->_socket->state() == QTcpSocket::ConnectedState);
    if ( conSuccess ) {
        this->connect(this->_socket, SIGNAL(readyRead()),
                      this,         SLOT(processPacket()));
    } else {
        this->_socket->disconnectFromHost();
        this->_pData->NetworkError(tr("Could not connect to electrochemical anlyzer"));
        return false;
    }
    return true;
}

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

int EAQtNetwork::sendToEA(const char* TxBuf)
{
    return static_cast<int>(
        this->_socket->write(
                TxBuf,
                static_cast<qint64>(NETWORK::TxBufLength)
           )
    );
}


void EAQtNetwork::processPacket()
{
    if (_socket->bytesAvailable()) {
        QByteArray rxdata = _socket->readAll();
        if ( false ) {
            throw("Network error occured. Packets size does not match the expected value.");
        }
        if (!_process1_busy) {
            emit go_process1(rxdata);
        } else if (!_process2_busy) {
            emit go_process2(rxdata);
        } else {
            throw('All processes busy.');
        }
    }
}


void EAQtNetwork::process1_ui(QByteArray rxdata)
{
    _process1_busy = true;
    int nextindex = 0;
    char* _pRxBuf = rxdata.data();
    while(nextindex < rxdata.size()) {
        nextindex += NETWORK::RxBufLength;
        bool nextPacketReady = (nextindex < rxdata.size());
        this->_pData->ProcessPacketFromEA(_pRxBuf, nextPacketReady);
        _pRxBuf += NETWORK::RxBufLength;
    }
    _process1_busy = false;
}


void EAQtNetwork::process2_no_ui(QByteArray rxdata)
{
    _process2_busy = true;
    int nextindex = 0;
    char* _pRxBuf = rxdata.data();
    while(nextindex < rxdata.size()) {
        nextindex += NETWORK::RxBufLength;
        bool nextPacketReady = (nextindex < rxdata.size());
        this->_pData->ProcessPacketFromEA(_pRxBuf, true);
        _pRxBuf += NETWORK::RxBufLength;
    }
    _process2_busy = false;
}
