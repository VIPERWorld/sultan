/*
 * checkstockadddialog.cpp
 * Copyright 2017 - ~, Apin <apin.klas@gmail.com>
 *
 * This file is part of Turbin.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "checkstockadddialog.h"
#include "ui_checkstockadddialog.h"
#include "keyevent.h"
#include "guiutil.h"
#include "message.h"
#include "messagebus.h"
#include "global_constant.h"
#include <QMessageBox>
#include <QDebug>

using namespace LibGUI;
using namespace LibG;

CheckStockAddDialog::CheckStockAddDialog(LibG::MessageBus *bus, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CheckStockAddDialog)
{
    ui->setupUi(this);
    setMessageBus(bus);
    auto ke = new KeyEvent(this);
    ke->addConsumeKey(Qt::Key_Return);
    ke->addConsumeKey(Qt::Key_Tab);
    ui->lineBarcode->installEventFilter(ke);
    connect(ke, SIGNAL(keyPressed(QObject*,QKeyEvent*)), SLOT(barcodeDone()));
    connect(ui->doubleStock, SIGNAL(valueChanged(double)), SLOT(checkDiff()));
    connect(ui->pushAdd, SIGNAL(clicked(bool)), SLOT(addClicked()));
    connect(ui->pushAddAgain, SIGNAL(clicked(bool)), SLOT(addAgainClicked()));
}

CheckStockAddDialog::~CheckStockAddDialog()
{
    delete ui;
}

void CheckStockAddDialog::reset()
{
    GuiUtil::enableWidget(false, QList<QWidget*>() << ui->pushAdd << ui->pushAddAgain << ui->doubleStock);
}

void CheckStockAddDialog::messageReceived(LibG::Message *msg)
{
    if(msg->isTypeCommand(MSG_TYPE::ITEM, MSG_COMMAND::GET)) {
        if(msg->isSuccess()) {
            ui->labelName->setText(msg->data("name").toString());
            mLastStock = msg->data("stock").toFloat();
            ui->labelStock->setText(msg->data("stock").toString());
            GuiUtil::enableWidget(true, QList<QWidget*>() << ui->pushAdd << ui->pushAddAgain << ui->doubleStock);
            ui->doubleStock->setValue(0);
            ui->doubleStock->setFocus(Qt::TabFocusReason);
            checkDiff();
        } else {
            QMessageBox::critical(this, tr("Error"), msg->data("error").toString());
        }
    }
}

void CheckStockAddDialog::inputCheckStock()
{
    ui->pushAdd->setEnabled(false);
    ui->pushAddAgain->setEnabled(false);
    if(mLastBarcode.isEmpty()) return;
    if(mLastBarcode != ui->lineBarcode->text()) {
        QMessageBox::warning(this, tr("Error"), tr("Please redo input barcode correctly"));
        return;
    }
}

void CheckStockAddDialog::barcodeDone()
{
    mLastBarcode = ui->lineBarcode->text();
    if(mLastBarcode.isEmpty()) return;
    Message msg(MSG_TYPE::ITEM, MSG_COMMAND::GET);
    msg.addData("barcode", mLastBarcode);
    sendMessage(&msg);
}

void CheckStockAddDialog::addClicked()
{
    mIsAddAgain = false;
    inputCheckStock();
}

void CheckStockAddDialog::addAgainClicked()
{
    mIsAddAgain = true;
    inputCheckStock();

}

void CheckStockAddDialog::checkDiff()
{
    ui->labelDiff->setText(QString::number(ui->doubleStock->value() - mLastStock));
}
