#ifndef PROCESS_H
#define PROCESS_H

#include <QWidget>
#include <memory>
#include <synchronized_scanner.h>

namespace Ui {
class process;
}

class process : public QWidget
{
    Q_OBJECT

public:
    explicit process();
    ~process();

    void add_group(qint64 id, std::vector<QString> const &files);

    void add_errrors(std::vector<QString> const &errors);

    void set_process_bar_visible(bool visible);

    void set_errors_visible(bool visible);

    void set_file_deletetion_enabled(bool enabled);

    void closeEvent(QCloseEvent *event);

signals:

    void closure();

    void return_to_main();

private slots:

    void delete_selected_files();

    void return_to_main_menu();

    void set_limit(quint64 limit);

    void increase_status(qint64 t);

private:
    std::unique_ptr<Ui::process> ui;
    qint64 total;
    qint64 current;
};

#endif // PROCESS_H
