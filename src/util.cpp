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

#include "util.h"

Util::Util()
{
}

QString Util::fileSizeToString(qint64 size)
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
