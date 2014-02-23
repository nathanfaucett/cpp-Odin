#ifndef ODIN_ARRAY_H
#define ODIN_ARRAY_H

namespace Odin {

	template <class Type>
	class Array {

		private:
			Type* m_array;
			unsigned int m_length;

			inline void m_QuickSort(std::function<int(const Type a, const Type b)> sortFunction, const int from, const int to);
			inline int m_Partition(std::function<int(const Type a, const Type b)> sortFunction, int i, int j);

		protected:


		public:

			inline explicit Array(void);
			inline explicit Array(const unsigned int length);
			inline ~Array(void);

			inline Type operator [](const unsigned int i);

			inline Type Splice(const unsigned int index, const unsigned int count);

			inline Type Push(const Type item);
			inline Type Unshift(const Type item);

			inline unsigned int Length(void);

			inline void Sort(std::function<int(const Type a, const Type b)> sortFunction);
	};
}
#endif