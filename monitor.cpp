#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <mutex>
#include <thread>
#include <cstdlib>

using namespace std;




template<class T>
class monitor
{
public:
    template<typename ...Args>
    monitor(Args&&... args) : m_cl(forward<Args>(args)...){}

    struct monitor_helper
    {
        monitor_helper(monitor* mon) : m_mon(mon), m_ul(mon->m_lock) {}
        T* operator->() { return &m_mon->m_cl;}
        monitor* m_mon;
        unique_lock<mutex> m_ul;
    };

    monitor_helper operator->() { return monitor_helper(this); }
    monitor_helper ManuallyLock() { return monitor_helper(this); }
    T& GetThreadUnsafeAccess() { return m_cl; }

private:
    T           m_cl;
    mutex  m_lock;
};
void pushing(monitor<vector<int>> threadSafeVector){
    for(int i=0; i<1024; ++i)
    {
        threadSafeVector->push_back(i);
    }
}
int main() {
    srand(time(nullptr));
    int h = 1;
    int p = 1;
    long sum = 0;
    for (long k = 0; k < 10000000; k++) {
        sum += floor(k / (1000000 * p * h));
    };
    cout << sum;
    return 0;
}