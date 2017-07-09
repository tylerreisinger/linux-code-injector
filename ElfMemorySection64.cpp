/*
 * ElfMemorySection64.cpp
 *
 *  Created on: Dec 8, 2013
 *      Author: tyler
 */

#include "ElfMemorySection64.h"

namespace Elf
{

ElfMemorySection64::ElfMemorySection64(Elf64_Shdr header,
		const std::string& name):
	m_header(header), m_name(name)
{
}

ElfMemorySection64::~ElfMemorySection64()
{
}

}
