#include <atomic>
#include <cstring>
#include <iostream>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

#include "f.h"
#include "g.h"


struct param {
    int input;

    int (*func)(int);

    std::atomic<bool> is_done;
    int output;
};


void *wrapper(void *ptr) {
    auto arg = (param *) ptr;
    arg->output = arg->func(arg->input);
    arg->is_done = true;
    //std::cout << "Computing is over for one of functions, res: " << arg->output << "\n";
    return nullptr;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return -1;
    }

    int x = std::stoi(argv[1]);
    key_t key = ftok("shmfile", 42);
    size_t len = 2 * sizeof(param);
    int shmid = shmget(key, len, 0666 | IPC_CREAT);

    auto params = (param *) shmat(shmid, nullptr, 0);

    std::memset(params, 0, len);
    params[0].func = f;
    params[1].func = g;
    params[0].input = x;
    params[1].input = x;

    std::cout << "Starting threads, for aborting pls press Ctrl+C\n";

    pthread_t threads[2];
    pthread_create(&threads[0], nullptr, wrapper, &params[0]);
    pthread_create(&threads[1], nullptr, wrapper, &params[1]);
    pthread_detach(threads[0]);
    pthread_detach(threads[1]);

    std::cout << std::boolalpha;
    while (true) {
        if (params[0].is_done && params[0].output != 0) {
            std::cout << "Result: " << true << "\n";
            break;
        }
        if (params[1].is_done && params[1].output != 0) {
            std::cout << "Result: " << true << "\n";
            break;
        }
        if (params[0].is_done && params[1].is_done) {
            std::cout << "Result: " << (params[0].output || params[1].output) << "\n";
            break;
        }

    }

    shmdt((void *) params);
    shmctl(shmid, IPC_RMID, nullptr);

    return 0;
}
