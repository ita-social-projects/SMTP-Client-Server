#pragma once

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMAinWindow>
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

    std::string get_server_choice() const;
    std::string get_server_port() const;
    std::string get_login() const;
    std::string get_password() const;
    std::string get_rcpt_to() const;
    std::string get_subject() const;
    std::string get_msg_data() const;    

private:

    bool InitializeSMTPClient();
    bool InitializeSecureSMTPClient();

private slots:

    void SendButtonClicked();
    void ServerButtonClicked();
    void GmailServerButtonClicked();
    void ExitButtonClicked();

private:
    Ui::MainWindow* m_ui;
    std::string     m_server_choice;
    std::string     m_login;
    std::string     m_password;
    std::string     m_rcpt_to;
    std::string     m_subject;
    std::string     m_msg_data;
    std::string     m_server_port;
    std::string     m_server_address;
    bool            m_gmail_server_clicked;
    bool            m_server_clicked;
    XMLParser       m_xml;
    Logger          LOG;
};
#endif // MAINWINDOW_H