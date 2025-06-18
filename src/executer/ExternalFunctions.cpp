#include "../executer/ExternalFunctions.h"

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
    *reinterpret_cast<word_t*>(&arg.value) = value;
}

void Arguments::addDWord(dword_t value) {
    ASSURE(size < capacity, "Too many arguments added to the buffer");
    
    arg_t& arg = buffer[size++];
    arg.type = arg_types::DWord;
    *reinterpret_cast<dword_t*>(&arg.value) = value;
}

void Arguments::addQWord(qword_t value) {
    ASSURE(size < capacity, "Too many arguments added to the buffer");
    
    arg_t& arg = buffer[size++];
    arg.type = arg_types::QWord;
    *reinterpret_cast<qword_t*>(&arg.value) = value;
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

    return result;
}

ExternalFunctions::~ExternalFunctions() = default;

#else // Linux

ExternalFunctions::ExternalFunctions() = default;

size_t ExternalFunctions::add(const std::string& library, const std::string& functionName) {
    ExternalFunction functionInfo;
    functionInfo.library = library;
    functionInfo.name = functionName;
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

qword_t ExternalFunctions::call(size_t id, const Arguments& args) {
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

    qword_t result;
    __asm__ volatile (
        "movq %[args_tag], %%R10\n" // Bring arg pointer into R10

        "call bring_next_value_into_rax\n"
        "movq %%rax, %%rdi\n" // Move the first argument into rdi

        "cmpq $0, (%%R10)\n" // Check if type is None (0)
        "je label_do_call\n" // End of args

        "call bring_next_value_into_rax\n"
        "movq %%rax, %%rsi\n" // Move the first argument into rsi

        "cmpq $0, (%%R10)\n" // Check if type is None (0)
        "je label_do_call\n" // End of args

        "call bring_next_value_into_rax\n"
        "movq %%rax, %%rdx\n" // Move the first argument into rdx

        "cmpq $0, (%%R10)\n" // Check if type is None (0)
        "je label_do_call\n" // End of args

        "call bring_next_value_into_rax\n"
        "movq %%rax, %%rcx\n" // Move the first argument into rcx

        "cmpq $0, (%%R10)\n" // Check if type is None (0)
        "je label_do_call\n" // End of args

        "call bring_next_value_into_rax\n"
        "movq %%rax, %%r8\n" // Move the first argument into r8

        "cmpq $0, (%%R10)\n" // Check if type is None (0)
        "je label_do_call\n" // End of args

        "call bring_next_value_into_rax\n"
        "movq %%rax, %%r9\n" // Move the first argument into r9

        "jmp label_do_call\n" // Skip the function and call the method

        // Function bring_next_value_into_rax
        "bring_next_value_into_rax:\n"
        // These chacks would be nice to have here, but the call somehow segfaults then
        // "   cmpq $0, (%%R10)\n" // Check if type is None (0)
        // "   je label_do_call\n" // End of args
        "   add $8, %%R10\n" // Bring the pointer to the argument value
        "   movq (%%R10), %%rax\n" // Put into target register
        "   add $8, %%R10\n" // Move to next argument
        "   ret\n"
        // End of bring_next_value_into_rax

        "label_do_call:"
        "call *%[fn_tag]\n"
        "movq %%rax, %[result_tag]\n"

        : [result_tag] "=r"(result) 
        : [fn_tag] "r"(func.functionPtr), 
          [args_tag] "r"(args.getBuffer())) ;

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