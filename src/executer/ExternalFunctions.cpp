#include "../executer/ExternalFunctions.h"
#include <dlfcn.h>

namespace ffi {

#ifdef WIN // Windows

ExternalFunctions::ExternalFunctions() = default;

size_t ExternalFunctions::add(const std::string& library, const std::string& functionName) {
    throwConstraintViolated("External functions are not supported on this platform");
}

qword_t ExternalFunctions::call(size_t id, const std::vector<void*>& args) {
    throwConstraintViolated("External functions are not supported on this platform");
    return 0;
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

    //void (*print)() = (void (*)())dlsym(handle, "print");
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

qword_t ExternalFunctions::call(size_t id, const std::vector<void*>& args) {
    ASSURE(id < functions.size(), "Function ID out of bounds");

    const ExternalFunction& func = functions[id];

    qword_t result;
    void* fn = func.functionPtr;
    __asm__ (
        "mov $5, %%rdi\n"
        "mov $10, %%rsi\n"
        "call *%[fn_tag]\n"
        "movq %%rax, %[result_tag]\n"
        : [result_tag] "=r"(result) : [fn_tag] "r"(fn)); // Call the function using inline assembly

    // https://github.com/tsoding/b/blob/main/src/codegen/fasm_x86_64.rs#L221
    // Floating point numbers are passed in xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7
    // Integer numbers are passed in rdi, rsi, rdx, rcx, r8, r9
    // The result is returned in rax
    // If we need more than 6 arguments, the rest are passed on the stack. (?)

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