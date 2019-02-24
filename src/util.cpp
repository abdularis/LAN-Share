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

#include <QFileInfo>

#include "util.h"
#include "settings.h"

QString Util::sizeToString(qint64 size)
{
    int count = 0;
    double f_size = size;
    while (f_size >= 1024) {
        f_size /= 1024;
        count++;
    }

    QString suffix;
    switch (count) {
    case 0 : suffix = " B"; break;
    case 1 : suffix = " KB"; break;
    case 2 : suffix = " MB"; break;
    case 3 : suffix = " GB"; break;
    case 4 : suffix = " TB"; break;
    }

    return QString::number(f_size, 'f', 2).append(suffix);
}

QVector< QPair<QString, QString> >
    Util::getInnerDirNameAndFullFilePath(const QDir& startingDir, const QString& innerDirName)
{
    QVector< QPair<QString, QString> > pairs;

    QFileInfoList fiList = startingDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
    for (const auto& fi : fiList)
        pairs.push_back( QPair<QString, QString>(innerDirName, fi.filePath()) );

    fiList = startingDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (const auto& fi : fiList) {
        QString newInnerDirName;
        if (innerDirName.isEmpty())
            newInnerDirName = fi.fileName();
        else
            newInnerDirName = innerDirName + QDir::separator() + fi.fileName();

        QVector< QPair<QString, QString> > otherPairs =
                getInnerDirNameAndFullFilePath( QDir(fi.filePath()), newInnerDirName );

        pairs.append(otherPairs);
    }

    return pairs;
}

QString Util::parseAppVersion(bool onlyVerNum)
{
    if (onlyVerNum) {
        return QString::number(PROGRAM_X_VER) + "." +
               QString::number(PROGRAM_Y_VER) + "." +
               QString::number(PROGRAM_Z_VER);
    }

    return "v " + QString::number(PROGRAM_X_VER) + "." +
           QString::number(PROGRAM_Y_VER) + "." +
           QString::number(PROGRAM_Z_VER) +
           " (" + QString(OS_NAME) + ")";
}

/*
 * cek file path (folderName+fileName).
 * jika file dengan nama "fileName" sudah ada
 * maka cek lagi untuk "fileName (1)" jika masih ada chek lagi untuk "fileName (2)" dst.
 * kemudian return file path untuk nama file yang belum ada.
 */
QString Util::getUniqueFileName(const QString& fileName, const QString& folderPath)
{
    int count = 1;
    QString originalFilePath = folderPath + QDir::separator() + fileName;
    QString fPath = originalFilePath;
    while (QFile::exists(fPath)) {
        QFileInfo fInfo(originalFilePath);
        QString baseName = fInfo.baseName() + " (" + QString::number(count) + ")";
        fPath = folderPath + QDir::separator() + baseName + "." + fInfo.completeSuffix();
        count++;
    }

    return fPath;
}
