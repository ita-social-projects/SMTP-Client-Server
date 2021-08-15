// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "qfiledialog.h"
#include "qtextbrowser.h"
#include <fstream>
#include "LogUi.h"

#include "..\Crypto\SymmetricCrypto.h"

const short TOTAL_COLUMNS			= 6;
const short LEVEL_COLUMN			= 2;
const char* FILE_FILTER				= "Text Files(*.txt)";
const char* FILE_SEARCH_WINDOW_NAME	= "Open File";
const char	ROW_SEPARATOR			= '\n';
const char	TOKEN_SEPARATOR			= '-';


LogUi::LogUi(QWidget* parent)
	: QMainWindow(parent), m_ui(new Ui::LogUiClass)
{
	m_ui->setupUi(this);

	connect(m_ui->action_open_file, SIGNAL(triggered()), this, SLOT(OpenFile()));
	connect(m_ui->line_edit, SIGNAL(textEdited(const QString&)), this, SLOT(Search(const QString&)));
	connect(m_ui->action_trace, SIGNAL(triggered()), this, SLOT(Filter()));
	connect(m_ui->action_debug, SIGNAL(triggered()), this, SLOT(Filter()));
	connect(m_ui->action_info, SIGNAL(triggered()), this, SLOT(Filter()));
	connect(m_ui->action_warning, SIGNAL(triggered()), this, SLOT(Filter()));
	connect(m_ui->action_error, SIGNAL(triggered()), this, SLOT(Filter()));
	connect(m_ui->action_fatal, SIGNAL(triggered()), this, SLOT(Filter()));

	//Setup table
	m_ui->table_widget->setColumnCount(TOTAL_COLUMNS);
	m_ui->table_widget->setHorizontalHeaderLabels(QStringList{ "Thread ID", "Full date", "Level", "File", "Function", "Message" });
	m_ui->table_widget->setSortingEnabled(true);
	m_ui->table_widget->setTextElideMode(Qt::ElideNone);
	m_ui->table_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_ui->table_widget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
}

LogUi::~LogUi()
{
	delete m_ui;
}

void LogUi::GetContent(std::ifstream& stream)
{
	std::vector<unsigned char> vec, dec_vec;
	SymmetricCrypto m_crypto;
	QString dec_line;

	char ch;
	while (stream)
	{
		stream.get(ch);
		if (stream) vec.push_back(ch);
	}

	auto vec_size{ m_crypto.Decrypt(vec, dec_vec) };
	dec_vec.shrink_to_fit();

	auto all_rows{ 0 };
	for (auto& el : dec_vec)
	{
		if (el == ROW_SEPARATOR) all_rows++;
		dec_line.append((char)el);
	}

	QList<QString> dec_strings = dec_line.split(ROW_SEPARATOR);

	for (int row = 0; row < all_rows; row++)
	{
		QString string = dec_strings.front();
		QList<QString> tokens = string.split(TOKEN_SEPARATOR);

		m_ui->table_widget->insertRow(row);
		for (int column = 0; column < TOTAL_COLUMNS; column++)
		{
			m_ui->table_widget->setItem(row, column, new QTableWidgetItem(tokens.front()));
			tokens.pop_front();
		}
		dec_strings.pop_front();

		ChangeRowColor();
	}
}

void LogUi::ChangeRowColor()
{
	int total_rows{ m_ui->table_widget->rowCount() };
	for (int row = 0; row < total_rows; row++)
	{
		if (m_ui->table_widget->item(row, LEVEL_COLUMN)->text() == "TRACE")
			for (int column = 0; column < TOTAL_COLUMNS; column++)
				m_ui->table_widget->item(row, column)->setBackground(QColor(230, 230, 230));
		else if (m_ui->table_widget->item(row, LEVEL_COLUMN)->text() == "DEBUG")
			for (int column = 0; column < TOTAL_COLUMNS; column++)
				m_ui->table_widget->item(row, column)->setBackground(QColor(159, 223, 191));
		else if (m_ui->table_widget->item(row, LEVEL_COLUMN)->text() == "INFO ")
			for (int column = 0; column < TOTAL_COLUMNS; column++)
				m_ui->table_widget->item(row, column)->setBackground(QColor(128, 191, 255));
		else if (m_ui->table_widget->item(row, LEVEL_COLUMN)->text() == "WARN.")
			for (int column = 0; column < TOTAL_COLUMNS; column++)
				m_ui->table_widget->item(row, column)->setBackground(QColor(255, 179, 179));
		else if (m_ui->table_widget->item(row, LEVEL_COLUMN)->text() == "ERROR")
			for (int column = 0; column < TOTAL_COLUMNS; column++)
				m_ui->table_widget->item(row, column)->setBackground(QColor(255, 77, 77));
		else
			for (int column = 0; column < TOTAL_COLUMNS; column++)
				m_ui->table_widget->item(row, column)->setBackground(QColor(255, 0, 0));
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
	QString search_text = m_ui->line_edit->text();
	uint total_rows = m_ui->table_widget->rowCount();

	auto search_res = m_ui->table_widget->findItems(search_text, Qt::MatchFlags::enum_type::MatchContains);

	for (uint row = 0; row < total_rows; row++)
		m_ui->table_widget->hideRow(row);

	for (auto& el : search_res)
		m_ui->table_widget->showRow(el->row());
}

void LogUi::Filter()
{
	uint total_rows = m_ui->table_widget->rowCount();
	QList<QTableWidgetItem*> search_res;

	if (m_ui->action_trace->isChecked() || m_ui->action_debug->isChecked() || m_ui->action_info->isChecked() ||
		m_ui->action_warning->isChecked() || m_ui->action_error->isChecked() || m_ui->action_fatal->isChecked())
	{
		for (uint row = 0; row < total_rows; row++)
			m_ui->table_widget->hideRow(row);
	}
	else
	{
		for (uint row = 0; row < total_rows; row++)
			m_ui->table_widget->showRow(row);
		return;
	}

	if (m_ui->action_trace->isChecked())
	{
		search_res += m_ui->table_widget->findItems("TRACE", Qt::MatchFlags::enum_type::MatchExactly);
	}
	if (m_ui->action_debug->isChecked())
	{
		search_res += m_ui->table_widget->findItems("DEBUG", Qt::MatchFlags::enum_type::MatchExactly);
	}
	if (m_ui->action_info->isChecked())
	{
		search_res += m_ui->table_widget->findItems("INFO ", Qt::MatchFlags::enum_type::MatchExactly);
	}
	if (m_ui->action_warning->isChecked())
	{
		search_res += m_ui->table_widget->findItems("WARN.", Qt::MatchFlags::enum_type::MatchExactly);
	}
	if (m_ui->action_error->isChecked())
	{
		search_res += m_ui->table_widget->findItems("ERROR", Qt::MatchFlags::enum_type::MatchExactly);
	}
	if (m_ui->action_fatal->isChecked())
	{
		search_res += m_ui->table_widget->findItems("PANIC", Qt::MatchFlags::enum_type::MatchExactly);
	}

	for (auto& el : search_res)
		m_ui->table_widget->showRow(el->row());
}