#include "process.h"
#include "ui_process.h"
#include <memory>
#include <QTreeWidgetItem>
#include <QFile>
#include <QStyle>
#include <QDesktopWidget>

process::process(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::process)
{
    ui->setupUi(this);
    ui->progress_bar->setRange(0, 100);
    ui->progress_bar->setValue(0);

    ui->result_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->result_tree->setUniformRowHeights(true);
    ui->result_tree->header()->resizeSection(0, 1000);

    ui->errors_tree->setVisible(false);

    this->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            this->size(),
            qApp->desktop()->availableGeometry()
        )
    );

    connect(ui->delete_selected_button, SIGNAL(clicked(bool)), this, SLOT(delete_selected_files()));

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
   ui->progress_bar->setRange(0, limit);
}

void process::increase_status(size_t t) {

    ui->progress_bar->setValue(ui->progress_bar->value() + t);
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

