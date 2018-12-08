#include "process.h"
#include "ui_process.h"

#include <memory>
#include <QTreeWidgetItem>
#include <QFile>
#include <QStyle>
#include <QDesktopWidget>
#include <iostream>

process::process() :
    QWidget(0),
    ui(new Ui::process)
{
    ui->setupUi(this);

    ui->progress_bar->setRange(0, 100);
    ui->progress_bar->setValue(0);

    ui->result_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->result_tree->setUniformRowHeights(true);
    ui->result_tree->header()->resizeSection(0, 1000);

    ui->errors_tree->setVisible(false);

    ui->delete_selected_button->setEnabled(false);

    current = total = 0;

    this->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            this->size(),
            qApp->desktop()->availableGeometry()
        )
    );

    connect(ui->delete_selected_button, SIGNAL(clicked(bool)), this, SLOT(delete_selected_files()));
    connect(ui->return_button, SIGNAL(clicked(bool)), this, SLOT(return_to_main_menu()));
}

process::~process(){}

void process::add_group(qint64 id, std::vector<QString> const &files) {
    std::unique_ptr<QTreeWidgetItem> header_item = std::make_unique<QTreeWidgetItem>(ui->result_tree);
    header_item->setExpanded(true);
    header_item->setText(0, QString("Group ") + QString::number(id) + " [" + QString::number(files.size()) + ']');
    for (QString current_path : files) {
        std::unique_ptr<QTreeWidgetItem> current_item = std::make_unique<QTreeWidgetItem>(header_item.get());
        current_item->setText(0, current_path);
        ui->result_tree->addTopLevelItem(current_item.release());
    }
    header_item.release();
}

void process::set_process_bar_visible(bool visible) {
    ui->progress_bar->setVisible(visible);
}

void process::delete_selected_files()
{
    QList<QTreeWidgetItem*> selected_items = ui->result_tree->selectedItems();
    for (QTreeWidgetItem* item : selected_items) {
        QFile file(item->text(0));
        if (file.exists() && file.remove()) {
            QFont font;
            font.setStrikeOut(true);
            item->setFont(0, font);
            item->setTextColor(0, QColor(255, 0, 0));
        }
    }
}

void process::set_limit(quint64 limit) {
    total = limit;
    if (limit == 0) {
        ui->progress_bar->setRange(0, 0);
    } else {
        ui->progress_bar->setRange(0, 100);
    }
}

void process::increase_status(qint64 t) {
    current += t;
    if (total != 0) {
        ui->progress_bar->setValue(current * 100 / total);
    }
}

void process::add_errrors(std::vector<QString> const &errors) {
    for (QString error : errors) {
        std::unique_ptr<QTreeWidgetItem> header_item = std::make_unique<QTreeWidgetItem>(ui->errors_tree);
        header_item->setExpanded(true);
        header_item->setText(0, error);
        header_item.release();
    }
}

void process::set_errors_visible(bool visible) {
    ui->errors_tree->setVisible(visible);
}

void process::closeEvent(QCloseEvent *event) {
    emit closure();
}

void process::set_file_deletetion_enabled(bool enabled) {
    ui->delete_selected_button->setEnabled(enabled);
}

void process::return_to_main_menu() {
    emit return_to_main();
}
