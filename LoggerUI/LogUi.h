#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LogUi.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>

class LogUi : public QMainWindow
{
    Q_OBJECT

public:
    LogUi(QWidget *parent = Q_NULLPTR);
    ~LogUi();

private:
    Ui::LogUiClass* ui;
    void GetContent(QTextStream& stream);
    void ChangeRowColor();

private slots:
    void OpenFile();
    void Search(const QString& filter);
    void Filter();
};
