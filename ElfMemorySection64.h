/*
 * ElfMemorySection64.h
 *
 *  Created on: Dec 8, 2013
 *      Author: tyler
 */

#ifndef ELFMEMORYSECTION64_H_
#define ELFMEMORYSECTION64_H_

#include <string>

#include <elf.h>

namespace Elf
{

class ElfMemorySection64
{
public:
	ElfMemorySection64(Elf64_Shdr header, const std::string& name);
	~ElfMemorySection64();

	const std::string getName() const {return m_name;}

	Elf64_Shdr getHeader() const {return m_header;}

protected:
	Elf64_Shdr m_header;

	std::string m_name;
};

}

#endif /* ELFMEMORYSECTION64_H_ */
