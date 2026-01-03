#include "../executer/ExternalFunctions.h"

#include <cstring>

#ifdef WIN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace ffi {

Arguments::Arguments() : size{0}, buffer() { 
    clear(); 
}

void Arguments::addWord(word_t value) {
    ASSURE(size < capacity, "Too many arguments added to the buffer");

    arg_t& arg = buffer[size++];
    arg.type = arg_types::Word;

    char* dest = reinterpret_cast<char*>(&arg.value);
    char* src = reinterpret_cast<char*>(&value);
    std::memcpy(dest, src, sizeof(word_t));
}

void Arguments::addDWord(dword_t value) {
    ASSURE(size < capacity, "Too many arguments added to the buffer");
    
    arg_t& arg = buffer[size++];
    arg.type = arg_types::DWord;

    char* dest = reinterpret_cast<char*>(&arg.value);
    char* src = reinterpret_cast<char*>(&value);
    std::memcpy(dest, src, sizeof(dword_t));
}

void Arguments::addQWord(qword_t value) {
    ASSURE(size < capacity, "Too many arguments added to the buffer");
    
    arg_t& arg = buffer[size++];
    arg.type = arg_types::QWord;
    arg.value = value;
}

void Arguments::clear() {
    size = 0;
    for (size_t i = 0; i < capacity; ++i) {
        buffer[i] = {arg_types::None, 0}; // Reset to default
    }
}

#ifdef WIN // Windows

ExternalFunctions::ExternalFunctions() = default;

size_t ExternalFunctions::add(const std::string& library, const std::string& functionName) {
    ExternalFunction functionInfo;
    functionInfo.library = library;
    functionInfo.name = functionName;
    functionInfo.functionPtr = nullptr;

    std::string libPath = "bin\\lib" + library + ".dll";
    HINSTANCE libraryHandle = LoadLibrary(libPath.c_str());

    if (!libraryHandle) {
        std::cerr << "Error: Could not load library " << libPath << std::endl;
        throwConstraintViolated("Failed to load library");
    }

    // resolve function address here
    functionInfo.functionPtr = (void*)GetProcAddress(libraryHandle, functionInfo.name.c_str());
    if (!functionInfo.functionPtr) {
        std::cerr << "Error: Could not located the function " << functionInfo.name << " in " << libPath << std::endl;
        throwConstraintViolated("Failed to find symbol in library");
    }
    ASSURE_NOT_NULL(functionInfo.functionPtr);

    functions.push_back(functionInfo);
    return functions.size() - 1;
}

qword_t ExternalFunctions::call(size_t id, const Arguments& args) {
    ASSURE(id < functions.size(), "Function ID out of bounds");

    const ExternalFunction& func = functions[id];
    ASSURE_NOT_NULL(func.functionPtr);

    // TODO: We don't support more than 6 arguments yet (put on stack)
    ASSURE(args.getSize() <= 6, "Too many arguments for external function");

    // Windows 64 calling convention:
    // https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170
    // RCX, RDX, R8, and R9, 
    // shadow store allocated on stack for those four registers
    // then follow by the rest of the arguments on the stack
    // Floats go into XMM0 - XMM3

    // This is AT&T (AT&T syntax) assembly code for x64 Windows
    qword_t result;
    __asm__ volatile (
        "movq %[args_tag], %%R10\n" // Bring arg pointer into R10
        "movq %[size_tag], %%R11\n" // Bring arg size into R11
        "movq %%rsp, %%rbx\n" // Clear rbx, used for shadow space

        "cmpq $0, (%%R10)\n" // Check if type is None (0)
        "je label_do_call\n" // End of args

        "addq $8, %%R10\n" // Bring the pointer to the argument value
        "movq (%%R10), %%rcx\n" // Put into target register
        "add $8, %%R10\n" // Move to next argument

        "cmpq $0, (%%R10)\n" // Check if type is None (0)
        "je label_do_call\n" // End of args

        "addq $8, %%R10\n" // Bring the pointer to the argument value
        "movq (%%R10), %%rdx\n" // Put into target register
        "add $8, %%R10\n" // Move to next argument

        "cmpq $0, (%%R10)\n" // Check if type is None (0)
        "je label_do_call\n" // End of args

        "addq $8, %%R10\n" // Bring the pointer to the argument value
        "movq (%%R10), %%r8\n" // Put into target register
        "addq $8, %%R10\n" // Move to next argument

        "cmpq $0, (%%R10)\n" // Check if type is None (0)
        "je label_do_call\n" // End of args

        "addq $8, %%R10\n" // Bring the pointer to the argument value
        "movq (%%R10), %%r9\n" // Put into target register
        "addq $8, %%R10\n" // Move to next argument

        "cmpq $0, (%%R10)\n" // Check if type is None (0)
        "je label_do_call\n" // End of args

        // Allocate stack
        "subq $64, %%rsp\n"

        // Next argument (stack 32)
        "addq $8, %%R10\n"
        "movq (%%R10), %%r12\n"
        "movq %%r12, 32(%%rsp)\n"
        "addq $8, %%R10\n"

        "cmpq $0, (%%R10)\n"
        "je label_do_call\n"

        // Next argument (stack 40)
        "addq $8, %%R10\n"
        "movq (%%R10), %%r12\n"
        "movq %%r12, 40(%%rsp)\n"
        "addq $8, %%R10\n"

        // Do the call
        "label_do_call:"

        "call *%[fn_tag]\n"
        "movq %%rax, %[result_tag]\n"

        // Reset stack
        "movq %%rbx, %%rsp\n"

        : [result_tag] "=r"(result) 
        : [fn_tag] "r"(func.functionPtr), 
          [args_tag] "r"(args.getBuffer()),
          [size_tag] "r"(args.getSize()));

    if (args.returnType == ret_type::Bool){
        // Only look at the lowest byte for boolean result
        return static_cast<bool>(result & 0xFF);
    }

    // TODO: Handle other return types properly

    return result;
}

