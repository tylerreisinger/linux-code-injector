/*
 * ElfMemoryFile64.h
 *
 *  Created on: Dec 8, 2013
 *      Author: tyler
 */

#ifndef ELFMEMORYFILE64_H_
#define ELFMEMORYFILE64_H_

#include <memory>
#include <vector>
#include <cstdint>
#include <unordered_map>

#include <elf.h>

#include "MemoryRegion.h"
#include "ElfMemorySection64.h"


class Process;

namespace Elf
{

struct ElfExport64
{
	uintptr_t offset;
	int sectionIndex;
};

class ElfMemoryFile64
{
public:

	ElfMemoryFile64(const std::vector<MemoryRegion>& memoryRegions, const Process& process);
	~ElfMemoryFile64();

	ElfMemoryFile64(const ElfMemoryFile64&) = delete;
	ElfMemoryFile64(ElfMemoryFile64&&) = delete;
	ElfMemoryFile64& operator =(const ElfMemoryFile64&) = delete;
	ElfMemoryFile64& operator =(ElfMemoryFile64&&) = delete;

	uintptr_t getMemoryPointerFromFileOffset(uintptr_t offset) const;
	uint8_t* getMemoryDataPointerFromOffset(uintptr_t offset) const;

	const std::vector<ElfMemorySection64>& getSections() const {return m_sections;}
	const ElfMemorySection64* getSectionByName(const std::string& name) const;
	const ElfMemorySection64* getSectionByIndex(int index) const {return &m_sections[index];}

	ElfExport64 getExport(const std::string& name) const {return m_exports.at(name);}
	const std::unordered_map<std::string, ElfExport64> getExports() const {return m_exports;}

protected:
	Elf64_Ehdr m_header;

	std::vector<MemoryRegion> m_memoryRegions;
	std::vector<std::unique_ptr<uint8_t []>> m_memoryData;
	std::vector<ElfMemorySection64> m_sections;

	std::unordered_map<std::string, ElfExport64> m_exports;

	const Process* m_process;
};

}

#endif /* ELFMEMORYFILE64_H_ */
