// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "LogUi.h"
#include "qfiledialog.h"
#include "qtextbrowser.h"
#include <fstream>

#include "..\Crypto\SymmetricCrypto.h"

const short TOTAL_COLUMNS = 6;
const short LEVEL_COLUMN = 2;
const char* FILE_FILTER = "Text Files(*.txt)";
const char* FILE_SEARCH_WINDOW_NAME = "Open File";


LogUi::LogUi(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::LogUiClass)
{
    ui->setupUi(this);

    connect(ui->action_open_file, SIGNAL(triggered()), this, SLOT(OpenFile()));
    connect(ui->line_edit, SIGNAL(textEdited(const QString&)), this, SLOT(Search(const QString&)));
    connect(ui->action_trace, SIGNAL(triggered()), this, SLOT(Filter()));
    connect(ui->action_debug, SIGNAL(triggered()), this, SLOT(Filter()));
    connect(ui->action_info, SIGNAL(triggered()), this, SLOT(Filter()));
    connect(ui->action_warning, SIGNAL(triggered()), this, SLOT(Filter()));
    connect(ui->action_error, SIGNAL(triggered()), this, SLOT(Filter()));
    connect(ui->action_fatal, SIGNAL(triggered()), this, SLOT(Filter()));

    //Setup table
    ui->table_widget->setColumnCount(TOTAL_COLUMNS);
    ui->table_widget->setHorizontalHeaderLabels(QStringList{ "Thread ID","Full date", "Level", "File", "Function", "Message" });
    ui->table_widget->setSortingEnabled(true);
    ui->table_widget->setTextElideMode(Qt::ElideNone);
    ui->table_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->table_widget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
}

LogUi::~LogUi()
{
    delete ui;
}

void LogUi::GetContent(std::ifstream& stream)
{
    std::vector<unsigned char> vec, dec_vec;
    SymmetricCrypto m_crypto;
    QString dec_line;

    char ch = '\0';
    while (stream)
    {
        stream.get(ch);
        if (stream) vec.push_back(ch);
    }

    auto vec_size = m_crypto.Decrypt(vec, dec_vec);
    dec_vec.shrink_to_fit();

    auto all_rows = 0;
    for (auto& el : dec_vec)
    {
        if (el == '\n') all_rows++;
        dec_line.append((char)el);
    }

    QList<QString> dec_strings = dec_line.split('\n');

    for (int row = 0; row < all_rows; row++)
    {
        QString string = dec_strings.front();
        QList<QString> tokens = string.split('-');

        ui->table_widget->insertRow(row);
        for (int column = 0; column < TOTAL_COLUMNS; column++)
        {
            ui->table_widget->setItem(row, column, new QTableWidgetItem(tokens.front()));
            tokens.pop_front();
        }
        dec_strings.pop_front();

        ChangeRowColor();
    }
}

void LogUi::ChangeRowColor()
{
    int total_rows{ ui->table_widget->rowCount() };
    for (int row = 0; row < total_rows; row++)
    {
        if (ui->table_widget->item(row, LEVEL_COLUMN)->text() == "TRACE")
            for (int column = 0; column < TOTAL_COLUMNS; column++)
                ui->table_widget->item(row, column)->setBackground(QColor(230, 230, 230));
        else if (ui->table_widget->item(row, LEVEL_COLUMN)->text() == "DEBUG")
            for (int column = 0; column < TOTAL_COLUMNS; column++)
                ui->table_widget->item(row, column)->setBackground(QColor(159, 223, 191));
        else if (ui->table_widget->item(row, LEVEL_COLUMN)->text() == "INFO ")
            for (int column = 0; column < TOTAL_COLUMNS; column++)
                ui->table_widget->item(row, column)->setBackground(QColor(128, 191, 255));
        else if (ui->table_widget->item(row, LEVEL_COLUMN)->text() == "WARN.")
            for (int column = 0; column < TOTAL_COLUMNS; column++)
                ui->table_widget->item(row, column)->setBackground(QColor(255, 179, 179));
        else if (ui->table_widget->item(row, LEVEL_COLUMN)->text() == "ERROR")
            for (int column = 0; column < TOTAL_COLUMNS; column++)
                ui->table_widget->item(row, column)->setBackground(QColor(255, 77, 77));
        else
            for (int column = 0; column < TOTAL_COLUMNS; column++)
                ui->table_widget->item(row, column)->setBackground(QColor(255, 0, 0));
    }
}

void LogUi::OpenFile()
{
    std::ifstream read_file(QFileDialog::getOpenFileName(this, tr(FILE_SEARCH_WINDOW_NAME), "", tr(FILE_FILTER)).toStdString().c_str(), std::ios::binary);

    if (read_file)
    {
        GetContent(read_file);
        read_file.close();
    }
}

void LogUi::Search(const QString&)
{
    QString search_text = ui->line_edit->text();
    uint total_rows = ui->table_widget->rowCount();

    auto search_res = ui->table_widget->findItems(search_text, Qt::MatchFlags::enum_type::MatchContains);

    for (uint row = 0; row < total_rows; row++)
        ui->table_widget->hideRow(row);

    for (auto& el : search_res)
        ui->table_widget->showRow(el->row());
}

void LogUi::Filter()
{
    uint total_rows = ui->table_widget->rowCount();
    QList<QTableWidgetItem*> search_res;

    if (ui->action_trace->isChecked() || ui->action_debug->isChecked() || ui->action_info->isChecked() ||
        ui->action_warning->isChecked() || ui->action_error->isChecked() || ui->action_fatal->isChecked())
    {
        for (uint row = 0; row < total_rows; row++)
            ui->table_widget->hideRow(row);
    }
    else
    {
        for (uint row = 0; row < total_rows; row++)
            ui->table_widget->showRow(row);
        return;
    }

    if (ui->action_trace->isChecked())
    {
        search_res += ui->table_widget->findItems("TRACE", Qt::MatchFlags::enum_type::MatchExactly);
    }
    if (ui->action_debug->isChecked())
    {
        search_res += ui->table_widget->findItems("DEBUG", Qt::MatchFlags::enum_type::MatchExactly);
    }
    if (ui->action_info->isChecked())
    {
        search_res += ui->table_widget->findItems("INFO ", Qt::MatchFlags::enum_type::MatchExactly);
    }
    if (ui->action_warning->isChecked())
    {
        search_res += ui->table_widget->findItems("WARN.", Qt::MatchFlags::enum_type::MatchExactly);
    }
    if (ui->action_error->isChecked())
    {
        search_res += ui->table_widget->findItems("ERROR", Qt::MatchFlags::enum_type::MatchExactly);
    }
    if (ui->action_fatal->isChecked())
    {
        search_res += ui->table_widget->findItems("PANIC", Qt::MatchFlags::enum_type::MatchExactly);
    }

    for (auto& el : search_res)
        ui->table_widget->showRow(el->row());
}