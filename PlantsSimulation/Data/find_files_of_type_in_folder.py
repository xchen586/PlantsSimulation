import os
import glob

def find_files_of_type_in_folder(folder_path, file_extension):
    files = []
    # Recursively traverse through all subdirectories
    for root, _, files_list in os.walk(folder_path):
        # Use glob.glob to find files matching the pattern in each directory
        files.extend(glob.glob(os.path.join(root, f"*.{file_extension}")))
    
    return files



# Example usage:
folder_path = f'D:\\xWork\\VoxelFarm\\PlantsSimulation\\PlantsSimulation\\Data'
file_extension = "py"

txt_files = find_files_of_type_in_folder(folder_path, file_extension)

# Print the list of found .txt files
for file in txt_files:
    print(file)