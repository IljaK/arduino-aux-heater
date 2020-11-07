#include "Timer.h"

TimerNode *Timer::pFirst = NULL;
unsigned long Timer::frameTS = 0;

ITimerCallback::~ITimerCallback()
{
	Timer::StopAll(this);
};
void ITimerCallback::OnTimerComplete(TimerID timerId)
{ 
};

TimerID Timer::Start(ITimerCallback *pCaller, unsigned long duration)
{
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
				break;
			}
		}
	}

	return newNode->id;
}

void Timer::Loop()
{
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

			freeNode->pCaller->OnTimerComplete(freeNode->id);

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
}
bool Timer::Stop(TimerID timerId)
{
	if (timerId == 0) return false;

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
			return true;
		}
		pPrev = pNode;
	}
	return false;
}


bool Timer::StopAll(ITimerCallback* pCaller)
{
	bool isDestroyed = false;

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

	return isDestroyed;
}

unsigned long Timer::Remain(TimerID timerId)
{
	for (TimerNode *pNode = pFirst; pNode; pNode = pNode->pNext) {
		if (pNode->id == timerId) {
			return pNode->remain;
		}
	}
	return 0;
}