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

 #ifndef CALCULATOR_H
 #define CALCULATOR_H

#include <QDialog>
#include "onyx/ui/catalog_view.h"

using namespace ui;

 class QLineEdit;

 class Button;

 class Calculator : public QDialog
 {
     Q_OBJECT

 public:
     Calculator(QWidget *parent = 0);

 protected:
     void keyPressEvent(QKeyEvent *ke);
     void keyReleaseEvent(QKeyEvent *ke);
     void showEvent(QShowEvent * e);

 private slots:
     void digitClicked(const QString &title);
     void unaryOperatorClicked(const QString &title);
     void additiveOperatorClicked(const QString &title);
     void multiplicativeOperatorClicked(const QString &title);
     void equalClicked();
     void pointClicked();
     void changeSignClicked();
     void backspaceClicked();
     void clear();
     void clearAll();
     void clearMemory();
     void readMemory();
     void setMemory();
     void addToMemory();

     void refreshScreen();

     void onItemActivated(CatalogView *catalog, ContentView *item, int user_data);
     void onBack();

     void onAboutToShutDown();

 private:
     void abortOperation();
     bool calculate(double rightOperand, const QString &pendingOperator);

     void createLineButtons(const QVector< QPair<QString, int> > & button_list, CatalogView &view);
     void createAllButtons();

     double sumInMemory;
     double sumSoFar;
     double factorSoFar;
     QString pendingAdditiveOperator;
     QString pendingMultiplicativeOperator;
     bool waitingForOperand;

     enum { eDigitClicked, eUnaryOperatorClicked, eAdditiveOperatorClicked,eMultiplicativeOperatorClicked,
            eEqualClicked, ePointClicked, eChangeSignClicked, eBackspaceClicked, eClear, eClearAll, eClearMemory,
            eReadMemory, eSetMemory, eAddToMemory};

     QLineEdit *display;
     CatalogView first_line_buttons_;
     CatalogView second_line_buttons_;
     CatalogView third_line_buttons_;
     CatalogView fourth_line_buttons_;
     CatalogView fifth_line_buttons_;
     QHBoxLayout back_layout_;
     QPushButton back_;
 };

 #endif
