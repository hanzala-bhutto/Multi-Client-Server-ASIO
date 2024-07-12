#pragma once
#include "thsfqueue.h"

namespace clsrv
{
	namespace net
	{
		template<typename T>
		ThSfQueue<T>::~ThSfQueue()
		{
			clear();
		}

		template<typename T>
		const T& ThSfQueue<T>::front()
		{
			std::scoped_lock lock(m_muxQueue);
			return m_deqQueue.front();
		}

		template<typename T>
		const T& ThSfQueue<T>::back()
		{
			std::scoped_lock lock(m_muxQueue);
			return m_deqQueue.back();
		}

		template<typename T>
		void ThSfQueue<T>::push_back(const T& item)
		{
			std::scoped_lock lock(m_muxQueue);
			m_deqQueue.emplace_back(std::move(item));
		}

		template<typename T>
		void ThSfQueue<T>::push_front(const T& item)
		{
			std::scoped_lock lock(m_muxQueue);
			m_deqQueue.emplace_front(std::move(item));
		}

		template<typename T>
		bool ThSfQueue<T>::empty()
		{
			std::scoped_lock lock(m_muxQueue);
			return m_deqQueue.empty();
		}

		template<typename T>
		size_t ThSfQueue<T>::count()
		{
			std::scoped_lock lock(m_muxQueue);
			return m_deqQueue.size();
		}

		template<typename T>
		void ThSfQueue<T>::clear()
		{
			std::scoped_lock lock(m_muxQueue);
			m_deqQueue.clear();
		}

		template<typename T>
		T ThSfQueue<T>::pop_front()
		{
			std::scoped_lock lock(m_muxQueue);
			auto t = std::move(m_deqQueue.front());
			m_deqQueue.pop_front();
			return t;
		}

		template<typename T>
		T ThSfQueue<T>::pop_back()
		{
			std::scoped_lock lock(m_muxQueue);
			auto t = std::move(m_deqQueue.back());
			m_deqQueue.pop_back();
			return t;
		}
	}
}

