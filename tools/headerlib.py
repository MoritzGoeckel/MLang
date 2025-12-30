import os

# g++ -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Werror -O0 -std=c++17 -fmax-errors=1 -DSINGLE_HEADER src/mains/Tests.cpp

def read_header_and_source_files(directory):
    header_files = []
    source_files = []
    
    for root, _, files in os.walk(directory):
        for file in files:
            path = os.path.join(root, file)
            content = ''

            is_header = file.endswith('.h') or file.endswith('.hpp')
            is_cpp = file.endswith('.cpp') or file.endswith('.c')

            if is_header or is_cpp:
                with open(path, 'r') as f:
                    content = f.read()

            # detect main
            if 'int main(' in content:
                print(f"Skipping main file: {file}")
                continue

            if is_header:
                header_files.append((path, content))
            elif is_cpp:
                source_files.append((path, content))
            elif file.endswith('.o') or file.endswith('.obj') or file.endswith('.exe'):
                continue
            else:
                print(f"Warning: Unrecognized file type: {file}")
    return header_files, source_files

def read_file_include_order(path, first_file):
    result = []
    visited = set()
    headers = []
    cpps = []
    stack = [first_file] # TODO remove stack

    for root, _, files in os.walk(directory):
        for file in files:
            path = os.path.join(root, file)
            if file.endswith('.h') or file.endswith('.hpp'):
                headers.append(path)
            if file.endswith('.cpp') or file.endswith('.c'):
                if not 'int main(' in open(path, 'r').read():
                    cpps.append(path)
                else:
                    print(f"Skipping main file: {file}")
    
    def postOrder(file):
        file = os.path.abspath(file)

        if file in visited:
            return

        visited.add(file)

        with open(file, 'r') as f:
            lines = f.readlines()

        for line in lines:
            stripped = line.strip()
            if stripped.startswith('#include "') and stripped.endswith('"'):
                include_file = stripped[len('#include "'): -1]
                include_path = os.path.join(os.path.dirname(file), include_file)
                if os.path.exists(include_path):
                    postOrder(include_path)

        result.append(file)
    
    while stack or headers or cpps:
        if stack:
            current_file = stack.pop()
            postOrder(current_file)
        elif headers:
            stack.append(headers.pop())
        elif cpps:
            stack.append(cpps.pop())
    
    return result


def concat_files(files):
    result = []
    for path in files:
        result.append(f"// File: {path}")
        with open(path, 'r') as f:
            for line in f:
                result.append(line.rstrip())
    return result

def comment_local_includes(lines):
    result = []
    for line in lines:
        stripped = line.strip()
        if stripped.startswith('#include "') and stripped.endswith('"'):
            result.append(f"// {line}  // Local include commented out")
        elif stripped == '#pragma once':
            result.append("// #pragma once  // Pragma once commented out")
        else:
            result.append(line)
    return result

def collect_includes(lines):
    # Can only do this if not in #ifdef or #ifndef blocks
    result = []
    content = []
    is_in_conditional_block = False
    for line in lines:
        stripped = line.strip()
        if stripped.startswith('#ifdef') or stripped.startswith('#ifndef'):
            is_in_conditional_block = True
        elif stripped.startswith('#endif'):
            is_in_conditional_block = False

        if stripped.startswith('#include') and not is_in_conditional_block:
            result.append(line)
        else:
            content.append(line)

    result = list(dict.fromkeys(result)) 
    result.extend(['', ''])
    result.extend(content)
    return result

def remove_comments(lines):
    return [line for line in lines if not line.strip().startswith('//')]

def remove_empty_lines(lines):
    return [line for line in lines if line.strip() != '']

if __name__ == "__main__":
    directory = "src" 
    result_file = "include/libmlang.h"

    os.makedirs("include", exist_ok=True)

    files = read_file_include_order(directory, "src/core/Mlang.cpp")
    print("Files to be processed in order:"  + '\n'.join(files))
    
    processed_code = concat_files(files)

    processed_code = comment_local_includes(processed_code)
    processed_code = collect_includes(processed_code)

    processed_code = remove_comments(processed_code)
    processed_code = remove_empty_lines(processed_code)

    processed_code.insert(0, '#pragma once')

    with open(result_file, 'w') as f:
        f.write('\n'.join(processed_code))

    print(f"Processed code written to {result_file}")