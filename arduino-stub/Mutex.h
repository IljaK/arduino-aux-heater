#pragma once
#include <mutex>

#define portMAX_DELAY ( uint32_t ) 0xffffffffUL;

class AbstractMutex
{
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
};

class Mutex: public AbstractMutex
{
private:
    std::mutex mtx;
public:
    void lock() override {
        mtx.lock();
    };
    void unlock() override {
        mtx.unlock();
    };
};

class RecoursiveMutex: public AbstractMutex
{
private:
    std::recursive_mutex mtx;
public:
    void lock() override {
        mtx.lock();
    };
    void unlock() override {
        mtx.unlock();
    };
};

typedef AbstractMutex * SemaphoreHandle_t;

// SemaphoreHandle_t xTimerSemaphore = xSemaphoreCreateMutex
//SemaphoreHandle_t xTimerSemaphore = xSemaphoreCreateRecursiveMutex();
//
//	xSemaphoreTakeRecursive( xTimerSemaphore, portMAX_DELAY );
//    xSemaphoreGiveRecursive(xTimerSemaphore);
//	xSemaphoreTake( xTimerSemaphore, portMAX_DELAY );
//    xSemaphoreGive(xTimerSemaphore);

#define xSemaphoreCreateMutex() new Mutex();
#define xSemaphoreCreateRecursiveMutex() new RecoursiveMutex();

#define xSemaphoreTake( xSemaphore, xBlockTime ) xSemaphore->lock();
#define xSemaphoreGive( xSemaphore ) xSemaphore->unlock();

#define xSemaphoreTakeRecursive( xSemaphore, xBlockTime ) xSemaphore->lock();
#define xSemaphoreGiveRecursive( xSemaphore ) xSemaphore->unlock();

/*
SemaphoreHandle_t xSemaphoreCreateMutex()
{
    return new Mutex();
}
SemaphoreHandle_t xSemaphoreCreateRecursiveMutex()
{
    return new RecoursiveMutex();
}

void xSemaphoreTake(SemaphoreHandle_t xSemaphore, uint32_t xBlockTime ) {
    xSemaphore->lock();
}
void xSemaphoreGive(SemaphoreHandle_t xSemaphore) {
    xSemaphore->unlock();
}

void xSemaphoreTakeRecursive( SemaphoreHandle_t xSemaphore, uint32_t xBlockTime ) {
    xSemaphore->lock();
}
void xSemaphoreGiveRecursive(SemaphoreHandle_t xSemaphore) {
    xSemaphore->unlock();
}
*/