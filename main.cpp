#include <iostream>
#include <sstream>
#include <memory>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <cmath>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <dlfcn.h>
#include <elf.h>

#include "Process.h"
#include "ElfMemoryFile64.h"

void WriteHex(const uint8_t* data, int len)
{

	for(int i = 0 ; i < len; ++i)
	{
		std::cout << std::hex;
		if(i % 16 == 0)
		{
			if(i != 0)
			{
				std::cout << std::endl;
			}
			std::cout << "0x" << std::setw(std::ceil(std::log(len)/std::log(16.))) << std::setfill('0') << i << ": ";
		}
		std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
	}
	std::cout << std::endl;
}

std::unique_ptr<uint8_t []> ReadProcessData(pid_t pId, uintptr_t offset, size_t len)
{
	auto buffer = std::unique_ptr<uint8_t []>(new uint8_t[len]);

	for(uintptr_t off = offset; off < offset + len; off += sizeof(uintptr_t))
	{
		uintptr_t data = ptrace(PTRACE_PEEKDATA, pId, off, 0);
		unsigned int copyLen = len - (off - offset);
		unsigned int realCopyLen = copyLen > sizeof(uintptr_t) ? sizeof(uintptr_t) : copyLen;
		for(unsigned int i = 0; i < realCopyLen; ++i)
		{
			buffer[i + off - offset] = static_cast<unsigned char>(data >> (i * 8));
		}
	}

	return std::move(buffer);
}

void WriteProcessData(pid_t pId, const uint8_t* data, uintptr_t offset, size_t len)
{
	for(uintptr_t off = offset; off < offset + len; off += sizeof(uintptr_t))
	{
		unsigned int copyLen = len - (off - offset);
		unsigned int realCopyLen = copyLen > sizeof(uintptr_t) ? sizeof(uintptr_t) : copyLen;

		std::unique_ptr<uint8_t []> extraData;

		if(realCopyLen < sizeof(uintptr_t))
		{
			extraData = ReadProcessData(pId, off, sizeof(uintptr_t));
		}

		std::array<uint8_t, sizeof(uintptr_t)> writeData;
		for(unsigned int i = 0; i < sizeof(uintptr_t); ++i)
		{
			if(i < realCopyLen)
			{
				writeData[i] = data[off - offset + i];
			}
			else
			{
				writeData[i] = extraData[i];
			}
		}
		ptrace(PTRACE_POKEDATA, pId, off, reinterpret_cast<void*>(*reinterpret_cast<uintptr_t*>(writeData.data())));
	}
}

int main(int argc, char** argv)
{
	///Shellcode patch. This will load our library into the target process.
	uint8_t patch [] =
	{
			0x48, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8 byte offset to library name */ //movq offset, %rdi
			0xbe, 0x02, 0x01, 0x00, 0x00, //movl RLTD_GLOBAL, %rsi
			0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    //mov dlopen, %rax
			0xff, 0xd0, //callq  *%rax
			0xcc   //int 3
	};
	const int patchLen = 28;
	pid_t pId = 28167;

	if(argc != 3)
	{
		std::cerr << "No pId and injection library specified, aborting";
		return 0;
	}

	std::stringstream pIdStream;
	pIdStream << argv[1];
	pIdStream >> pId;

	std::string fileName = argv[2];

	//Attach to our process and send SIGSTOP, aborting if it fails
	if(ptrace(PTRACE_ATTACH, pId, 0, 0) != 0)
	{
		std::cerr << "Unable to attach to process " << pId << ", aborting." << std::endl;
		return 0;
	}

	Process process(pId);
	auto dlRegions = process.getLibraryRegions("/usr/lib/libdl");
	Elf::ElfMemoryFile64 libdl(dlRegions, process);

	std::cout << "Attached to process " << pId << std::endl;

	const int readLen = patchLen + fileName.length() + 1;

	//Wait for the process to stop executing so we can edit the code
	int waitStatus = 0;
	waitpid(pId, &waitStatus, __WALL);

	//Get the current processor register values
	user_regs_struct registers;
	ptrace(PTRACE_GETREGS, pId, nullptr, &registers);

	std::cout << "Process paused. Instruction pointer = " << std::hex << registers.rip << std::endl;

	//Write the library name address into the patch
	*reinterpret_cast<uintptr_t*>(&patch[2]) = registers.rip + patchLen;

	Elf::ElfExport64 dlOpenExport = libdl.getExport("dlopen");
	uintptr_t dlopenPtr = libdl.getMemoryPointerFromFileOffset(dlOpenExport.offset);

	//Write the address of dlopen into the patch
	*reinterpret_cast<uintptr_t*>(&patch[17]) = dlopenPtr;

	//First, read the original shellcode at the instruction pointer
	auto data = ReadProcessData(pId, registers.rip, readLen);
	//Replace the shellcode with our patch
	WriteProcessData(pId, patch, registers.rip, patchLen);
	WriteProcessData(pId, reinterpret_cast<const uint8_t*>(fileName.c_str()), registers.rip + patchLen, fileName.length() + 1);

	auto data2 = ReadProcessData(pId, registers.rip, readLen);

	std::cout << "Code injected successfully... executing stub." << std::endl;

	user_regs_struct registers2;
	ptrace(PTRACE_CONT, pId, 0, 0);
	waitpid(pId, &waitStatus, __WALL);
	ptrace(PTRACE_GETREGS, pId, nullptr, &registers2);


	std::cout << "Execution complete, instruction pointer = " << std::hex << registers2.rip << std::endl;
	std::cout << "dlopen return value " << registers2.rax << std::endl;
	if(registers2.rax == 0)
	{
		std::cout << "Injection failed!" << std::endl;
	}
	else
	{
		std::cout << "Injection success!" << std::endl;
	}

	std::cout << "Restoring old code..." << std::endl;


	//Replace the patched shellcode with the original code
	WriteProcessData(pId, data.get(), registers.rip, readLen);
	auto data3 = ReadProcessData(pId, registers.rip, readLen);

	//Replace the registers to their initial values
	ptrace(PTRACE_SETREGS, pId, nullptr, &registers);

	/*WriteHex(data.get(), readLen);
	std::cout << std::endl;
	WriteHex(data2.get(), readLen);
	std::cout << std::endl;
	WriteHex(data3.get(), readLen);*/

	std::cout << "Detatching..." << std::endl;

	ptrace(PTRACE_DETACH, pId, 0, 0);

	std::cout << "Done!" << std::endl;

	return 0;
}


