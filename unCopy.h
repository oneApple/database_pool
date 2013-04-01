/*
 * unCopy.h
 *
 *  Created on: 2013-4-1
 *      Author: keym
 */

#ifndef UNCOPY_H_
#define UNCOPY_H_

//禁止复制
class uncopy{
protected:
uncopy(){}
~uncopy(){}
private:
uncopy(const uncopy&);
uncopy& operator= (const uncopy&);
};


#endif /* UNCOPY_H_ */
