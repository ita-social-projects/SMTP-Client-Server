#pragma once

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../CPPLogger/CPPLogger.h"
#include "../XMLParser/XMLParser.h"

constexpr auto GMAIL_SERVER_DOMAIN = "smtp.gmail.com";

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    std::string GetServerChoice() const;
    std::string GetServerPort() const;
    std::string GetLogin() const;
    std::string GetPassword() const;
    std::string GetRecepient() const;
    std::string GetSubject() const;
    std::string GetMsgData() const;    

private:

    bool InitializeSMTPClient();
    bool InitializeSecureSMTPClient();

private slots:

    void SendButtonClicked();

    void OwnServerButtonClicked();

    void GmailServerButtonClicked();

    void ExitButtonClicked();

private:
    Ui::MainWindow* ui;
    std::string     m_server_choice;
    std::string     m_login;
    std::string     m_password;
    std::string     m_rcpt_to;
    std::string     m_subject;
    std::string     m_msg_data;
    std::string     m_own_server_port;
    std::string     m_own_server_address;
    bool            m_gmail_server_clicked  = false;
    bool            m_own_server_clicked    = true;
    XMLParser       m_xml;
    Logger          LOG;
};
#endif // MAINWINDOW_H
