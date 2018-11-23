#ifndef JIT_COMPILER_H
#define JIT_COMPILER_H

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <cstddef>
#include <Windows.h>


std::string to_hex(const std::byte* buffer, size_t size) {
    using namespace std;
    std::stringstream str;
    str.setf(std::ios_base::hex, std::ios::basefield);
    str.setf(std::ios_base::uppercase);
    str.fill('0');	//str.width(2);

    for (size_t i = 0; i<size; ++i) {
        str << "0x";
        str << std::setw(2) << (unsigned short)(std::byte)buffer[i];
        str << " ";
    }
    return str.str();
}

struct X86InstrVector
{
    void push_function_prolog()
    {
        push_back({ static_cast<std::byte>(0x55), 
					static_cast<std::byte>(0x8B), 
					static_cast<std::byte>(0xEC) });
    }

    void push_function_epilog()
    {
        push_back({ static_cast<std::byte>(0x5D), 
					static_cast<std::byte>(0xC3) });
    }

    void push_back(std::byte b)
    {
        code_vector.push_back(b);
    }

    void push_back(const std::vector<std::byte>& bytes)
    {
        std::copy(bytes.begin(), bytes.end(), std::back_inserter(code_vector));
    }

    void push_argument(void* argument)
    {
        push_back(static_cast<std::byte>(0xA1));
        push_back(get_address(argument));
    }

    void move_result_to(void* address)
    {
        push_back(static_cast<std::byte>(0xA3));
        push_back(get_address(address));
    }

    std::vector<std::byte> get_address(void* addr)
    {
        std::vector<std::byte> result;
        auto addrBytes = intToBytes(reinterpret_cast<int>(addr));
        std::copy(addrBytes.rbegin(), addrBytes.rend(), std::back_inserter(result));
        return result;
    }

    std::vector<std::byte> int_to_bytes(int value)
    {
        std::vector<std::byte> result;
        auto addrBytes = intToBytes(value);
        std::copy(addrBytes.rbegin(), addrBytes.rend(), std::back_inserter(result));
        return result;
    }

    void dump() const { std::cout << to_hex(&code_vector[0], code_vector.size()) << std::endl;}
    
    size_t size() const { return code_vector.size(); }

    std::vector<std::byte> instruction_vector() const { return code_vector; }

private:
    std::vector<std::byte> code_vector;
    
    std::vector<std::byte> intToBytes(int value)
    {
        std::vector<std::byte> result;
        result.push_back(static_cast<std::byte>(value >> 24));
        result.push_back(static_cast<std::byte>(value >> 16));
        result.push_back(static_cast<std::byte>(value >> 8));
        result.push_back(static_cast<std::byte>(value));
        return result;
    }
};

struct JitCompiler
{
    typedef int(*pfunc)(void);

    JitCompiler(const X86InstrVector& i_vector) :buf(nullptr), instr_vector(i_vector) 
    {
        size = instr_vector.size();
        buf = (std::byte*)VirtualAllocEx(GetCurrentProcess(), 0, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    }

    ~JitCompiler() { VirtualFreeEx(GetCurrentProcess(), buf, size, MEM_RELEASE); }

    pfunc compile()
    {
        memcpy(buf, &instr_vector.instruction_vector()[0], size);
        pfunc func = (pfunc)buf;
        return func;
    }

private:
    std::byte* buf;
    size_t size;
    
    const X86InstrVector& instr_vector;

    JitCompiler(const JitCompiler&);
    JitCompiler& operator=(const JitCompiler&);

};

#endif
