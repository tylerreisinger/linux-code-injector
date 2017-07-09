/*
 * MemoryRegion.cpp
 *
 *  Created on: Nov 20, 2013
 *      Author: tyler
 */

#include "MemoryRegion.h"

MemoryRegion::MemoryRegion(intptr_t startPtr, intptr_t endPtr, intptr_t fileOffset,
		const Flags<ProtectionFlags>& flags, const std::string& name):
	m_startPtr(startPtr), m_endPtr(endPtr), m_fileOffset(fileOffset),
	m_protectionFlags(flags), m_name(name)
{

}

MemoryRegion::~MemoryRegion()
{
}

MemoryRegion::MemoryRegion(const MemoryRegion& other):
	m_startPtr(other.m_startPtr), m_endPtr(other.m_endPtr), m_fileOffset(other.m_fileOffset),
	m_protectionFlags(other.m_protectionFlags), m_name(other.m_name)
{
}

MemoryRegion::MemoryRegion(MemoryRegion&& other):
	m_startPtr(other.m_startPtr), m_endPtr(other.m_endPtr), m_fileOffset(other.m_fileOffset),
	m_protectionFlags(other.m_protectionFlags), m_name(std::move(other.m_name))
{
}

MemoryRegion& MemoryRegion::operator =(const MemoryRegion& other)
{
	m_startPtr = other.m_startPtr;
	m_endPtr = other.m_endPtr;
	m_fileOffset = other.m_fileOffset;
	m_protectionFlags = other.m_protectionFlags;
	m_name = other.m_name;
	return *this;
}

MemoryRegion& MemoryRegion::operator =(MemoryRegion&& other)
{
	m_startPtr = other.m_startPtr;
	m_endPtr = other.m_endPtr;
	m_fileOffset = other.m_fileOffset;
	m_protectionFlags = other.m_protectionFlags;
	m_name = std::move(other.m_name);
	return *this;
}
