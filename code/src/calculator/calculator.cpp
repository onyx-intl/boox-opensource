 /****************************************************************************
 **
 ** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
 **     the names of its contributors may be used to endorse or promote
 **     products derived from this software without specific prior written
 **     permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

 #include <QtGui>

 #include <math.h>

 #include "button.h"
 #include "calculator.h"
 #include "onyx/screen/screen_update_watcher.h"
#include "onyx/ui/keyboard_navigator.h"
#include "button_view.h"
#include "onyx/data/data_tags.h"

using namespace ui;
CalculatorFactory factory;

 Calculator::Calculator(QWidget *parent)
     : QDialog(parent, Qt::FramelessWindowHint)
     , first_line_buttons_(&factory, this)
     , second_line_buttons_(&factory, this)
     , third_line_buttons_(&factory, this)
     , fourth_line_buttons_(&factory, this)
     , fifth_line_buttons_(&factory, this)
 {
     sumInMemory = 0.0;
     sumSoFar = 0.0;
     factorSoFar = 0.0;
     waitingForOperand = true;

     display = new QLineEdit("0");
     display->setReadOnly(true);
     QFont line_edit_font;
     line_edit_font.setPointSize(45);
     display->setFont(line_edit_font);
     display->setAlignment(Qt::AlignRight);
     display->setMaxLength(15);
     display->setFocusPolicy(Qt::NoFocus);

     QFont font = display->font();
     font.setPointSize(font.pointSize() + 8);
     display->setFont(font);

     QVBoxLayout *mainLayout = new QVBoxLayout;
     mainLayout->setSizeConstraint(QLayout::SetMaximumSize);
     mainLayout->setSpacing(10);

     mainLayout->addWidget(display);

     createAllButtons();
     mainLayout->addWidget(&first_line_buttons_);
     mainLayout->addWidget(&second_line_buttons_);
     mainLayout->addWidget(&third_line_buttons_);
     mainLayout->addWidget(&fourth_line_buttons_);
     mainLayout->addWidget(&fifth_line_buttons_);
     mainLayout->addStretch();

     first_line_buttons_.setNeighbor(&second_line_buttons_, CatalogView::DOWN);
     second_line_buttons_.setNeighbor(&third_line_buttons_, CatalogView::DOWN);
     third_line_buttons_.setNeighbor(&fourth_line_buttons_, CatalogView::DOWN);
     fourth_line_buttons_.setNeighbor(&fifth_line_buttons_, CatalogView::DOWN);
     fifth_line_buttons_.setNeighbor(&first_line_buttons_, CatalogView::DOWN);

     setLayout(mainLayout);

     setWindowTitle(tr("Calculator"));
}

void Calculator::createAllButtons()
{
     QVector< QPair<QString, int> > button_list;

     //first_line
     {
         button_list.clear();
         button_list.push_back(QPair<QString, int>(tr("Backspace"), eBackspaceClicked));
         button_list.push_back(QPair<QString, int>(tr("Clear"), eClear));
         button_list.push_back(QPair<QString, int>(tr("Clear All"), eClearAll));

         createLineButtons(button_list, first_line_buttons_);
     }

     //second_line
     {
         button_list.clear();
         button_list.push_back( QPair<QString, int>("MC", eClearMemory) );
         button_list.push_back(QPair<QString, int>("7", eDigitClicked));
         button_list.push_back(QPair<QString, int>("8", eDigitClicked));
         button_list.push_back(QPair<QString, int>("9", eDigitClicked));
         button_list.push_back(QPair<QString, int>("\367", eMultiplicativeOperatorClicked));
         button_list.push_back(QPair<QString, int>("Sqrt", eUnaryOperatorClicked));

         createLineButtons(button_list, second_line_buttons_);
     }

     //third_line
     {
         button_list.clear();
         button_list.push_back(QPair<QString, int>("MR",eReadMemory));
         button_list.push_back(QPair<QString, int>("4", eDigitClicked));
         button_list.push_back(QPair<QString, int>("5", eDigitClicked));
         button_list.push_back(QPair<QString, int>("6", eDigitClicked));
         button_list.push_back(QPair<QString, int>("\327", eMultiplicativeOperatorClicked));
         button_list.push_back(QPair<QString, int>("x\262", eUnaryOperatorClicked));

         createLineButtons(button_list, third_line_buttons_);
     }

     //fourth_line
     {
         button_list.clear();
         button_list.push_back(QPair<QString, int>("MS", eSetMemory));
         button_list.push_back(QPair<QString, int>("1", eDigitClicked));
         button_list.push_back(QPair<QString, int>("2", eDigitClicked));
         button_list.push_back(QPair<QString, int>("3", eDigitClicked));
         button_list.push_back(QPair<QString, int>("-", eAdditiveOperatorClicked));
         button_list.push_back(QPair<QString, int>("1/x", eUnaryOperatorClicked));

         createLineButtons(button_list, fourth_line_buttons_);
     }

     //fifth_line
     {
         button_list.clear();
         button_list.push_back(QPair<QString, int>("M+", eAddToMemory));
         button_list.push_back(QPair<QString, int>("0", eDigitClicked));
         button_list.push_back(QPair<QString, int>(".", ePointClicked));
         button_list.push_back(QPair<QString, int>("\261", eChangeSignClicked));
         button_list.push_back(QPair<QString, int>("+", eAdditiveOperatorClicked));
         button_list.push_back(QPair<QString, int>("=", eEqualClicked));

         createLineButtons(button_list, fifth_line_buttons_);
     }
 }

 void Calculator::createLineButtons(const QVector< QPair<QString, int> > & button_list, CatalogView &view)
 {
     ODatas button_data;
     view.setSubItemType(ButtonView::type());
     view.setPreferItemSize(QSize(80, 80));

     for (int i = 0; i < button_list.size(); ++i)
     {
         OData * dd = new OData;
         dd->insert(TAG_TITLE, button_list[i].first);
         dd->insert(TAG_ID, button_list[i].second);
         dd->insert(TAG_FONT_SIZE, 32);
         button_data.push_back(dd);
     }

     view.setData(button_data);
     view.setFixedGrid(1, button_list.size());
     view.setSpacing(10);
     view.setMinimumHeight(85);

     view.setSearchPolicy(CatalogView::AutoHorRecycle|CatalogView::NeighborFirst);
     connect(&view, SIGNAL(itemActivated(CatalogView*,ContentView*,int)),
             this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
 }

 void Calculator::keyPressEvent(QKeyEvent *ke)
 {
     ke->accept();
 }

 void Calculator::keyReleaseEvent(QKeyEvent *ke)
 {
     int key = ke->key();
     if (key == Qt::Key_Escape)
     {
         ke->accept();
         reject();
         return;
     }
     else if (key == Qt::Key_Up || key == Qt::Key_Down
             || key == Qt::Key_Left || key == Qt::Key_Right)
     {
         update();
         onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW,
                 onyx::screen::ScreenCommand::WAIT_NONE);
         return;
     }
     QDialog::keyReleaseEvent(ke);
 }

 void Calculator::refreshScreen()
 {
    display->repaint();
    display->update();
    update();
    onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::DW);
 }

 void Calculator::digitClicked(const QString &title)
 {
     int digitValue = title.toInt();
     if (display->text() == "0" && digitValue == 0.0)
         return;

     if (waitingForOperand) {
         display->clear();
         waitingForOperand = false;
     }
     display->setText(display->text() + QString::number(digitValue));

     refreshScreen();
 }

 void Calculator::unaryOperatorClicked(const QString &title)
 {
     QString clickedOperator = title;
     double operand = display->text().toDouble();
     double result = 0.0;

     if (clickedOperator == "Sqrt") {
         if (operand < 0.0) {
             abortOperation();
             return;
         }
         result = sqrt(operand);
     } else if (clickedOperator == "x\262") {
         result = pow(operand, 2.0);
     } else if (clickedOperator == "1/x") {
         if (operand == 0.0) {
             abortOperation();
             return;
         }
         result = 1.0 / operand;
     }
     display->setText(QString::number(result));
     waitingForOperand = true;

     refreshScreen();
 }

 void Calculator::additiveOperatorClicked(const QString &title)
 {
     QString clickedOperator = title;
     double operand = display->text().toDouble();

     if (!pendingMultiplicativeOperator.isEmpty()) {
         if (!calculate(operand, pendingMultiplicativeOperator)) {
             abortOperation();
             return;
         }
         display->setText(QString::number(factorSoFar));
         operand = factorSoFar;
         factorSoFar = 0.0;
         pendingMultiplicativeOperator.clear();
     }

     if (!pendingAdditiveOperator.isEmpty()) {
         if (!calculate(operand, pendingAdditiveOperator)) {
             abortOperation();
             return;
         }
         display->setText(QString::number(sumSoFar));
     } else {
         sumSoFar = operand;
     }

     pendingAdditiveOperator = clickedOperator;
     waitingForOperand = true;

 }

 void Calculator::multiplicativeOperatorClicked(const QString &title)
 {
     QString clickedOperator = title;
     double operand = display->text().toDouble();

     if (!pendingMultiplicativeOperator.isEmpty()) {
         if (!calculate(operand, pendingMultiplicativeOperator)) {
             abortOperation();
             return;
         }
         display->setText(QString::number(factorSoFar));
     } else {
         factorSoFar = operand;
     }

     pendingMultiplicativeOperator = clickedOperator;
     waitingForOperand = true;

 }

 void Calculator::equalClicked()
 {
     double operand = display->text().toDouble();

     if (!pendingMultiplicativeOperator.isEmpty()) {
         if (!calculate(operand, pendingMultiplicativeOperator)) {
             abortOperation();
             return;
         }
         operand = factorSoFar;
         factorSoFar = 0.0;
         pendingMultiplicativeOperator.clear();
     }
     if (!pendingAdditiveOperator.isEmpty()) {
         if (!calculate(operand, pendingAdditiveOperator)) {
             abortOperation();
             return;
         }
         pendingAdditiveOperator.clear();
     } else {
         sumSoFar = operand;
     }

     display->setText(QString::number(sumSoFar));
     sumSoFar = 0.0;
     waitingForOperand = true;

     refreshScreen();
 }

 void Calculator::pointClicked()
 {
     if (waitingForOperand)
         display->setText("0");
     if (!display->text().contains("."))
         display->setText(display->text() + ".");
     waitingForOperand = false;

     refreshScreen();
 }

 void Calculator::changeSignClicked()
 {
     QString text = display->text();
     double value = text.toDouble();

     if (value > 0.0) {
         text.prepend("-");
     } else if (value < 0.0) {
         text.remove(0, 1);
     }
     display->setText(text);

     refreshScreen();
 }

 void Calculator::backspaceClicked()
 {
     if (waitingForOperand)
         return;

     QString text = display->text();
     text.chop(1);
     if (text.isEmpty()) {
         text = "0";
         waitingForOperand = true;
     }
     display->setText(text);

     refreshScreen();
 }

 void Calculator::clear()
 {
     if (waitingForOperand)
         return;

     display->setText("0");
     waitingForOperand = true;

     refreshScreen();
 }

 void Calculator::clearAll()
 {
     sumSoFar = 0.0;
     factorSoFar = 0.0;
     pendingAdditiveOperator.clear();
     pendingMultiplicativeOperator.clear();
     display->setText("0");
     waitingForOperand = true;

     refreshScreen();
 }

 void Calculator::clearMemory()
 {
     sumInMemory = 0.0;
 }

 void Calculator::readMemory()
 {
     display->setText(QString::number(sumInMemory));
     waitingForOperand = true;
 }

 void Calculator::setMemory()
 {
     equalClicked();
     sumInMemory = display->text().toDouble();
 }

 void Calculator::addToMemory()
 {
     equalClicked();
     sumInMemory += display->text().toDouble();
 }

 void Calculator::abortOperation()
 {
     clearAll();
     display->setText("####");
 }

 bool Calculator::calculate(double rightOperand, const QString &pendingOperator)
 {
     if (pendingOperator == "+") {
         sumSoFar += rightOperand;
     } else if (pendingOperator == "-") {
         sumSoFar -= rightOperand;
     } else if (pendingOperator == "\327") {
         factorSoFar *= rightOperand;
     } else if (pendingOperator == "\367") {
         if (rightOperand == 0.0)
             return false;
         factorSoFar /= rightOperand;
     }
     return true;
 }

void Calculator::onItemActivated(CatalogView *catalog, ContentView *item, int user_data)
{
    if (!item || !item->data())
    {
        return;
    }

    OData * item_data = item->data();
    int method = item_data->value(TAG_ID).toInt();
    QString title = item_data->value(TAG_TITLE).toString();

    switch(method)
    {
    case eDigitClicked:
        digitClicked(title);
        break;
    case eUnaryOperatorClicked:
        unaryOperatorClicked(title);
        break;
    case eAdditiveOperatorClicked:
        additiveOperatorClicked(title);
        break;
    case eMultiplicativeOperatorClicked:
        multiplicativeOperatorClicked(title);
        break;
    case eEqualClicked:
        equalClicked();
        break;
    case ePointClicked:
        pointClicked();
        break;
    case eChangeSignClicked:
        changeSignClicked();
        break;
    case eBackspaceClicked:
        backspaceClicked();
        break;
    case eClear:
        clear();
        break;
    case eClearAll:
        clearAll();
        break;
    case eClearMemory:
        clearMemory();
        break;
    case eReadMemory:
        readMemory();
        break;
    case eSetMemory:
        setMemory();
        break;
    case eAddToMemory:
        addToMemory();
        break;
    }
}

void Calculator::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    second_line_buttons_.setFocus();
    second_line_buttons_.setFocusTo(0, 2);
}
