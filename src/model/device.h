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

#ifndef DEVICE_H
#define DEVICE_H

#include <QtNetwork/QHostAddress>
#include <QObject>

/*
 * class Device merepresentasikan Node/Computer yang terhubung ke jaringan/LAN
 * yang sama dan bisa bertransfer data
 */
class Device
{
public:
    explicit Device();

    inline QString getId() const { return mId; }
    inline QString getName() const { return mName; }
    inline QHostAddress getAddress() const { return mAddress; }
    inline QString getOSName() const { return mOSName; }
    bool isValid() const;

    void set(const QString& id, const QString& name, const QString& osName, const QHostAddress& addr);
    void setId(const QString& id);
    void setName(const QString& name);
    void setAddress(const QHostAddress& address);
    void setOSName(const QString& osName);

    bool operator==(const Device& other) const;
    bool operator!=(const Device& other) const;

private:
    QString mId;
    QString mName;
    QString mOSName;
    QHostAddress mAddress;
};

#endif // DEVICE_H
