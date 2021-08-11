// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <QMessageBox>
#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "..\XMLParser\XMLParser.h"
#include "..\XMLCreator\XMLCreator.h"

SettingsWindow::SettingsWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::SettingsWindow)
{
    m_ui->setupUi(this);
    XMLParser parser;    
    m_ui->lineServerAddress->setPlaceholderText(QString::fromLocal8Bit(parser.GetIpAddress().c_str()));
    m_ui->linePort->setPlaceholderText(QString::number(parser.GetListenerPort()));
    m_ui->lineTimeout->setPlaceholderText(QString::number(parser.GetSocketTimeOut()));
    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(SaveButtonClicked()));
    connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(CloseButtonClicked()));
    setWindowModality(Qt::ApplicationModal);
}

SettingsWindow::~SettingsWindow()
{
    delete m_ui;    
}

void SettingsWindow::closeEvent(QCloseEvent* event)
{
    emit windowClosed();
}

void SettingsWindow::CloseButtonClicked()
{
    emit windowClosed();
}

void SettingsWindow::SaveButtonClicked()
{
    QString     qs = m_ui->lineServerAddress->text();
    std::string str_val;
    int         int_val;
    XMLCreator  xml_writer;    

    enum class  INDEX { DEF = -1, FIRST, SECOND, THIRD, FOURTH, FIFTH, SIXTH };    

    if (!qs.isEmpty())
    {        
        str_val = qs.toLocal8Bit().constData();
        xml_writer.SetIpAddress(str_val);
    }

    qs = m_ui->linePort->text();
    if (!qs.isEmpty())
    {
        str_val = qs.toLocal8Bit().constData();
        try
        {
            int_val = stoi(str_val);            
            xml_writer.SetListenerPort(str_val);
        }
        catch (const std::exception&)
        {
            QMessageBox::critical(this, "Port field", "Value for port should be an integer");
            return;
        }
    }
    
    qs = m_ui->lineTimeout->text();
    if (!qs.isEmpty())
    {
        str_val = qs.toLocal8Bit().constData();
        try
        {
            int_val = stoi(str_val);            
            xml_writer.SetSocketTimeOut(str_val);
        }
        catch (const std::exception&)
        {
            QMessageBox::critical(this, "Socket timeout field", "Value for timeout should be an integer");
            return;
        }
    }

    int_val = m_ui->comboLogLevel->currentIndex();    
    
    if (int_val != (int)INDEX::DEF)
    {
        switch (int_val)
        {
        case ((int)INDEX::FIRST):            
            xml_writer.SetLogLevel(std::to_string(++int_val));
            break;
        case ((int)INDEX::SECOND):            
            xml_writer.SetLogLevel(std::to_string(++int_val));
            break;
        case ((int)INDEX::THIRD):            
            xml_writer.SetLogLevel(std::to_string(++int_val));
            break;
        case ((int)INDEX::FOURTH):            
            xml_writer.SetLogLevel(std::to_string(++int_val));
            break;
        case ((int)INDEX::FIFTH):            
            xml_writer.SetLogLevel(std::to_string(++int_val));
            break;
        default:            
            xml_writer.SetLogLevel(std::to_string(++int_val));
            break;
        }        
    }

    if (m_ui->flushOn->isChecked())
    {        
        xml_writer.SetLogFlush("1");
    }
    else if (m_ui->flushOff->isChecked())
    {        
        xml_writer.SetLogFlush("0");
    }

    xml_writer.Write();
    QMessageBox::information(this, "Saved", "Your preferences was successfully saved in XML configuration file");
}

