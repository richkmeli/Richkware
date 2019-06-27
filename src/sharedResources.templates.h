/*
*      Copyright 2016 Riccardo Melioli.
*/

template<class T>
void SharedList<T>::add(T item) {
    sc.lock();
    list.push_back(item);
    sc.unlock();
}

template<class T>
void SharedList<T>::remove(T item) {
    sc.lock();
    list.remove(item);
    sc.unlock();
}

template<class T>
void SharedList<T>::clear() {
    sc.lock();
    list.clear();
    sc.unlock();
}

template<class T>
std::list<T> SharedList<T>::getCopy() {
    sc.lock();
    std::list<T> tmp = list;
    sc.unlock();
    return tmp;
}
