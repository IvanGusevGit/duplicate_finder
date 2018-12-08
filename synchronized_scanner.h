#ifndef SYNCHRONIZED_SCANNER_H
#define SYNCHRONIZED_SCANNER_H

#include <QThread>
#include <memory>
#include <vector>
#include <unordered_map>
#include <vector>
#include <thread>

namespace std {
    template<>
    struct hash<QByteArray> {
        std::size_t operator()(const QByteArray &s) const {
            return qHash(s);
        }
    };
}

class synchronized_scanner : public QThread
{
Q_OBJECT
public:
    explicit synchronized_scanner(std::vector<QString> roots);
    ~synchronized_scanner() = default;

    void stop();

    void emit_hashed_signal(qint64 size);

    void emit_file_error(QString file);

signals:
    void finished_search(std::vector<std::vector<QString>> const &);
    void files_number(quint64);
    void hashed_file(qint64);
    void file_error(QString);

protected:
    virtual void run();

private:

    std::vector<QString> scan_directories();

    std::vector<std::vector<QString>> filter_results();

    void calc_hashes(std::vector<QString> const &files);

    void collect_results(size_t threads_numbers);

    static void calc_file_hash(synchronized_scanner* scanner, std::pair<QByteArray, QString>* const result, bool* stop_flag);

private:

    static size_t const BUFFER_SIZE = 50120;
    static size_t const THREADS_NUMBER = 3;

    std::vector<QString> directories;
    std::unordered_map<QByteArray, std::unique_ptr<std::vector<QString>>> hashes;

    std::thread threads[THREADS_NUMBER];

    std::pair<QByteArray, QString> results[THREADS_NUMBER];

    bool stop_flag;

};

#endif // SYNCHRONIZED_SCANNER_H
