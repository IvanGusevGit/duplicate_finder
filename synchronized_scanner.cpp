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
}

void synchronized_scanner::run() {
    std::vector<QString> files = scan_directories();
    calc_hashes(files);
    std::vector<std::vector<QString>> filtered_files = filter_results();

    emit finished_search(filtered_files);
}


std::vector<QString> synchronized_scanner::scan_directories() {

    std::unordered_map<qint64, std::unique_ptr<std::vector<QString>>> files;
    quint64 files_counter = 0;
    for (QString current_directory_path : directories) {
        QDirIterator it(current_directory_path, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            files_counter++;
            QString file_path = it.next();
            QFileInfo file_parameters(file_path);
            auto files_it = files.find(file_parameters.size());
            if (files_it == files.end()) {
                files[file_parameters.size()] = std::make_unique<std::vector<QString>>();
            }
            (*files[file_parameters.size()]).push_back(file_path);
        }
    }

    std::vector<QString> filtered_files;
    emit files_number(files_counter);
    size_t skipped = 0;
    for (auto it = files.begin(); it != files.end(); it++) {
        if (it->second->size() > 1) {
            for (QString file_path : *(it->second)) {
                filtered_files.push_back(file_path);
            }
        } else {
            skipped++;
        }
    }
    emit hashed_file(skipped);

    return filtered_files;
}

void synchronized_scanner::calc_hashes(std::vector<QString> const  &files) {
    size_t next_thread = 0;
    for (QString file_path : files) {
        if (next_thread == THREADS_NUMBER) {
            for (size_t i = 0; i < THREADS_NUMBER; ++i) {
                threads[i].join();
                if (hashes.find(results[i].first) == hashes.end()) {
                    hashes[results[i].first] = std::make_unique<std::vector<QString>>();
                }
                (*hashes[results[i].first]).push_back(results[i].second);
            }
            emit hashed_file(THREADS_NUMBER);
            next_thread = 0;
        }
        results[next_thread].second = file_path;
        threads[next_thread] = std::thread(calc_file_hash, &results[next_thread]);
        next_thread++;
    }
    for (size_t i = 0; i < next_thread; ++i) {
        threads[i].join();
        if (hashes.find(results[i].first) == hashes.end()) {
            hashes[results[i].first] = std::make_unique<std::vector<QString>>();
        }
        (*hashes[results[i].first]).push_back(results[i].second);
    }
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

void synchronized_scanner::calc_file_hash(std::pair<QByteArray, QString>* result) {
    QFile file(result->second);
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (file.open(QFile::ReadOnly)) {
        hash.addData(&file);
    }
    result->first = hash.result().toHex();
}
