#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <ctime>
#include <thread>
#include <mutex>
#include <iomanip>
#include <mutex>

enum class LogType {INFO, WARNING, ERROR, USER, SYSTEM};

class Logger{
    private:
    std::mutex m_mutex;
    std::vector<std::unique_ptr<std::ostream>> output_streams_;
    std::string getLogTypeString(LogType level) const {
        switch (level) {
            case LogType::INFO:
                return "INFO";
            case LogType::WARNING:
                return "WARNING";
            case LogType::ERROR:
                return "ERROR";
            case LogType::USER:
                return "USER";
            case LogType::SYSTEM:
                return "SYSTEM";
        }
        return "UNKNOWN";
    }
    public:
    void addOutput(std::ostream& output_stream) {
        output_streams_.push_back(std::make_unique<std::ostream>(output_stream.rdbuf()));
    }
    void removeOutput(std::ostream& output_stream) {
        std::vector<std::unique_ptr<std::ostream>> new_output_streams;
        for (const auto& stream : output_streams_) {
            if (stream.get() != &output_stream) {
                new_output_streams.push_back(std::make_unique<std::ostream>(stream->rdbuf()));
            }
        }
        output_streams_ = std::move(new_output_streams);
    }
    void log(LogType level, const std::string& message) {
        std::lock_guard<std::mutex> locker(m_mutex);
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&t);
        
        // Obtener los milisegundos
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        // Formatear el timestamp como "YYYY-MM-dd HH:mm:ss.SSS"
        std::ostringstream timestamp;
        timestamp << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
        
        std::string log_entry = "[" + getLogTypeString(level) + " "
                                + timestamp.str() + "] "
                                + message + "\n";

        for (const auto& output_stream : output_streams_) {
            (*output_stream) << log_entry;
        }
    }
};