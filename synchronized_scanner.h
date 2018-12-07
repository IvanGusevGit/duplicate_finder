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

    void stop() {
        stop_flag = true;
    }

    void emit_hashed_signal(qint64 size);

signals:
    void finished_search(std::vector<std::vector<QString>> const &);
    void files_number(quint64);
    void hashed_file(qint64);

protected:
    virtual void run();

private:

    std::vector<QString> scan_directories();

    void calc_hashes(std::vector<QString> const &files);

    std::vector<std::vector<QString>> filter_results();

    static void calc_file_hash(synchronized_scanner* scanner, std::pair<QByteArray, QString>* const result);

private:

    static size_t const BUFFER_SIZE = 10240;
    static size_t const THREADS_NUMBER = 2;

    std::vector<QString> directories;
    std::unordered_map<QByteArray, std::unique_ptr<std::vector<QString>>> hashes;

    std::thread threads[THREADS_NUMBER];

    std::pair<QByteArray, QString> results[THREADS_NUMBER];

    std::atomic_bool stop_flag;

};

#endif // SYNCHRONIZED_SCANNER_H
