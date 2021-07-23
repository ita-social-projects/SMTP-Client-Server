// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SMTPClient.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);   
    
    m_own_server_address    = m_xml.GetIpAddress();
    m_own_server_port       = std::to_string(m_xml.GetListenerPort());
    m_server_choice         = m_own_server_address;

    connect(ui->ownServerButton, SIGNAL(clicked()), this, SLOT(OwnServerButtonClicked()));
    connect(ui->gmailServerButton, SIGNAL(clicked()), this, SLOT(GmailServerButtonClicked()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(SendButtonClicked()));
    connect(ui->exitButton, SIGNAL(clicked()), this, SLOT(ExitButtonClicked()));
}

MainWindow::~MainWindow()
{        
    delete ui;
}

void MainWindow::SendButtonClicked()
{    
    if (!m_own_server_clicked && !m_gmail_server_clicked)
    {
        LOG_WARN << "User tried to send message without specifying server.";
        QMessageBox::critical(this, "Server", "Choose one of the SMTP servers");
        return;
    }
    if (m_own_server_clicked)
    {
        m_server_choice = m_own_server_address;
    }
    else
    {
        m_server_choice = GMAIL_SERVER_DOMAIN;
    }

    QString qs = ui->lineLogin->text();
    QString qs2;
    if (qs.size())
    {
        m_login = qs.toStdString();
    }
    else
    {
        LOG_WARN << "User tried to send message without specifying login.";
        QMessageBox::critical(this, "Login", "Login field is empty");
        return;
    }

    qs = ui->linePassword1->text();
    qs2 = ui->linePassword2->text();

    if (qs.size() && qs2.size())
    {
        if (qs == qs2)
        {
            m_password = qs.toStdString();
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

    qs = ui->lineRCPT->text();
    if (qs.size())
    {
        m_rcpt_to = qs.toStdString();
    }
    else
    {
        LOG_WARN << "User tried to send message without specifying recepient.";
        QMessageBox::critical(this, "Recepient", "Recepient field is empty");
        return;
    }

    qs = ui->lineSubject->text();
    if (qs.size())
    {
        m_subject = qs.toStdString();
    }
    else
    {
        LOG_WARN << "User tried to send message without specifying subject.";
        QMessageBox::critical(this, "Subject", "Subject field is empty");
        return;
    }

    qs = ui->lineData->toPlainText();
    if (qs.size())
    {
        m_msg_data = qs.toStdString();
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

void MainWindow::OwnServerButtonClicked()
{
    m_own_server_clicked    = true;
    m_gmail_server_clicked  = false;
}

void MainWindow::GmailServerButtonClicked()
{
    m_gmail_server_clicked  = true;
    m_own_server_clicked    = false;
}

bool MainWindow::InitializeSMTPClient()
{
    try
    {
        SMTPClientClass client;

        client.set_smtp_address(this->get_server_choice());
        client.set_port(this->get_own_server_port());
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
        return m_own_server_address;
}

std::string MainWindow::get_own_server_port() const
{
    return m_own_server_port;
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