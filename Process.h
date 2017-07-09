#ifndef PROCESS_H_
#define PROCESS_H_

#include <string>
#include <vector>
#include <memory>

#include "MemoryRegion.h"

class Process
{
public:
	Process(int pId);
	virtual ~Process();

	int getProcessId() const {return m_pId;}

	const std::vector<MemoryRegion>& memoryRegions() const {return m_regions;}
	int memoryRegionCount() const {return m_regions.size();}

	std::string getMappingFilePath() const;
	std::string getMemoryFilePath() const;

	std::unique_ptr<uint8_t []> readMemory(uintptr_t offset, size_t len) const;

	bool isLibraryLoaded(const std::string& partialName) const;

	std::vector<MemoryRegion> getLibraryRegions(const std::string& partialName) const;

protected:
	void loadMemoryRegions();

	int m_pId = 0;
	std::vector<MemoryRegion> m_regions;
};

#endif

