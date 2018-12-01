#ifndef PROCESS_H
#define PROCESS_H

#include <QWidget>
#include <memory>

namespace Ui {
class process;
}

class process : public QWidget
{
    Q_OBJECT

public:
    explicit process(QWidget *parent = 0);
    ~process();

    void add_group(qint64 id, std::vector<QString> const &files);

    void add_errrors(std::vector<QString> const &errors);

    void set_process_bar_visible(bool visible);

    void set_errors_visible(bool visible);

private slots:

    void delete_selected_files();

    void set_limit(quint64 limit);

    void increase_status(size_t t);

private:
    std::unique_ptr<Ui::process> ui;

};

#endif // PROCESS_H
