#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

protected:
    void closeEvent(QCloseEvent* event);

signals:
    void windowClosed();

private slots:
    
    void SaveButtonClicked();
    void CloseButtonClicked();

private:
    Ui::SettingsWindow *m_ui;
};

#endif // SETTINGSWINDOW_H
