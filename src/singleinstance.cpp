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

#include <QtNetwork/QLocalSocket>

#include "singleinstance.h"

SingleInstance::SingleInstance(const QString& name, QObject* parent)
    : QObject(parent), mName(name)
{
    connect(&mServer, &QLocalServer::newConnection, this, &SingleInstance::onNewConnection);
}

QString SingleInstance::getLastErrorString() const
{
    return mServer.errorString();
}

bool SingleInstance::start()
{
    mServer.removeServer(mName);
    return mServer.listen(mName);
}

bool SingleInstance::hasPreviousInstance()
{
    QLocalSocket socket;
    socket.connectToServer(mName);

    return socket.waitForConnected();
}

void SingleInstance::onNewConnection()
{
    emit newInstanceCreated();
}
