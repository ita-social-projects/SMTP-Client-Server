// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <QMessageBox>
#include "settingswindow.h"
#include "ui_settingswindow.h"

SettingsWindow::SettingsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
}

SettingsWindow::~SettingsWindow()
{
    delete ui;    
}

void SettingsWindow::closeEvent(QCloseEvent* event)
{
    emit windowClosed();
}

void SettingsWindow::on_buttonBox_rejected()
{
    emit windowClosed();
}

void SettingsWindow::on_buttonBox_accepted()
{
    QString     qs = ui->lineServerAddress->text();
    std::string str_val;
    int         int_val;

    enum class  INDEX { DEF = -1, FIRST, SECOND, THIRD };    

    if (!qs.isEmpty())
    {
        //TODO call parser for write
    }

    qs = ui->linePort->text();
    if (!qs.isEmpty())
    {
        str_val = qs.toLocal8Bit().constData();
        try
        {
            int_val = stoi(str_val);
            //TODO call parser for write
        }
        catch (const std::exception&)
        {
            QMessageBox::critical(this, "Port field", "Value for port should be an integer");
            return;
        }
    }
    
    qs = ui->lineTimeout->text();
    if (!qs.isEmpty())
    {
        str_val = qs.toLocal8Bit().constData();
        try
        {
            int_val = stoi(str_val);
            //TODO call parser for write
        }
        catch (const std::exception&)
        {
            QMessageBox::critical(this, "Socket timeout field", "Value for timeout should be an integer");
            return;
        }
    }

    int_val = (ui->comboLogLevel->itemData(ui->comboLogLevel->currentIndex())).toInt();
    
    if (int_val != (int)INDEX::DEF)
    {
        switch (int_val)
        {
        case ((int)INDEX::FIRST):
            //TODO call parser for write
            break;
        case ((int)INDEX::SECOND):
            //TODO call parser for write
            break;        
        default:
            //TODO call parser for write
            break;
        }
        //TODO call parser for write
    }

    if (ui->flushOn->isChecked())
    {
        //TODO call parser for write
    }
    else if (ui->flushOff->isChecked())
    {
        //TODO call parser for write
    }

    QMessageBox::information(this, "Saved", "Your preferences was successfully saved in XML configuration file");
}

