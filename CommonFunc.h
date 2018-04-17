//
// Created by henry on 18-4-17.
//

#ifndef HYPERTOOL_COMMONFUNC_H
#define HYPERTOOL_COMMONFUNC_H

template <class T>
inline void ReleaseArray(T* array) {
    if (nullptr != array) {
        delete[] array;
        array = nullptr;
    }
}

#endif //HYPERTOOL_COMMONFUNC_H
