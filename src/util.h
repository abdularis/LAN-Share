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

#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QDir>
#include <QVector>
#include <QPair>

class Util
{
public:
    static QString sizeToString(qint64 size);

    /*
     *  relative dir name
     *          |        +------> full path to file inside relative dir name
     *          |        |
     * QPair<QString, QString>
     */
    static QVector< QPair<QString, QString> >
        getInnerDirNameAndFullFilePath(const QDir& startingDir, const QString& innerDirName);

    static QString parseAppVersion(bool onlyVerNum = true);

    static QString getUniqueFileName(const QString& fileName, const QString& folderPath);
};

#endif // UTIL_H
