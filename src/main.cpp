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

#include <QApplication>
#include <QtDebug>

#include "settings.h"
#include "ui/mainwindow.h"
#include "singleinstance.h"
#include "util.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    SingleInstance si(PROGRAM_NAME);
    if (si.hasPreviousInstance()) {
        return EXIT_SUCCESS;
    }

    if (!si.start()) {
        qDebug() << si.getLastErrorString();
        return EXIT_FAILURE;
    }

    app.setApplicationDisplayName(PROGRAM_NAME);
    app.setApplicationName(PROGRAM_NAME);
    app.setApplicationVersion(Util::parseAppVersion());

    MainWindow mainWindow;
    mainWindow.show();

    QObject::connect(&si, &SingleInstance::newInstanceCreated, [&mainWindow]() {
        mainWindow.setMainWindowVisibility(true);
    });

    return app.exec();
}
