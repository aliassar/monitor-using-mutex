#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <mutex>
#include <thread>
#include <cstdlib>
#include <semaphore.h>

using namespace std;

sem_t sumProtector;
long totalPollution=0;

template<class T>
class monitor {
public:
    template<typename ...Args>
    explicit monitor(Args &&... args) : m_cl(forward<Args>(args)...) {}

    struct monitor_helper {
        explicit monitor_helper(monitor *mon) : m_mon(mon), m_ul(mon->m_lock) {}

        T *operator->() { return &m_mon->m_cl; }

        monitor *m_mon;
        unique_lock<mutex> m_ul;
    };

    monitor_helper operator->() { return monitor_helper(this); }

    monitor_helper ManuallyLock() { return monitor_helper(this); }

    T &GetThreadUnsafeAccess() { return m_cl; }

private:
    T m_cl;
    mutex m_lock;
};


struct road {
    string startEnd;
    int p;

    road(string startEnd1, int p1)
            : startEnd(move(startEnd1)), p(p1) {
        cout << "Road constructed.\n";
    }

    road(road &&other)
            : startEnd(move(other.startEnd)), p(other.p) {
        cout << "I am being moved.\n";
    }

    long calculation() {
        srand(time(nullptr));
        int h = rand() % 10 + 1;
        long sum = 0;
        for (long k = 0; k < 10000000; k++) {
            sum += floor(k / (1000000 * p * h));
        }
        return sum;
    }
};

long calculate(const vector<monitor<road> *> &passedRoads, int carNumber,int pathNumber) {
    for (auto passedRoadMonitor:passedRoads) {
        time_t startTime = time(nullptr);
        monitor<road> &passedRoad = *passedRoadMonitor;
        long pollution=passedRoad->calculation();
        sem_wait(&sumProtector);
        totalPollution+=pollution;
        time_t endTime = time(nullptr);
        ofstream outfile (to_string(pathNumber)+"-"+to_string(carNumber));
        outfile << passedRoad->startEnd[0];
        outfile << ", "<<startTime<<", ";
        outfile <<passedRoad->startEnd[4]<<", ";
        outfile<<endTime<<", ";
        outfile<<pollution;
        outfile<<", "<<totalPollution<<"\n";
        outfile.close();
        sem_post(&sumProtector);
    }
}

struct path {
    vector<monitor<road> *> passedRoads;
    int carCount;

    path(vector<monitor<road> *> &passedRoads1, int carCount1)
            : passedRoads(passedRoads1), carCount(carCount1) {
        cout << "Path constructed.\n";
    }
};


bool is_number(const string &s) {
    return !s.empty() && find_if(s.begin(),
                                 s.end(), [](char c) { return !isdigit(c); }) == s.end();
}

int main() {
    sem_init(&sumProtector, 0, 1);
    vector<monitor<road> *> roads;
    vector<path> paths;
    string line;
    vector<thread> threads;
    ifstream myfile("test.txt");
    if (myfile.is_open()) {
        while (getline(myfile, line)) {
            line.erase(remove(line.begin(), line.end(), '\r'), line.end());
            if (line != "#") {
                string::size_type sz;
                auto *newRoad = new monitor<road>{line.substr(0, 5), stoi(line.substr(8, 1), &sz)};
                roads.push_back(newRoad);
            }
            if (line == "#") {
                while (getline(myfile, line)) {
                    line.erase(remove(line.begin(), line.end(), '\r'), line.end());
                    if (is_number(line)) {
                        int carCount = 0;
                        stringstream stream(line);
                        stream >> carCount;
                        paths[paths.size() - 1].carCount = carCount;
                    } else {
                        vector<monitor<road> *> passedRoads;
                        for (int i = 0; i < line.size() - 4; i += 4) {
                            for (auto road:roads) {
                                auto &rawVector = road->GetThreadUnsafeAccess();
                                if (rawVector.startEnd == line.substr(i, 5)) {
                                    passedRoads.push_back(road);
                                }
                            }
                        }
                        paths.emplace_back(passedRoads, 0);
                    }
                }
            }
        }
        myfile.close();
        for (int n = 0; n < paths.size(); n++) {
            for (int k = 0; k < paths[n].carCount; k++) {
                threads.emplace_back(calculate, paths[n].passedRoads, k, n);
            }
        }
        for (auto &thread: threads) {
            thread.join();
        }
        cout << totalPollution;

    } else cout << "Unable to open file";
    sem_destroy(&sumProtector);
    return 0;
}