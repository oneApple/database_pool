/*
 * guard.h
 *
 *  Created on: 2013-4-1
 *      Author: keym
 */

#ifndef GUARD_H_
#define GUARD_H_

//释放互斥锁
#include<pthread.h>

class guard{
private:
	bool success;
	pthread_mutex_t *m;
public:
	guard(pthread_mutex_t *mutex)
	{
		this->m = mutex;
		if(pthread_mutex_trylock(this->m) != 0)
		{
			success = 0;
		}
		else
		{
			success = 1;
		}
	}

	bool getstate()
	{
		return this->success;
	}

	~guard()
	{
		if(this->success)
		{
			pthread_mutex_unlock(m);
		}
	}
};


#endif /* GUARD_H_ */
