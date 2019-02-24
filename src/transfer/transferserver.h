/*
    LANShare - LAN file transfer.
    Copyright (C) 2016 Abdul Aris R. <abdularisrahmanudin10@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRANSFERSERVER_H
#define TRANSFERSERVER_H

#include <QTcpServer>
#include <QObject>

#include "receiver.h"
#include "model/devicelistmodel.h"

class TransferServer : public QObject
{
    Q_OBJECT

public:
    explicit TransferServer(DeviceListModel* devList, QObject *parent = nullptr);

    bool listen(const QHostAddress& addr = QHostAddress::Any);

Q_SIGNALS:
    void newReceiverAdded(Receiver* receiver);

private Q_SLOTS:
    void onNewConnection();

private:
    DeviceListModel* mDevList;
    QTcpServer* mServer;
    QVector<Receiver*> mReceivers;
};

#endif // TRANSFERSERVER_H
