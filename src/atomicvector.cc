/*
 *   Copyright (C) 2019 by Andreas Theofilu (TheoSys) <andreas@theosys.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <utility>

template<typename T> void AtomicVector<T>::insert(T in, const int index)
{
    std::lock_guard<std::mutex> lock(mut);
    vec[index] = std::move(in);
    cond.notify_one();
}

template<typename T> void AtomicVector<T>::push_back(T in)
{
    std::lock_guard<std::mutex> lock(mut);
    vec.push_back(std::move(in));
    cond.notify_one();
}

template<typename T> T & AtomicVector<T>::operator[](const int index)
{
    return vec[index];
}

template<typename T> T & AtomicVector<T>::at(const int index)
{
    return vec[index];
}

template<typename T> typename std::vector<T>::iterator AtomicVector<T>::erase(typename std::vector<T>::iterator it)
{
	std::lock_guard<std::mutex> lock(mut);
	typename std::vector<T>::iterator rit = vec.erase(it);
	cond.notify_one();
	return rit;
}

template<typename T> size_t AtomicVector<T>::size()
{
	return vec.size();
}

template<typename T> typename std::vector<T>::iterator AtomicVector<T>::begin()
{
    return vec.begin();
}

template<typename T> typename std::vector<T>::iterator AtomicVector<T>::end()
{
    return vec.end();
}

template<typename T> std::vector<T> AtomicVector<T>::toVector()
{
    return vec;
}
