/*
    Locally - LAN file transfer.
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

#include <QFile>

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "textviewdialog.h"
#include "settings.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    QString version = "v " + QString::number(ProgramXVersion) + "." +
                             QString::number(ProgramYVersion) + "." +
                             QString::number(ProgramZVersion) +
                      " (" + QString(OS_NAME) + ")";
    ui->programNameLbl->setText(ProgramName);
    ui->programVersionLbl->setText(version);
    ui->programDescLbl->setText(ProgramDescription);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::onCreditsClicked()
{
    QFile file(":/text/credits.html");
    file.open(QIODevice::ReadOnly);

    TextViewDialog dialog(QString(file.readAll()));
    dialog.setWindowTitle(tr("Credits"));
    dialog.setMinimumSize(320, 230);
    dialog.setMaximumSize(320, 230);
    dialog.exec();
}

void AboutDialog::onLicenseClicked()
{
    QFile file(":/text/gpl-3.0.txt");
    file.open(QIODevice::ReadOnly);

    TextViewDialog dialog(QString(file.readAll()), this);
    dialog.setWindowTitle(tr("License"));
    dialog.setMinimumSize(500, 300);
    dialog.setMaximumSize(500, 300);
    dialog.exec();
}
