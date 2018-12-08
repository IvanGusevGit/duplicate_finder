#include "synchronized_scanner.h"

#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QDirIterator>
#include <vector>
#include <iostream>

synchronized_scanner::synchronized_scanner(std::vector<QString> roots): directories(roots){
    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<std::vector<std::vector<QString>>>("std::vector<std::vector<QString>>");
    stop_flag = false;
}

void synchronized_scanner::run() {
    std::vector<QString> files = scan_directories();
    calc_hashes(files);
    std::vector<std::vector<QString>> filtered_files = filter_results();

    emit finished_search(filtered_files);
}


std::vector<QString> synchronized_scanner::scan_directories() {

    std::unordered_map<qint64, std::unique_ptr<std::vector<QString>>> files;
    quint64 size_counter = 0;
    for (QString current_directory_path : directories) {
        QDirIterator it(current_directory_path, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString file_path = it.next();
            QFileInfo file_parameters(file_path);
            size_counter += file_parameters.size();
            auto files_it = files.find(file_parameters.size());
            if (files_it == files.end()) {
                files[file_parameters.size()] = std::make_unique<std::vector<QString>>();
            }
            (*files[file_parameters.size()]).push_back(file_path);
        }
    }

    std::vector<QString> filtered_files;
    emit files_number(size_counter);
    qint64 skipped = 0;
    for (auto it = files.begin(); it != files.end(); it++) {
        if (it->second->size() > 1) {
            for (QString file_path : *(it->second)) {
                filtered_files.push_back(file_path);
            }
        } else {
            QFileInfo file_parameters(it->second->at(0));
            skipped += file_parameters.size();
        }
    }
    emit hashed_file(skipped);

    return filtered_files;
}

void synchronized_scanner::calc_hashes(std::vector<QString> const  &files) {
    size_t next_thread = 0;
    for (QString file_path : files) {
        if (stop_flag) {
            break;
        }
        if (next_thread == THREADS_NUMBER) {
            collect_results(THREADS_NUMBER);
            next_thread = 0;
        }
        results[next_thread].second = file_path;
        threads[next_thread] = std::thread(calc_file_hash, this, &results[next_thread], &stop_flag);
        next_thread++;
    }
    collect_results(next_thread);
}

std::vector<std::vector<QString>> synchronized_scanner::filter_results() {
    std::vector<std::vector<QString>> results;
    for (auto it = hashes.begin(); it != hashes.end(); it++) {
        if (it->second->size() > 1) {
            results.push_back(*(it->second));
        }
    }
    return results;
}

void synchronized_scanner::calc_file_hash(synchronized_scanner* scanner, std::pair<QByteArray, QString>* result, bool* stop_flag) {
    QFile file(result->second);
    QFileInfo parameters(result->second);
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (file.open(QFile::ReadOnly)) {
        while (!file.atEnd() && !(*stop_flag)) {
            QByteArray chunk = file.read(10240);
            hash.addData(chunk);
            scanner->emit_hashed_signal(10240);
        }
    } else {
        scanner->emit_file_error(result->second);
    }
    result->first = hash.result();
}

void synchronized_scanner::emit_hashed_signal(qint64 size) {
    emit hashed_file(size);
}

void synchronized_scanner::emit_file_error(QString file) {
    emit file_error(file);
}

void synchronized_scanner::collect_results(size_t thread_number) {
    for (size_t i = 0; i < thread_number; ++i) {
        threads[i].join();
        if (hashes.find(results[i].first) == hashes.end()) {
            hashes[results[i].first] = std::make_unique<std::vector<QString>>();
        }
        (*hashes[results[i].first]).push_back(results[i].second);
    }
}

void synchronized_scanner::stop() {
    stop_flag = true;
}



