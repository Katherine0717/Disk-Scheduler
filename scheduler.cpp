#include <fstream>
#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <climits>
#include "thread.h"
#include "disk.h"
#include <utility>
#include <unordered_map>

using std::cout;
using std::endl;
using std::pair;
using std::vector;
// using std::unordered_map;


mutex mutex_queue;
cv cv_queue;
cv cv_service;
int max_disk_queue;
std::unordered_map<int, int> queue;
int global_argc;
int last_track = 0;
int active_requesters;

// 1. 写逻辑
// 2. 想想要怎么加锁 
// 3. thoughts: critical section == queue

/* Requester
 * Input: requester index, an array of track number
*/
void requester(void* a)
{
    active_requesters++;
    char* filename = (char*) a;
    int idx = atoi(filename+7);

    //read file
    std::ifstream disk_file;
    disk_file.open(filename);

    int n;
    vector<int> track_arr;
    while (disk_file >> n) {
        track_arr.push_back(n);
    }
    disk_file.close();
    for(int i = 0; i < track_arr.size(); i++){
        mutex_queue.lock();
        while(queue.size() >= max_disk_queue){
            cv_queue.wait(mutex_queue);
        }

        while(queue.find(idx) != queue.end()){
            cv_queue.wait(mutex_queue);
        }
        queue.insert({idx, track_arr[i]});
        print_request(idx, track_arr[i]);

        cv_service.signal();
        mutex_queue.unlock();
    }
    active_requesters--;
}

void servicer(void *a)
{
    // lock??
    // find the smallest seek time in the queue
    // removed it from the queue
    // print
    while(true){
        mutex_queue.lock();
        while(!(queue.size() >= max_disk_queue || (active_requesters < max_disk_queue && !queue.empty()))) {
            cv_service.wait(mutex_queue);
        }
        int min_ = INT_MAX;
        int curr_req;
        for (const auto& pair : queue) {
            if(abs(pair.second - last_track) < min_){
                min_ = abs(pair.second - last_track);
                curr_req = pair.first;
            }
        }
        if (auto search = queue.find(curr_req); search != queue.end()){
            last_track = search -> second;
            queue.erase(curr_req);
            print_service(curr_req, last_track);
            cv_queue.broadcast();  
        }
        if (active_requesters == 0 && queue.empty()) {
            // cout << active_requesters << endl;
            break;
        }
        mutex_queue.unlock();
    }
}

void prog_start(void *argv){
    // 创建所有的threads
    // request0
    // request1
    // request2
    // request3
    // ...
    // service
    thread(servicer, (void*) 0);
    for(int i = 2; i < global_argc; i++){
        char** filenames = static_cast<char**>(argv);
        // cout << 34 << endl;
        thread(requester, (void*) filenames[i]);
    }
}

int main(int argc, char *argv[])
{
    max_disk_queue = atoi(argv[1]);
    // cout << max_disk_queue << endl;
    global_argc = argc;

    if(argc > 2){
        cpu::boot((thread_startfunc_t) prog_start, argv, 0);
    }
}
