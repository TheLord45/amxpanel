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

#ifndef __ATOMIC_VECTOR_H__
#define __ATOMIC_VECTOR_H__

#include <vector>
#include <mutex>
#include <condition_variable>

template <typename T>
class AtomicVector
{
	public:
		AtomicVector() : vec(), mut(), cond() {}
		AtomicVector(const AtomicVector& orig) : vec(orig.vec), mut(), cond() {}

		void insert(T in, const int index);
//		void push_back(T in);
		void push_back(const T& in);
		T& operator[](const int index);
		T& at(const int index);
		T& front();
		T& back();
		size_t size();
		typename std::vector<T>::iterator erase(typename std::vector<T>::iterator it);
		typename std::vector<T>::iterator begin();
		typename std::vector<T>::iterator end();
		std::vector<T> toVector();

	private:
		std::vector<T> vec;
		std::mutex mut;
		std::condition_variable cond;
};

#include "atomicvector.cc"

#endif
