#include "main_widget.h"
#include "ui_main_widget.h"
#include <process.h>

#include <unordered_map>
#include <memory>
#include <iostream>

#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QCryptographicHash>
#include <thread>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QtAlgorithms>
#include <QStyle>
#include <QDesktopWidget>

main_widget::main_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::main_widget)
{
    ui->setupUi(this);
    ui->message_label->setVisible(false);
    ui->directories->setSelectionMode(QAbstractItemView::ExtendedSelection);

    this->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            this->size(),
            qApp->desktop()->availableGeometry()
        )
    );
    connect(ui->select_directory_button, SIGNAL(clicked(bool)), this, SLOT(select_directory()));
    connect(ui->remove_directory_button, SIGNAL(clicked(bool)), this, SLOT(remove_directory()));
    connect(ui->add_directory_button, SIGNAL(clicked(bool)), this, SLOT(add_directory()));
    connect(ui->start_button, SIGNAL(clicked(bool)), this, SLOT(validate_start()));
}

main_widget::~main_widget() {}

void main_widget::show_message(QString message) {
    ui->message_label->setText(message);
    ui->message_label->setVisible(true);
}

void main_widget::select_directory() {
    QString directory_path = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->path_edit->setText(directory_path);
}

void main_widget::add_directory() {
    QString path = ui->path_edit->text();
    if (!QDir(path).exists()) {
        show_message("Dirrectory does not exist");
    } else {
        bool passed = true;
        for (int32_t i = 0; i < ui->directories->count(); ++i) {
            QString element = ui->directories->item(i)->text();
            if (path.indexOf(element) == 0 || element.indexOf(path) == 0) {
                show_message("This folder intersects with other folders");
                passed = false;
                break;
            }
        }
        if (passed) {
            ui->directories->addItem(path);
            ui->message_label->setVisible(false);
        }
    }
}

std::vector<QString> main_widget::get_roots() {
    std::vector<QString> roots;
    for (int32_t i = 0; i < ui->directories->count(); ++i) {
        QString current_directory = ui->directories->item(i)->text();
        QDir directory(current_directory);
        if (!directory.exists() || !directory.isReadable()) {
            errors.push_back("Unable to access \"" + current_directory + "\" directory");
        } else {
            roots.push_back(current_directory);
        }
    }
    return roots;
}

void main_widget::remove_directory() {
    qDeleteAll(ui->directories->selectedItems());
}

void main_widget::validate_start()
{
    if (ui->directories->count() == 0) {
        show_message("No directories were selected");
    } else {
        this->hide();
        start_search();
    }
}

void main_widget::start_search() {
    process_widget = std::make_unique<process>();
    scanner = new synchronized_scanner(get_roots());
    connect(scanner, SIGNAL(finished_search(std::vector<std::vector<QString>> const &)), this, SLOT(show_result(std::vector<std::vector<QString>> const &)));
    connect(scanner, SIGNAL(files_number(quint64)), process_widget.get(), SLOT(set_limit(quint64)));
    connect(scanner, SIGNAL(hashed_file(qint64)), process_widget.get(), SLOT(increase_status(qint64)));
    connect(scanner, SIGNAL(file_error(QString)), this, SLOT(catch_file_error(QString)));
    connect(process_widget.get(), SIGNAL(closure()), this, SLOT(stop_search_and_close()));
    connect(process_widget.get(), SIGNAL(return_to_main()), this, SLOT(stop_and_reload()));
    scanner->start();
    process_widget->show();
}

void main_widget::show_result(std::vector<std::vector<QString>> const & groups) {
    qint64 group_number = 1;
    for (std::vector<QString> group : groups) {
        process_widget->add_group(group_number++, group);
    }
    if (errors.size() != 0) {
        process_widget->add_errrors(errors);
        process_widget->set_errors_visible(true);
    }
    process_widget->set_process_bar_visible(false);
    process_widget->set_file_deletetion_enabled(true);
}

void main_widget::stop_search() {
    scanner->stop();
    scanner->wait();
    scanner->deleteLater();
}

void main_widget::stop_search_and_close() {
    stop_search();
    close();
}

void main_widget::stop_and_reload() {
    stop_search();
    this->show();
    process_widget->hide();
    clear_workspace();
}

void main_widget::clear_workspace() {
    ui->directories->clear();
    errors.clear();
}

void main_widget::catch_file_error(QString file) {
    errors.push_back("Unable to access " + file);
}
