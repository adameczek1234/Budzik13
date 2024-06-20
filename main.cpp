#include <chrono>
#include <exception>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <ctime>
#include <vector>
#include <algorithm>
#include <atomic>

class WeekDays {
public:
    bool monday = 0;
    bool tuesday = 0;
    bool wednesday = 0;
    bool thursday = 0;
    bool friday = 0;
    bool saturday = 0;
    bool sunday = 0;

    void SetWeekend() {
        saturday = true;
        sunday = true;
    }
    void SetWorkdays() {
        monday = true;
        tuesday = true;
        wednesday = true;
        thursday = true;
        friday = true;
    }
    void UnSetWeekend() {
        saturday = false;
        sunday = false;
    }
    void UnSetWorkdays() {
        monday = false;
        tuesday = false;
        wednesday = false;
        thursday = false;
        friday = false;
    }
};

class Budzik {
public:
    void Set();
    void Unset();
    void ShowActualFormat();
private:
    WeekDays active_days;
    int hours = 0;
    int minutes = 0;
    std::thread* t = nullptr;
    std::atomic<bool> running{false};

    void Waiting();
    std::vector<int> GetActiveDays();
};

void Budzik::Unset() {
    running = false;
    if (t && t->joinable()) {
        t->join();
    }
    delete t;
    t = nullptr;
    std::cout << "Budzik wyłączony." << std::endl;
}

void Budzik::Waiting() {
    auto active_days = GetActiveDays();
    std::sort(active_days.begin(), active_days.end());
    running = true;

    t = new std::thread([this, active_days]() {
        while (running) {
            std::time_t now = std::time(0);
            std::tm *ltm = std::localtime(&now);

            int current_day = ltm->tm_wday;
            int current_hour = ltm->tm_hour;
            int current_minute = ltm->tm_min;

            for (auto day : active_days) {
                if (!running) break;

                if ((day == current_day && current_hour < this->hours) ||
                    (day == current_day && current_hour == this->hours && current_minute < this->minutes) ||
                    day > current_day) {

                    std::tm alarm_time = *ltm;
                    alarm_time.tm_wday = day;
                    alarm_time.tm_hour = this->hours;
                    alarm_time.tm_min = this->minutes;
                    alarm_time.tm_sec = 0;

                    std::time_t alarm_epoch = std::mktime(&alarm_time);
                    if (alarm_epoch < now) {
                        alarm_epoch += 7 * 24 * 60 * 60; // add one week
                    }

                    std::this_thread::sleep_until(std::chrono::system_clock::from_time_t(alarm_epoch));
                    if (running) {
                        std::cout << "dzwoni" << std::endl;
                    }
                    break;
                }
            }
        }
    });
    t->detach();
}

std::vector<int> Budzik::GetActiveDays() {
    std::vector<int> days;
    if (active_days.monday) days.push_back(1);
    if (active_days.tuesday) days.push_back(2);
    if (active_days.wednesday) days.push_back(3);
    if (active_days.thursday) days.push_back(4);
    if (active_days.friday) days.push_back(5);
    if (active_days.saturday) days.push_back(6);
    if (active_days.sunday) days.push_back(0);
    return days;
}

void Budzik::Set() {
    while (true) {
        ShowActualFormat();
        std::cout
                << "================================================\n"
                << "Set days as f 0 0 0 0 0 0 0\n"
                << "or write w for weekend, nw for no weekend\n"
                << "and c for common days, nc for not commond days\n"
                << "and t for time 12:12\n"
                << "save for continue or exit for return\n"
                << "================================================\n";

        std::string format; std::cin >> format;
        if (format == "w") {
            active_days.SetWeekend();
            continue;
        }
        if (format == "nw") {
            active_days.UnSetWeekend();
            continue;
        }
        if (format == "c") {
            active_days.SetWorkdays();
            continue;
        }
        if (format == "nc") {
            active_days.UnSetWorkdays();
            continue;
        }
        if (format == "f") {
            try {
                std::cin >> active_days.monday;
                std::cin >> active_days.tuesday;
                std::cin >> active_days.wednesday;
                std::cin >> active_days.thursday;
                std::cin >> active_days.friday; // Poprawka: dodano piątek
                std::cin >> active_days.saturday;
                std::cin >> active_days.sunday;
            } catch (std::exception) {
                std::cout << "Bad format!\n";
                continue;
            }
        }
        if (format == "t") {
            try {
                std::string time;
                std::getline(std::cin, time, ':');
                hours = std::stoi(time);
                std::cin >> time;
                minutes = std::stoi(time);
            } catch (std::exception) {
                std::cout << "Bad format!\n";
                continue;
            }
        }
        if (format == "save") break;
        if (format == "exit") return;
    }
    Waiting();
}

void Budzik::ShowActualFormat() {
    std::cout <<
              "Pn: " << active_days.monday << ", " <<
              "Wt: " << active_days.tuesday << ", " <<
              "Sr: " << active_days.wednesday << ", " <<
              "Cz: " << active_days.thursday << ", " <<
              "Pt: " << active_days.friday << ", " <<
              "Sb: " << active_days.saturday << ", " <<
              "Nd: " << active_days.sunday << std::endl <<
              "time: " << hours << ':' << minutes << std::endl;
}

void Menu() {
    std::cout <<
              "===================\n" <<
              "1. Ustaw budzik\n" <<
              "2. Wylacz budzik\n" <<
              "===================\n";
}

int main() {
    auto budzik = new Budzik();
    while (true) {
        int choice = 0;
        Menu();
        try {
            std::cin >> choice;
        } catch (std::exception) {
            std::cout << "Niepoprawna opcja!\n";
            continue;
        }

        switch (choice) {
            case 1:
                budzik->Set();
                break;
            case 2:
                budzik->Unset();
                break;
            default:
                std::cout << "Niepoprawna opcja!\n";
        }
    }
    delete budzik;
}