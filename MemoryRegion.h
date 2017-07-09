/*
 * MemoryRegion.h
 *
 *  Created on: Nov 20, 2013
 *      Author: tyler
 */

#ifndef MEMORYREGION_H_
#define MEMORYREGION_H_

#include <cstdint>
#include <string>

#include "Flags.h"

class MemoryRegion
{
public:
	enum class ProtectionFlags {None, Read = 1, Write = 2, Execute = 4, Shared = 8};

	MemoryRegion(intptr_t startPtr, intptr_t endPtr, intptr_t fileOffset = 0,
			const Flags<ProtectionFlags>& flags = ProtectionFlags::None, const std::string& name = "");
	~MemoryRegion();

	MemoryRegion(const MemoryRegion& other);
	MemoryRegion(MemoryRegion&& other);
	MemoryRegion& operator =(const MemoryRegion& other);
	MemoryRegion& operator =(MemoryRegion&& other);

	intptr_t getStartPtr() const {return m_startPtr;}
	intptr_t getEndPtr() const {return m_endPtr;}
	intptr_t getSize() const{return m_endPtr - m_startPtr;}

	intptr_t getFileOffset() const {return m_fileOffset;}

	Flags<ProtectionFlags> getProtectionFlags() const {return m_protectionFlags;}

	const std::string& getName() const {return m_name;}

protected:
	intptr_t m_startPtr = 0;
	intptr_t m_endPtr = 0;
	intptr_t m_fileOffset = 0;

	Flags<ProtectionFlags> m_protectionFlags;

	std::string m_name;
};

#endif /* MEMORYREGION_H_ */
