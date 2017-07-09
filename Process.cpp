#include "Process.h"

#include <sstream>
#include <fstream>
#include <array>
#include <iostream>

#include <boost/regex.hpp>



Process::Process(int pId):
	m_pId(pId)
{
	loadMemoryRegions();
}

Process::~Process()
{

}

std::string Process::getMappingFilePath() const
{
	std::stringstream mapFileName;
	mapFileName << "/proc/" << m_pId << "/maps";
	return mapFileName.str();
}

std::string Process::getMemoryFilePath() const
{
	std::stringstream mapFileName;
	mapFileName << "/proc/" << m_pId << "/mem";
	return mapFileName.str();
}


void Process::loadMemoryRegions()
{
	std::array<char, 1024> line;

	boost::regex pattern("^([0-9a-zA-Z]+)-([0-9a-zA-Z]+) (r|-)(w|-)(x|-)(s|p) ([0-9a-zA-Z]+) [0-9a-fA-F]+:[0-9a-fA-F]+ [0-9]+[ \t]*(.*)");

	std::string mapFilePath = getMappingFilePath();
	std::ifstream fileReader(mapFilePath, std::ios::in);

	m_regions.clear();

	while(fileReader.getline(line.data(), 1024))
	{
		boost::cmatch match;

		if(!boost::regex_match(line.data(), match, pattern))
		{
			std::cout << "Error: " << line.data() << std::endl;
		}

		uintptr_t startAddr;
		uintptr_t endAddr;
		uintptr_t fileOffset;

		//Convert hex strings to integers
		std::stringstream convStream;
		convStream << std::hex << match[1] << " " << match[2] << " " << match[7];
		convStream >> startAddr >> endAddr >> fileOffset;

		Flags<MemoryRegion::ProtectionFlags> flags;

		if(match[3] == "r")
		{
			flags |= MemoryRegion::ProtectionFlags::Read;
		}
		if(match[4] == "w")
		{
			flags |= MemoryRegion::ProtectionFlags::Write;
		}
		if(match[5] == "x")
		{
			flags |= MemoryRegion::ProtectionFlags::Execute;
		}
		if(match[6] == "s")
		{
			flags |= MemoryRegion::ProtectionFlags::Shared;
		}

		MemoryRegion region(startAddr, endAddr, fileOffset, flags, match[8]);

		m_regions.push_back(region);
	}
}

std::unique_ptr<uint8_t []> Process::readMemory(uintptr_t offset, size_t len) const
{
	std::unique_ptr<uint8_t []> data = std::unique_ptr<uint8_t []>(new uint8_t[len]);
	std::ifstream inStream(getMemoryFilePath(), std::ios::in | std::ios::binary);
	inStream.seekg(offset);
	inStream.read(reinterpret_cast<char*>(data.get()), len);
	return data;
}

bool Process::isLibraryLoaded(const std::string& partialName) const
{
	boost::regex pattern("partialName.*\.so");

	for(const auto& region : m_regions)
	{
		boost::smatch match;

		if(boost::regex_match(region.getName(), match, pattern))
		{
			return true;
		}
	}
	return false;
}

std::vector<MemoryRegion> Process::getLibraryRegions(
		const std::string& partialName) const
{
	std::vector<MemoryRegion> regions;
	boost::regex pattern(partialName + ".*\\.so");

	for(const auto& region : m_regions)
	{
		boost::smatch match;

		if(boost::regex_match(region.getName(), match, pattern))
		{
			regions.push_back(region);
		}
	}
	return regions;
}
