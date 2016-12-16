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

#include <QFile>

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "settings.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog),
    mCredits(""), mLicense("")
{
    ui->setupUi(this);

    QString version = "v " + QString::number(ProgramXVersion) + "." +
                             QString::number(ProgramYVersion) + "." +
                             QString::number(ProgramZVersion) +
                      " (" + QString(OS_NAME) + ")";
    ui->programNameLbl->setText(ProgramName);
    ui->programVersionLbl->setText(version);
    ui->programDescLbl->setText(ProgramDescription);

    ui->textEdit->setVisible(false);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::onCreditsClicked(bool checked)
{
    if (checked) {
        if (mCredits.isEmpty()) {
            QFile file(":/text/credits.html");
            file.open(QIODevice::ReadOnly);
            mCredits = file.readAll();
        }
        ui->textEdit->setText(mCredits);
    }

    ui->textContent->setVisible(!checked);
    ui->textEdit->setVisible(checked);
    ui->licenseBtn->setChecked(false);
}

void AboutDialog::onLicenseClicked(bool checked)
{
    if (checked) {
        if (mLicense.isEmpty()) {
            QFile file(":/text/gpl-3.0.txt");
            file.open(QIODevice::ReadOnly);
            mLicense = file.readAll();
        }
        ui->textEdit->setText(mLicense);
    }

    ui->textContent->setVisible(!checked);
    ui->textEdit->setVisible(checked);
    ui->creditBtn->setChecked(false);
}
