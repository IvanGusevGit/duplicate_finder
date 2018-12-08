#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>
#include <memory>
#include <vector>
#include <unordered_map>

#include <process.h>
#include <synchronized_scanner.h>

namespace Ui {
class main_widget;
}

class main_widget : public QWidget
{
    Q_OBJECT

public:
    explicit main_widget(QWidget *parent = 0);
    ~main_widget();

private slots:
    void select_directory();

    void add_directory();

    void remove_directory();

    void validate_start();

    void show_result(std::vector<std::vector<QString>> const & groups);

    void stop_search_and_close();

    void stop_and_reload();

    void catch_file_error(QString file);

private:

    void show_message(QString message);

    void start_search();

    std::vector<QString>* scan_directories();

    std::vector<QString> get_roots();

    void calc_hashes(std::vector<QString>& files);

    void stop_search();

    void clear_workspace();


    std::unique_ptr<Ui::main_widget> ui;
    std::unique_ptr<process> process_widget;
    synchronized_scanner* scanner;
    std::vector<QString> errors;


};


#endif // MAIN_WIDGET_H
