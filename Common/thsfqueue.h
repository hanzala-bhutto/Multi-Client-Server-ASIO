#pragma once
#include "common.h"

namespace clsrv
{
	namespace net
	{
		template<typename T>
		class ThSfQueue
		{
		public:
			ThSfQueue() = default;
			ThSfQueue(const ThSfQueue<T>&) = delete;
			virtual ~ThSfQueue();
			const T& front();
			const T& back();
			void push_back(const T& item);
			void push_front(const T& item);
			bool empty();
			size_t count();
			void clear();
			T pop_front();
			T pop_back();
		protected:
			std::mutex m_muxQueue;
			std::deque<T> m_deqQueue;
		};
	}
}