ExternalFunctions::~ExternalFunctions() = default;

#else // Linux

ExternalFunctions::ExternalFunctions() = default;

size_t ExternalFunctions::add(const std::string& library, const std::string& functionName, qword_t returnType) {
    ExternalFunction functionInfo;
    functionInfo.library = library;
    functionInfo.name = functionName;
    functionInfo.returnType = returnType;
    functionInfo.functionPtr = nullptr;

    void* libraryHandle = nullptr;
    if (auto it = libraries.find(library); it != libraries.end()) {
        libraryHandle = it->second;
    } else {
        libraryHandle = dlopen(("bin/lib" + library + ".so").c_str(), RTLD_LAZY);
        if (!libraryHandle) {
            std::cerr << "Error: " << dlerror() << std::endl;
            throwConstraintViolated("Failed to load library");
        }
        libraries[library] = libraryHandle;
    }

    ASSURE_NOT_NULL(libraryHandle);
    dlerror(); // Clear any existing error

    functionInfo.functionPtr = dlsym(libraryHandle, functionName.c_str());
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Error: " << dlsym_error << std::endl;
        throwConstraintViolated("Failed to find symbol in library");
    }
    ASSURE_NOT_NULL(functionInfo.functionPtr);

    functions.push_back(functionInfo);
    return functions.size() - 1;
}

__attribute__((optimize("-O0"))) qword_t ExternalFunctions::call(size_t id, const Arguments& args) {
    ASSURE(id < functions.size(), "Function ID out of bounds");

    const ExternalFunction& func = functions[id];
    ASSURE_NOT_NULL(func.functionPtr);

    // TODO: We don't support more than 6 arguments yet (put on stack)
    ASSURE(args.getSize() <= 6, "Too many arguments for external function");

    // TODO: We don't support float/double arguments yet (xmm registers)

    // https://github.com/tsoding/b/blob/main/src/codegen/fasm_x86_64.rs#L221
    // Floating point numbers are passed in xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7
    // Integer numbers are passed in rdi, rsi, rdx, rcx, r8, r9
    // The result is returned in rax
    // If we need more than 6 arguments, the rest are passed on the stack. (?)

    // This is ATT (AT&T syntax) assembly code for x86_64 Linux
    qword_t result = 0;
    const void* fn_ptr = func.functionPtr;
    const void* args_ptr = args.getBuffer();
    __asm__ volatile (
        "pushq %%rbp\n"
        "movq %%rsp, %%rbp\n"
        "andq $-16, %%rsp\n"

        // Use R11 as our base pointer to avoid clobbering R10 if the compiler used it
        "movq %[args_tag], %%r11\n"

        // Arg 1: Type is at 0(r11), Value is at 8(r11)
        "cmpq $0, 0(%%r11)\n"
        "je 1f\n"
        "movq 8(%%r11), %%rdi\n"

        // Arg 2: Type is at 16(r11), Value is at 24(r11)
        "cmpq $0, 16(%%r11)\n"
        "je 1f\n"
        "movq 24(%%r11), %%rsi\n"

        // Arg 3: Type is at 32(r11), Value is at 40(r11)
        "cmpq $0, 32(%%r11)\n"
        "je 1f\n"
        "movq 40(%%r11), %%rdx\n"

        // Arg 4: Type is at 48(r11), Value is at 56(r11)
        "cmpq $0, 48(%%r11)\n"
        "je 1f\n"
        "movq 56(%%r11), %%rcx\n"

        // Arg 5: Type is at 64(r11), Value is at 72(r11)
        "cmpq $0, 64(%%r11)\n"
        "je 1f\n"
        "movq 72(%%r11), %%r8\n"

        // Arg 6: Type is at 80(r11), Value is at 88(r11)
        "cmpq $0, 80(%%r11)\n"
        "je 1f\n"
        "movq 88(%%r11), %%r9\n"

    "1:\n"
        "xorl %%eax, %%eax\n" 
        "xorq %%rax, %%rax\n" 

        "call *%[fn_tag]\n"
        "movq %%rax, %[result_tag]\n"

        "movq %%rbp, %%rsp\n"
        "popq %%rbp\n"

        : [result_tag] "=r"(result) 
        : [fn_tag] "r"(fn_ptr), 
        [args_tag] "r"(args_ptr)
        : "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r11", "memory", "cc"
    );

    if (func.returnType == ret_type::Bool){
        // Only look at the lowest byte for boolean result
        return static_cast<bool>(result & 0xFF);
    }

    if (func.returnType == ret_type::Void) {
        return 0;
    }

    if (func.returnType == ret_type::Number) {
        int iresult = 0;
        std::memcpy(&iresult, &result, sizeof(int));
        return iresult;
    }

    /*if (func.returnType == ret_type::Float) {
        float fresult;
        std::memcpy(&fresult, &result, sizeof(float));
        return fresult;
    }*/
    // if (func.returnType == ret_type::Ptr)

    return result;
}

ExternalFunctions::~ExternalFunctions() {
    for (const auto& [libName, libHandle] : libraries) {
        if (libHandle) {
            dlclose(libHandle);
        }
    }
    libraries.clear();
}

#endif

} // namespace ffi