#ifndef __STD_LIST_HPP__
#define __STD_LIST_HPP__

//MaxScript standard: List
//Basically an std::vector but optimized for low memory usage & I want to

#include <ms/impl/common.hpp>
#include <limits>
#include <initializer_list>

MS_STD_BEGIN

//Use std::vector names?
#define USE_VNAMES 1

template<typename Type, typename Count = u32>
class CList {
private:
	Type* m_Data = nullptr;
	Count m_Length = 0;

	bool xExceededMax(s8 p_Difference) {
		return (static_cast<u64>(m_Length + p_Difference) > std::numeric_limits<Count>::max());
	}
	bool xOverRange(const Count& p_Value) {
		return (p_Value < 0 || p_Value >= m_Length);
	}
	
	void _extend(u8 p_Increase, Type* p_Data) {
		if (m_Length == 0) {
			m_Data = new Type[p_Increase]{ Type() };
			m_Length = p_Increase;
			for (Count i = 0; i < p_Increase;i++) {
				m_Data[i] = p_Data[i];
			}
			return;
		}
		if (xExceededMax(p_Increase)) {
			return;
		}
		Type* output = new Type[m_Length + p_Increase]{ Type() };
		Count i = 0;
		for (; i < m_Length;i++) {
			output[i] = m_Data[i];
		}
		if (p_Increase == 1) { //Type* is not a Type[], instead its a Type&
			output[m_Length] = *p_Data;
		}
		else {
			for (i = 0; i < p_Increase;i++) {
				output[m_Length + i] = p_Data[i];
			}
		}
		delete[] m_Data;
		m_Data = output;
		m_Length += p_Increase;
	}
public:
	//Extending/Pushing
	void push(Type p_Input) {
		_extend(1, &p_Input);
	}

	void push_some(Type* p_Input, u8 p_Amount) {
		_extend(p_Amount, p_Input);
	}

	void insert(Count p_Index, Type p_Value) {
		if (p_Index == m_Length) {
			push(p_Value);
			return;
		}
		m_Length++;
		Type* output = new Type[m_Length]{ Type() };
		Count i = 0;
		for (; i < p_Index;i++) {
			output[i] = m_Data[i];
		}
		//We could also use (i), but I want to be safe here
		output[i++] = p_Value;
		for (; i < m_Length;i++) {
			output[i] = m_Data[i + 1];
		}
		delete[] m_Data;
		m_Data = output;
	}


	//Indexes
	Type& at(Count p_Index) {
		if (xExceededMax(0)) {
			throw std::runtime_error("Failed");
		}
		return static_cast<Type&>(m_Data[p_Index]);
	}

	//This copies the std::vector standard of std::vector::at() checking for the range, but the [] operator using no bound checks
	Type& operator[](Count p_Index) {
		return static_cast<Type&>(m_Data[p_Index]);
	}

	Type& front() {
		return at(0);
	}
	Type& back() {
		return at(m_Length - 1);
	}

	//Utility
	Type* data() {
		return m_Data;
	}
	Count length() {
		return m_Length;
	}
	Count size() {
		return length();
	}
	bool is_empty() {
		return (m_Length == 0);
	}

	
	



	//Erasing
	void erase(Count p_Index, u8 p_Count = 1) {
		m_Length -= p_Count;
		Type* output = new Type[m_Length]{ Type() };
		for (Count i = 0; i < m_Length;i++) {
			output[i] = m_Data[p_Index + i + p_Count];
		}
		delete[] m_Data;
		m_Data = output;
	}

	//Changing as a whole
	void operator=(CList<Type, Count>& p_Other) {
		delete[] m_Data;
		m_Data = p_Other.m_Data;
		m_Length = p_Other.m_Length;
	}
	void operator=(std::initializer_list<Type> p_Other) {
		delete[] m_Data;
		m_Data = new Type[p_Other.size()];
		m_Length = static_cast<Count>(p_Other.size());
		for (Count i = 0; i < m_Length;i++) {
			m_Data[i] = p_Other.data()[i];
		}

	}

	Type* begin() {
		return m_Data;
	}
	Type* end() {
		return (m_Data + m_Length);
	}


	//Constructors/Deconstructors
	CList() {

	}

	~CList() {
		if (!m_Data)
			return;
		delete[] m_Data;
		m_Data = nullptr;
		
	}
};


MS_STD_END


#endif