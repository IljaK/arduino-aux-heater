#include "Timer.h"

TimerNode *Timer::pFirst = NULL;
unsigned long Timer::frameTS = 0;
SemaphoreHandle_t Timer::xTimerSemaphore = xSemaphoreCreateRecursiveMutex();

ITimerCallback::~ITimerCallback()
{
	Timer::StopAll(this);
};
void ITimerCallback::OnTimerComplete(TimerID timerId, uint8_t data)
{ 
};

TimerID Timer::Start(ITimerCallback *pCaller, unsigned long duration, uint8_t data)
{
	xSemaphoreTakeRecursive( xTimerSemaphore, portMAX_DELAY );

	TimerNode *newNode = new TimerNode();
	newNode->pCaller = pCaller;
	newNode->remain = duration;

	if (pFirst == NULL || pFirst->id != 1u) {
		if (pFirst != NULL) newNode->pNext = pFirst;
		pFirst = newNode;
		newNode->id = 1u;
	} else {
		for (TimerNode *pNode = pFirst; pNode; pNode = pNode->pNext ) {
			if (pNode->pNext == NULL || pNode->pNext->id - pNode->id > 1) {
				newNode->pNext = pNode->pNext;
				pNode->pNext = newNode;
				newNode->id = pNode->id + 1u;
				newNode->data = data;
				break;
			}
		}
	}

	TimerID id = newNode->id;
	
    xSemaphoreGiveRecursive(xTimerSemaphore);
	return id;
}

void Timer::Loop()
{
	xSemaphoreTakeRecursive( xTimerSemaphore, portMAX_DELAY );

	unsigned long microsTS = micros();
	unsigned long delta = microsTS - frameTS;
	frameTS = microsTS;


	TimerNode *pPrev = NULL;

	for (TimerNode* pNode = pFirst; pNode; ) {
		
		if (pNode->remain <= delta) {
			TimerNode *freeNode = pNode;
			pNode = pNode->pNext;

			if (freeNode == pFirst) {
				pFirst = pNode;
				pPrev = NULL;
			} else {
				pPrev->pNext = pNode;
			}

			freeNode->pCaller->OnTimerComplete(freeNode->id, freeNode->data);

			// pPrev node could be changed during callback
			if (pNode != NULL) {
				if ((pPrev == NULL && pFirst != pNode) || (pPrev != NULL && pPrev->pNext != pNode)) {
					for (TimerNode* pN = pFirst; pN; pN = pN->pNext) {
						if (pN->pNext && pN->pNext == pNode) {
							pPrev = pN;
							break;
						}
					}
				}
			}

			freeNode->remain = 0;
			freeNode->pNext = NULL;
			delete(freeNode);

			continue;
		}
		pNode->remain -= delta;
		pPrev = pNode;
		pNode = pNode->pNext;
	}
    xSemaphoreGiveRecursive(xTimerSemaphore);
}
bool Timer::Stop(TimerID timerId)
{
	if (timerId == 0) return false;

	xSemaphoreTakeRecursive( xTimerSemaphore, portMAX_DELAY );
	bool result = false;

	TimerNode *pPrev = NULL;
	for (TimerNode *pNode = pFirst; pNode; pNode = pNode->pNext) {

		if (pNode->id == timerId) {
			if (pNode == pFirst) {
				pFirst = pNode->pNext;
				pPrev = NULL;
			} else {
				pPrev->pNext = pNode->pNext;
			}
			delete(pNode);
			result = true;
			break;
		}
		pPrev = pNode;
	}

    xSemaphoreGiveRecursive(xTimerSemaphore);
	return result;
}


void Timer::StopAll(ITimerCallback* pCaller)
{
	xSemaphoreTakeRecursive( xTimerSemaphore, portMAX_DELAY );
	TimerNode* pPrev = NULL;

	for (TimerNode* pNode = pFirst; pNode; ) {

		if (pNode->pCaller <= pCaller) {
			TimerNode* freeNode = pNode;
			pNode = pNode->pNext;

			if (freeNode == pFirst) {
				pFirst = freeNode->pNext;
				pPrev = NULL;
			}
			else {
				pPrev->pNext = freeNode->pNext;
			}
			delete(freeNode);

			continue;
		}
		pPrev = pNode;
		pNode = pNode->pNext;
	}
    xSemaphoreGiveRecursive(xTimerSemaphore);
}

unsigned long Timer::Remain(TimerID timerId)
{
	xSemaphoreTakeRecursive( xTimerSemaphore, portMAX_DELAY );
	unsigned long remain = 0;
	for (TimerNode *pNode = pFirst; pNode; pNode = pNode->pNext) {
		if (pNode->id == timerId) {
			remain = pNode->remain;
			break;
		}
	}
    xSemaphoreGiveRecursive(xTimerSemaphore);
	return remain;
}

bool Timer::Contains(ITimerCallback *pCaller, uint8_t data)
{
	xSemaphoreTakeRecursive( xTimerSemaphore, portMAX_DELAY );
	bool result = false;
	for (TimerNode *pNode = pFirst; pNode; pNode = pNode->pNext) {
		if (pNode->pCaller == pCaller && pNode->data == data) {
			result = true;
			break;
		}
	}
    xSemaphoreGiveRecursive(xTimerSemaphore);
	return result;
}