// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SMTPClient.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);  

    LOG = LOG->GetInstance();
    m_gmail_server_clicked  = false;
    m_server_clicked        = true;    
    m_server_address        = m_xml.GetIpAddress();    
    m_server_port           = std::to_string(m_xml.GetListenerPort());    
    m_server_choice         = m_server_address;
    m_settings_ui           = nullptr;

    connect(m_ui->ServerButton, SIGNAL(clicked()), this, SLOT(ServerButtonClicked()));
    connect(m_ui->gmailServerButton, SIGNAL(clicked()), this, SLOT(GmailServerButtonClicked()));
    connect(m_ui->sendButton, SIGNAL(clicked()), this, SLOT(SendButtonClicked()));
    connect(m_ui->exitButton, SIGNAL(clicked()), this, SLOT(ExitButtonClicked()));
}

MainWindow::~MainWindow()
{    
    delete m_settings_ui;
    delete m_ui;
}

void MainWindow::SendButtonClicked()
{
    if (m_server_clicked)
    {
        m_server_choice = m_server_address;
    }
    else
    {
        m_server_choice = GMAIL_SERVER_DOMAIN;
    }

    QString qs = m_ui->lineLogin->text();    
    QString qs2;
    if (!qs.isEmpty())
    {        
        m_login = qs.toLocal8Bit().constData();        
    }
    else
    {
        LOG_WARN << "User tried to send message without specifying login.";
        QMessageBox::critical(this, "Login", "Login field is empty");
        return;
    }

    qs = m_ui->linePassword1->text();
    qs2 = m_ui->linePassword2->text();

    if (!qs.isEmpty() && !qs2.isEmpty())
    {
        if (qs == qs2)
        {            
            m_password = qs.toLocal8Bit().constData();
        }
        else
        {
            LOG_WARN << "User password fields didn't match.";
            QMessageBox::critical(this, "Password", "Passwords don't match");
            return;
        }
    }
    else
    {
        LOG_WARN << "User tried to send message without specifying password.";
        QMessageBox::critical(this, "Password", "One of the passwords field is empty");
        return;
    }

    qs = m_ui->lineRCPT->text();
    if (qs.size())
    {        
        m_rcpt_to = qs.toLocal8Bit().constData();
    }
    else
    {
        LOG_WARN << "User tried to send message without specifying recepient.";
        QMessageBox::critical(this, "Recepient", "Recepient field is empty");
        return;
    }

    qs = m_ui->lineSubject->text();
    if (!qs.isEmpty())
    {        
        m_subject = qs.toLocal8Bit().constData();
    }    

    qs = m_ui->lineData->toPlainText();
    if (qs.size())
    {        
        m_msg_data = qs.toLocal8Bit().constData();
    }
    else
    {
        LOG_WARN << "User tried to send message without entering any letter data.";
        QMessageBox::critical(this, "Letter Data", "Letter data fields are empty");
        return;
    }

    if (m_server_choice == GMAIL_SERVER_DOMAIN)
        this->InitializeSecureSMTPClient();
    else
        this->InitializeSMTPClient();
}

void MainWindow::ServerButtonClicked()
{
    m_server_clicked        = true;
    m_gmail_server_clicked  = false;
}

void MainWindow::GmailServerButtonClicked()
{
    m_gmail_server_clicked  = true;
    m_server_clicked        = false;
}

bool MainWindow::InitializeSMTPClient()
{
    try
    {
        SMTPClientClass client;

        client.set_smtp_address(this->get_server_choice());
        client.set_port(this->get_server_port());
        client.set_server_timeout(m_xml.GetSocketTimeOut());
        client.set_login(this->get_login());
        client.set_password(this->get_password());
        client.set_recepient_email(this->get_rcpt_to());
        client.set_subject(this->get_subject());
        client.set_letter_message(this->get_msg_data());
        client.Send();
    }
    catch (SMTPErrorClass& error)
    {        
        QMessageBox::information(this, "Sending error", error.GetErrorText().c_str());
        return false;
    }

    LOG_INFO << "User message was sent successfully.";
    QMessageBox::information(this, "Message", "Mail was send successfully");

    return true;
}

bool MainWindow::InitializeSecureSMTPClient()
{
    try
    {
        SMTPSecureClientClass client;

        client.set_smtp_address(this->get_server_choice());
        client.set_server_timeout(m_xml.GetSocketTimeOut());        
        client.set_login(this->get_login());
        client.set_password(this->get_password());
        client.set_recepient_email(this->get_rcpt_to());
        client.set_subject(this->get_subject());
        client.set_letter_message(this->get_msg_data());
        client.Send();
    }
    catch (SMTPErrorClass& error)
    {
        QMessageBox::information(this, "Sending error", error.GetErrorText().c_str());
        return false;
    }

    LOG_INFO << "User message was sent successfully.";
    QMessageBox::information(this, "Message", "Mail was send successfully");

    return true;
}

void MainWindow::ExitButtonClicked()
{
    QApplication::quit();
}

std::string MainWindow::get_server_choice() const
{
    if (m_server_choice == GMAIL_SERVER_DOMAIN)
        return GMAIL_SERVER_DOMAIN;
    else
        return m_server_address;
}

std::string MainWindow::get_server_port() const
{
    return m_server_port;
}

std::string MainWindow::get_login() const
{
    return m_login;
}

std::string MainWindow::get_password() const
{
    return m_password;
}

std::string MainWindow::get_rcpt_to() const
{
    return m_rcpt_to;
}

std::string MainWindow::get_subject() const
{
    return m_subject;
}

std::string MainWindow::get_msg_data() const
{
    return m_msg_data;
}

void MainWindow::on_pushButton_clicked()
{
    if (m_settings_ui != nullptr)
        return;
    
    m_settings_ui = new SettingsWindow(this);
    connect(m_settings_ui, SIGNAL(windowClosed()), this, SLOT(ChildWindowClosed()));
    m_settings_ui->show();
}

void MainWindow::ChildWindowClosed()
{
    delete m_settings_ui;
    m_settings_ui = nullptr;
    setWindowModality(Qt::NonModal);
    this->hide();
    this->show();
}

