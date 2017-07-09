/*
 * ElfMemoryFile64.cpp
 *
 *  Created on: Dec 8, 2013
 *      Author: tyler
 */

#include "ElfMemoryFile64.h"

#include "Process.h"

#include <iostream>

namespace Elf
{

ElfMemoryFile64::ElfMemoryFile64(const std::vector<MemoryRegion>& memoryRegions, const Process& process):
	m_memoryRegions(memoryRegions), m_process(&process)
{
	for(const auto& rgn : m_memoryRegions)
	{
		m_memoryData.push_back(process.readMemory(rgn.getStartPtr(), rgn.getSize()));
	}
	int i = 0;
	for(const auto& region : m_memoryRegions)
	{
		//Find the header
		if(region.getFileOffset() == 0)
		{
			uint8_t* data = m_memoryData[i].get();
			m_header = *reinterpret_cast<Elf64_Ehdr*>(data);

			if(m_header.e_ident[0] != ELFMAG0 || m_header.e_ident[1] != ELFMAG1 || m_header.e_ident[2] != ELFMAG2 || m_header.e_ident[3] != ELFMAG3)
			{
				throw std::runtime_error("Given data does not represent a valid ELF image");
			}
			if(m_header.e_ident[4] != ELFCLASS64)
			{
				throw std::runtime_error("ELF image is not 64-bit");
			}

			Elf64_Shdr* sectionTablePtr = reinterpret_cast<Elf64_Shdr*>(getMemoryDataPointerFromOffset(getMemoryPointerFromFileOffset(m_header.e_shoff)));
			char* stringTable = reinterpret_cast<char*>(getMemoryDataPointerFromOffset(getMemoryPointerFromFileOffset(sectionTablePtr[m_header.e_shstrndx].sh_offset)));
			for(int i = 0; i < m_header.e_shnum; ++i)
			{
				Elf64_Shdr header = sectionTablePtr[i];
				std::string name;
				if(header.sh_name != 0)
				{
					name = &stringTable[header.sh_name];

					int x = 1;
				}
				m_sections.push_back(Elf::ElfMemorySection64(header, name));
				if(name == ".dynsym")
				{

					int x = 0;
				}
			}


		}
		i++;
	}

	const ElfMemorySection64* dynamicSymbolSection = getSectionByName(".dynsym");
	const ElfMemorySection64* dynamicStringTableSection = getSectionByName(".dynstr");

	Elf64_Sym* dynamicSymbolTable = reinterpret_cast<Elf64_Sym*>(getMemoryDataPointerFromOffset(getMemoryPointerFromFileOffset(dynamicSymbolSection->getHeader().sh_offset)));
	char* dynamicStringTable = reinterpret_cast<char*>(getMemoryDataPointerFromOffset(getMemoryPointerFromFileOffset(dynamicStringTableSection->getHeader().sh_offset)));
	for(int i = 0; i < dynamicSymbolSection->getHeader().sh_size / sizeof(Elf64_Sym); ++i)
	{
		std::string name;
		if(dynamicSymbolTable[i].st_name != 0)
		{
			name = &dynamicStringTable[dynamicSymbolTable[i].st_name];
		}

		if(dynamicSymbolTable[i].st_name != 0 && dynamicSymbolTable[i].st_value != 0)
		{
			m_exports.insert(std::make_pair(name, ElfExport64{dynamicSymbolTable[i].st_value, dynamicSymbolTable[i].st_shndx}));
		}
		int x = 0;
	}
}

ElfMemoryFile64::~ElfMemoryFile64()
{
}

uintptr_t ElfMemoryFile64::getMemoryPointerFromFileOffset(uintptr_t offset) const
{
	for(const auto& region : m_memoryRegions)
	{
		if(region.getFileOffset() <= offset && region.getFileOffset() + region.getSize() >= offset)
		{
			return offset - region.getFileOffset() + region.getStartPtr();
		}
	}
	return 0;
}

uint8_t* ElfMemoryFile64::getMemoryDataPointerFromOffset(uintptr_t offset) const
{
	int i = 0;
	for(const auto& region : m_memoryRegions)
	{
		if(region.getStartPtr() <= offset && region.getEndPtr() >= offset)
		{
			return &m_memoryData[i][offset - region.getStartPtr()];
		}
		++i;
	}
	return nullptr;
}

const ElfMemorySection64* ElfMemoryFile64::getSectionByName(
		const std::string& name) const
{
	for(auto& section : m_sections)
	{
		if(section.getName() == name)
		{
			return &section;
		}
	}
	return nullptr;
}

}
