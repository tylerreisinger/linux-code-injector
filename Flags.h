/*
 * Flags.h
 *
 *  Created on: Aug 30, 2013
 *      Author: tyler
 */

#ifndef FLAGS_H_
#define FLAGS_H_

#include <type_traits>
#include <initializer_list>
#include <boost/iterator/iterator_facade.hpp>

template <typename T>
class FlagsIterator;


/**
 * @brief Provides a safe way to store a bitwise or of enum values.
 */
template <typename T>
class Flags
{
public:
	typedef T Enum;
	typedef FlagsIterator<T> iterator;
	typedef FlagsIterator<T> Iterator;
	using StorageType = typename std::underlying_type<T>::type;

	Flags();
	Flags(T initialValue);
	Flags(const std::initializer_list<T>& flags);
	~Flags() = default;

	Flags(const Flags<T>& flags) = default;
	Flags(Flags<T>&& flags) = default;
	Flags<T>& operator =(const Flags<T>& flags) = default;
	Flags<T>& operator =(Flags<T>&& flags) = default;

	Flags<T>& operator =(const std::initializer_list<T>& flags);

	bool operator ==(const Flags<T>& flags) const {return m_value == flags.m_value;}
	bool operator ==(T value) const {return m_value == static_cast<StorageType>(value);}
	bool operator !=(const Flags<T>& flags) const {return !(m_value == flags.m_value);}
	bool operator !=(T value) const {return !(m_value == value);}

	Flags<T> operator |(T flag) const {return Flags<T>(m_value | static_cast<StorageType>(flag));}
	Flags<T> operator |(const Flags<T>& flags) const {return Flags<T>(m_value | flags.m_value);}
	Flags<T>& operator |=(T flag) {m_value |= static_cast<StorageType>(flag); return *this;}
	Flags<T>& operator |=(const Flags<T>& flags) {m_value |= flags.m_value; return *this;}
	Flags<T> operator &(T flag) const {return Flags<T>(m_value & static_cast<StorageType>(flag));}
	Flags<T> operator &(const Flags<T>& flags) const {return Flags<T>(m_value & flags.m_value);}
	Flags<T>& operator &=(T flag) {m_value &= static_cast<StorageType>(flag); return *this;}
	Flags<T>& operator &=(const Flags<T>& flags) {m_value &= flags.m_value; return *this;}
	Flags<T> operator ^(T flag) const {return Flags<T>(m_value ^ static_cast<StorageType>(flag));}
	Flags<T> operator ^(const Flags<T>& flags) const {return Flags<T>(m_value ^ flags.m_value);}
	Flags<T>& operator ^=(T flag) {m_value ^= static_cast<StorageType>(flag); return *this;}
	Flags<T>& operator ^=(const Flags<T>& flags) {m_value ^= flags.m_value; return *this;}
	Flags<T> operator ~() const {return Flags<T>(~m_value);}

	bool HasFlag(T flag) const;
	void ToggleFlag(T flag);
	void SetFlag(T flag);
	void ClearFlag(T flag);

	StorageType GetRawValue() const {return m_value;}

	Iterator Begin() {return Iterator(m_value, 0);}
	Iterator End() {return Iterator(m_value, -1);}
	Iterator begin() {return Begin();}
	Iterator end() {return End();}

	friend Flags<T> operator &(T flag, const Flags<T>& flags) {return flags & flag;}
	friend Flags<T> operator |(T flag, const Flags<T>& flags) {return flags | flag;}
	friend Flags<T> operator ^(T flag, const Flags<T>& flags) {return flags ^ flag;}

	friend Flags<T> operator &(T flag1, T flag2) {return Flags<T>(static_cast<StorageType>(flag1) & static_cast<StorageType>(flag2));}
	friend Flags<T> operator |(T flag1, T flag2) {return Flags<T>(static_cast<StorageType>(flag1) | static_cast<StorageType>(flag2));}
	friend Flags<T> operator ^(T flag1, T flag2) {return Flags<T>(static_cast<StorageType>(flag1) ^ static_cast<StorageType>(flag2));}

protected:
	explicit Flags(StorageType value);
	StorageType m_value = 0;
};

template <typename T>
class FlagsIterator : public boost::iterator_facade<FlagsIterator<T>, const Flags<T>, boost::forward_traversal_tag, T>
{
	friend class boost::iterator_core_access;
	friend class Flags<T>;
public:
	~FlagsIterator() = default;

	FlagsIterator(const FlagsIterator<T>&) = default;
	FlagsIterator(FlagsIterator<T>&&) = default;
	FlagsIterator& operator =(const FlagsIterator<T>&) = default;
	FlagsIterator& operator =(FlagsIterator<T>&&) = default;

protected:
	FlagsIterator(typename Flags<T>::StorageType value, int currentBit = 0);

	void increment();

	bool equal(const FlagsIterator<T>& otherIterator) const ;

	T dereference() const;



	int m_curBit = 0;
	typename Flags<T>::StorageType m_flagsValue;
};

template <typename T>
Flags<T>::Flags():
m_value(0)
{
}

template <typename T>
Flags<T>::Flags(T flag):
m_value(static_cast<StorageType>(flag))
{
}

template <typename T>
Flags<T>::Flags(const std::initializer_list<T>& flags)
{
	for(auto flag : flags)
	{
		m_value |= static_cast<StorageType>(flag);
	}
}

template<typename T>
inline Flags<T>& Flags<T>::operator =(const std::initializer_list<T>& flags)
{
	m_value = 0;
	for(auto flag : flags)
	{
		m_value |= static_cast<StorageType>(flag);
	}
	return *this;
}

template <typename T>
inline Flags<T>::Flags(StorageType value):
	m_value(value)
{
}

template <typename T>
inline bool Flags<T>::HasFlag(T flag) const
{
	return (m_value & static_cast<StorageType>(flag)) > 0;
}

template <typename T>
inline void Flags<T>::ToggleFlag(T flag)
{
	m_value ^= static_cast<StorageType>(flag);
}

template <typename T>
inline void Flags<T>::SetFlag(T flag)
{
	m_value |= static_cast<StorageType>(flag);
}

template <typename T>
inline void Flags<T>::ClearFlag(T flag)
{
	m_value &= ~static_cast<StorageType>(flag);
}

template<typename T>
inline FlagsIterator<T>::FlagsIterator(typename Flags<T>::StorageType value, int currentBit):
	m_curBit(currentBit), m_flagsValue(value)
{
	if(currentBit == -1)
	{
		return;
	}
	for(int i = m_curBit; i < sizeof(typename Flags<T>::StorageType) * 8; ++i)
	{
		if((m_flagsValue & (1 << i)) > 0)
		{
			m_curBit = i;
			break;
		}
	}
}

template<typename T>
inline void FlagsIterator<T>::increment()
{
	if(m_curBit == -1)
	{
		return;
	}
	for(int i = m_curBit + 1; i < sizeof(typename Flags<T>::StorageType) * 8; ++i)
	{
		if((m_flagsValue & (1 << i)) > 0)
		{
			m_curBit = i;
			return;
		}
	}
	//Nothing was found, set to invalid value
	m_curBit = -1;
}

template<typename T>
inline bool FlagsIterator<T>::equal(const FlagsIterator<T>& otherIterator) const
{
	return (m_curBit == otherIterator.m_curBit && m_flagsValue == otherIterator.m_flagsValue);
}

template<typename T>
inline T FlagsIterator<T>::dereference() const
{
	if(m_curBit == -1)
	{
		return static_cast<T>(0);
	}
	else
	{
		return static_cast<T>(1 << m_curBit);
	}
}

#endif /* FLAGS_H_ */
