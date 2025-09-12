import os
import sys

def create_component_folder(component_name):
    # Get the current directory
    current_dir = os.getcwd()
    
    # Define the path for the 'component' directory
    component_dir = os.path.join(current_dir, 'components')
    
    # Check if 'component' folder exists
    if not os.path.exists(component_dir):
        print("Error: 'components' folder not found in the current directory.")
        return
    
    # Create the new folder inside the 'component' folder
    new_folder_path = os.path.join(component_dir, component_name)
    
    # Check if the folder already exists
    if os.path.exists(new_folder_path):
        print(f"Error: Folder '{component_name}' already exists in the 'component' directory.")
        return
    
    # Create the new folder
    os.makedirs(new_folder_path)
    
    # Create the .c and .h files inside the new folder
    c_file_path = os.path.join(new_folder_path, f"{component_name}.c")
    h_file_path = os.path.join(new_folder_path, f"{component_name}.h")
    
    # Content to be written into .c and .h files
    c_file_content = f'#include "{component_name}.h"\n\nvoid {component_name}_init() {{\n    // Initialize {component_name} component\n}}\n'
    h_file_content = f'#ifndef {component_name.upper()}_H\n#define {component_name.upper()}_H\n\nvoid {component_name}_init();\n\n#endif // {component_name.upper()}_H\n'

    # Write to .c file
    with open(c_file_path, 'w') as c_file:
        c_file.write(c_file_content)
    
    # Write to .h file
    with open(h_file_path, 'w') as h_file:
        h_file.write(h_file_content)
    
    print(f"Component '{component_name}' created successfully with .c and .h files!")

    # Update the CMakeLists.txt file
    cmake_file_path = os.path.join(current_dir, 'CMakeLists.txt')
    if os.path.exists(cmake_file_path):
        with open(cmake_file_path, 'a') as cmake_file:
            cmake_file.write(f"\n# Add the component {component_name}\n")
            cmake_file.write(f"target_sources(app PRIVATE\n")
            cmake_file.write(f"    components/{component_name}/{component_name}.c)\n")
            cmake_file.write(f"target_include_directories(app\n")
            cmake_file.write(f"    PRIVATE\n")
            cmake_file.write(f"    ${{CMAKE_CURRENT_SOURCE_DIR}}/components/{component_name}\n")  # Corrected here
            cmake_file.write(f")\n")  # Closing bracket added
        print(f"CMakeLists.txt updated to include the component {component_name}")
    else:
        print("Error: CMakeLists.txt file not found in the current directory.")

if __name__ == "__main__":
    # Ensure there is exactly one command-line argument (folder name)
    if len(sys.argv) != 2:
        print("Usage: python create_component.py <component_name>")
        sys.exit(1)
    
    # Get the component name from the command-line argument
    component_name = sys.argv[1]
    
    # Call the function to create the folder and files
    create_component_folder(component_name)
