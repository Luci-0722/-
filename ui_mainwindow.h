/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *display;
    QPushButton *caption_in;
    QPushButton *stop;
    QPushButton *img_video_in;
    QLCDNumber *personNumLCD;
    QLabel *label;
    QTextBrowser *textBrowser;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1024, 766);
        MainWindow->setMinimumSize(QSize(1, 1));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        display = new QPushButton(centralwidget);
        display->setObjectName(QString::fromUtf8("display"));
        display->setGeometry(QRect(310, 700, 110, 40));
        caption_in = new QPushButton(centralwidget);
        caption_in->setObjectName(QString::fromUtf8("caption_in"));
        caption_in->setGeometry(QRect(160, 700, 110, 40));
        stop = new QPushButton(centralwidget);
        stop->setObjectName(QString::fromUtf8("stop"));
        stop->setGeometry(QRect(470, 700, 110, 40));
        img_video_in = new QPushButton(centralwidget);
        img_video_in->setObjectName(QString::fromUtf8("img_video_in"));
        img_video_in->setGeometry(QRect(10, 700, 110, 40));
        personNumLCD = new QLCDNumber(centralwidget);
        personNumLCD->setObjectName(QString::fromUtf8("personNumLCD"));
        personNumLCD->setGeometry(QRect(870, 115, 111, 61));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(700, 100, 151, 91));
        QFont font;
        font.setPointSize(21);
        label->setFont(font);
        textBrowser = new QTextBrowser(centralwidget);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        textBrowser->setGeometry(QRect(680, 230, 321, 461));
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        display->setText(QCoreApplication::translate("MainWindow", "\346\222\255\346\224\276/\346\232\202\345\201\234", nullptr));
        caption_in->setText(QCoreApplication::translate("MainWindow", "\346\221\204\345\203\217\345\244\264\350\276\223\345\205\245", nullptr));
        stop->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242", nullptr));
        img_video_in->setText(QCoreApplication::translate("MainWindow", "\345\233\276\347\211\207/\350\247\206\351\242\221\350\276\223\345\205\245", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\345\275\223\345\211\215\344\272\272\346\225\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
