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

 Calculator::Calculator(QWidget *parent)
     : QDialog(parent, Qt::FramelessWindowHint)
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

     QFont font = display->font();
     font.setPointSize(font.pointSize() + 8);
     display->setFont(font);

     Button * init_focus;
     for (int i = 0; i < NumDigitButtons; ++i) {
         digitButtons[i] = createButton(QString::number(i), SLOT(digitClicked()));
         if (5 == i)
         {
             init_focus = digitButtons[i];
         }
     }

     Button *pointButton = createButton(".", SLOT(pointClicked()));
     Button *changeSignButton = createButton("\261", SLOT(changeSignClicked()));

     Button *backspaceButton = createButton(tr("Backspace"), SLOT(backspaceClicked()));
     Button *clearButton = createButton(tr("Clear"), SLOT(clear()));
     Button *clearAllButton = createButton(tr("Clear All"), SLOT(clearAll()));

     Button *clearMemoryButton = createButton("MC", SLOT(clearMemory()));
     Button *readMemoryButton = createButton("MR", SLOT(readMemory()));
     Button *setMemoryButton = createButton("MS", SLOT(setMemory()));
     Button *addToMemoryButton = createButton("M+", SLOT(addToMemory()));

     Button *divisionButton = createButton("\367", SLOT(multiplicativeOperatorClicked()));
     Button *timesButton = createButton("\327", SLOT(multiplicativeOperatorClicked()));
     Button *minusButton = createButton("-", SLOT(additiveOperatorClicked()));
     Button *plusButton = createButton("+", SLOT(additiveOperatorClicked()));

     Button *squareRootButton = createButton("Sqrt", SLOT(unaryOperatorClicked()));
     Button *powerButton = createButton("x\262", SLOT(unaryOperatorClicked()));
     Button *reciprocalButton = createButton("1/x", SLOT(unaryOperatorClicked()));
     Button *equalButton = createButton("=", SLOT(equalClicked()));

     QGridLayout *mainLayout = new QGridLayout;
     mainLayout->setSizeConstraint(QLayout::SetMaximumSize);

     mainLayout->addWidget(display, 0, 0, 1, 6);
     mainLayout->addWidget(backspaceButton, 1, 0, 1, 2);
     mainLayout->addWidget(clearButton, 1, 2, 1, 2);
     mainLayout->addWidget(clearAllButton, 1, 4, 1, 2);

     mainLayout->addWidget(clearMemoryButton, 2, 0);
     mainLayout->addWidget(readMemoryButton, 3, 0);
     mainLayout->addWidget(setMemoryButton, 4, 0);
     mainLayout->addWidget(addToMemoryButton, 5, 0);

     for (int i = 1; i < NumDigitButtons; ++i) {
         int row = ((9 - i) / 3) + 2;
         int column = ((i - 1) % 3) + 1;
         mainLayout->addWidget(digitButtons[i], row, column);
     }

     mainLayout->addWidget(digitButtons[0], 5, 1);
     mainLayout->addWidget(pointButton, 5, 2);
     mainLayout->addWidget(changeSignButton, 5, 3);

     mainLayout->addWidget(divisionButton, 2, 4);
     mainLayout->addWidget(timesButton, 3, 4);
     mainLayout->addWidget(minusButton, 4, 4);
     mainLayout->addWidget(plusButton, 5, 4);

     mainLayout->addWidget(squareRootButton, 2, 5);
     mainLayout->addWidget(powerButton, 3, 5);
     mainLayout->addWidget(reciprocalButton, 4, 5);
     mainLayout->addWidget(equalButton, 5, 5);
     setLayout(mainLayout);

     setWindowTitle(tr("Calculator"));

     initFocus(init_focus);
 }

 void Calculator::initFocus(Button * init_focus)
 {
     init_focus->setFocus();
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
         QWidget *wid = ui::moveFocus(this, key);
         if (wid)
         {
             wid->setFocus();
         }
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

 void Calculator::digitClicked()
 {
     Button *clickedButton = qobject_cast<Button *>(sender());
     int digitValue = clickedButton->text().toInt();
     if (display->text() == "0" && digitValue == 0.0)
         return;

     if (waitingForOperand) {
         display->clear();
         waitingForOperand = false;
     }
     display->setText(display->text() + QString::number(digitValue));

     clickedButton->setFocus();
     refreshScreen();
 }

 void Calculator::unaryOperatorClicked()
 {
     Button *clickedButton = qobject_cast<Button *>(sender());
     QString clickedOperator = clickedButton->text();
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

 void Calculator::additiveOperatorClicked()
 {
     Button *clickedButton = qobject_cast<Button *>(sender());
     QString clickedOperator = clickedButton->text();
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

 void Calculator::multiplicativeOperatorClicked()
 {
     Button *clickedButton = qobject_cast<Button *>(sender());
     QString clickedOperator = clickedButton->text();
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
 Button *Calculator::createButton(const QString &text, const char *member)
 {
     Button *button = new Button(text);
     connect(button, SIGNAL(clicked()), this, member);
     return button;
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
