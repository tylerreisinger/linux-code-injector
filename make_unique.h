/*
 * make_unique.h
 *
 *  Created on: Oct 8, 2013
 *      Author: tyler
 */

#ifndef MAKE_UNIQUE_H_
#define MAKE_UNIQUE_H_

#include <memory>


///std::make_unique is proposed to be implemented in C++14 but is not yet supported by gcc.
///In order to use the functionality it provides and to be forward compatible with
///the standard proposition, it is implemented here for the time being.
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args&& ...args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}



#endif /* MAKE_UNIQUE_H_ */
