import os
import shutil
import csv
from pathlib import Path

def merge_instances_csv_files_multiple(*source_folders, destination_folder):
    """
    Merge CSV files from multiple folders into a destination folder.
    
    Args:
        *source_folders: Variable number of source folder paths
        destination_folder (str): Path to destination folder (keyword argument)
    
    Example usage:
        merge_instances_csv_files("folder_a", "folder_b", "folder_c", destination_folder="merged_output")
        merge_instances_csv_files("folder1", "folder2", "folder3", "folder4", destination_folder="result")
    """
    if not source_folders:
        print("Error: No source folders provided.")
        return
    
    print(f'Starting to merge CSV files from {len(source_folders)} folders: {list(source_folders)} to destination_folder: {destination_folder}')
    
    # Ensure destination folder exists and clear it
    if os.path.exists(destination_folder):
        shutil.rmtree(destination_folder)
    os.makedirs(destination_folder, exist_ok=True)
    
    # Get CSV files from all folders
    all_folder_files = {}
    all_unique_files = set()
    
    for i, folder in enumerate(source_folders):
        folder_files = set()
        if os.path.exists(folder):
            try:
                folder_files = {f for f in os.listdir(folder) if f.lower().endswith('.csv')}
                print(f"Found {len(folder_files)} CSV files in {folder}")
            except OSError as e:
                print(f"Error accessing folder {folder}: {e}")
                folder_files = set()
        else:
            print(f"Warning: Folder {folder} does not exist, skipping.")
        
        all_folder_files[folder] = folder_files
        all_unique_files.update(folder_files)
    
    if not all_unique_files:
        print("No CSV files found in any folder.")
        return
    
    print(f"Processing {len(all_unique_files)} unique CSV files...")
    
    for file_name in all_unique_files:
        # Find which folders contain this file
        folders_with_file = [folder for folder, files in all_folder_files.items() if file_name in files]
        dest_path = os.path.join(destination_folder, file_name)
        
        try:
            if len(folders_with_file) == 1:
                # File exists in only one folder, just copy it
                source_path = os.path.join(folders_with_file[0], file_name)
                shutil.copy2(source_path, dest_path)
                print(f"Copied from {folders_with_file[0]}: {file_name}")
            
            elif len(folders_with_file) > 1:
                # File exists in multiple folders, merge them
                source_paths = [os.path.join(folder, file_name) for folder in folders_with_file]
                merge_multiple_csv_files(source_paths, dest_path)
                print(f"Merged from {len(folders_with_file)} folders: {file_name}")
                
        except Exception as e:
            print(f"Error processing {file_name}: {e}")
            continue
    
    print("Multi-folder CSV merge operation completed.")

def merge_multiple_csv_files(source_file_paths, dest_path):
    """
    Merge multiple CSV files, keeping the header from the first file.
    
    Args:
        source_file_paths (list): List of source CSV file paths
        dest_path (str): Path to destination CSV file
    """
    if not source_file_paths:
        return
    
    try:
        with open(dest_path, 'w', newline='', encoding='utf-8') as dest_file:
            writer = csv.writer(dest_file)
            header_written = False
            
            for i, file_path in enumerate(source_file_paths):
                try:
                    with open(file_path, 'r', newline='', encoding='utf-8') as source_file:
                        reader = csv.reader(source_file)
                        
                        # Handle header
                        header = next(reader, None)
                        if header:
                            if not header_written:
                                writer.writerow(header)
                                header_written = True
                            # Skip header for subsequent files (assuming same structure)
                        
                        # Write data rows
                        for row in reader:
                            writer.writerow(row)
                            
                except Exception as e:
                    print(f"Error reading file {file_path}: {e}")
                    continue
                    
    except Exception as e:
        # If CSV parsing fails, fall back to line-by-line merge
        print(f"CSV parsing failed, using line-by-line merge: {e}")
        merge_multiple_files_line_by_line(source_file_paths, dest_path)

def merge_multiple_files_line_by_line(source_file_paths, dest_path):
    """
    Fallback method: merge multiple files line by line.
    """
    try:
        with open(dest_path, 'w', newline='', encoding='utf-8') as dest_file:
            for i, file_path in enumerate(source_file_paths):
                try:
                    with open(file_path, 'r', newline='', encoding='utf-8') as source_file:
                        lines = source_file.readlines()
                        
                        if i == 0:
                            # First file: write all lines
                            dest_file.writelines(lines)
                        else:
                            # Subsequent files: skip header (first line) if it exists
                            if len(lines) > 1:
                                dest_file.writelines(lines[1:])
                                
                except Exception as e:
                    print(f"Error reading file {file_path}: {e}")
                    continue
                    
    except Exception as e:
        print(f"Error writing merged file: {e}")

# Alternative version with explicit folder list parameter
def merge_csv_files_from_folder_list(source_folders, destination_folder):
    """
    Alternative version that takes a list of folders instead of variable arguments.
    
    Args:
        source_folders (list): List of source folder paths
        destination_folder (str): Path to destination folder
    
    Example usage:
        folders = ["folder_a", "folder_b", "folder_c", "folder_d"]
        merge_csv_files_from_folder_list(folders, "merged_output")
    """
    return merge_instances_csv_files_multiple(*source_folders, destination_folder=destination_folder)

# Example usage:
if __name__ == "__main__":
    # Method 1: Using variable arguments (recommended)
    
    geo_chemical_level0_trees_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\GeoChemical_Level_0_Trees"
    geo_chemical_level0_pois_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\GeoChemical_Level_0_POIs"
    geo_chemical_level0_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\GeoChemical_Level_0"
    geo_chemical_level1_trees_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\GeoChemical_Level_1_Trees"
    geo_chemical_level1_pois_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\GeoChemical_Level_1_POIs"   
    geo_chemical_level1_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\GeoChemical_Level_1"
    geo_chemical_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\GeoChemical"
    
    tree_instance_level0_trees_output_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\instanceoutput_level0\\Trees"
    tree_instance_level0_pois_output_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\instanceoutput_level0\\POIs"
    tree_instance_level1_trees_output_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\instanceoutput_level1\\Trees"
    tree_instance_level1_pois_output_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\instanceoutput_level1\\POIs"
    tree_instance_level0_merge_output_foler_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\instanceoutput_level0_Merged"
    tree_instance_level1_merge_output_foler_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\instanceoutput_level1_Merged"
    tree_instance_output_folder_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\instanceoutput"
    
    
    merge_instances_csv_files_multiple(geo_chemical_level0_trees_folder_path, geo_chemical_level0_pois_folder_path, destination_folder=geo_chemical_level0_folder_path)
    merge_instances_csv_files_multiple(geo_chemical_level1_trees_folder_path, geo_chemical_level1_pois_folder_path, destination_folder=geo_chemical_level1_folder_path)
    
    merge_instances_csv_files_multiple(tree_instance_level0_trees_output_folder_path, tree_instance_level0_pois_output_folder_path, tree_instance_level0_merge_output_foler_path)
    merge_instances_csv_files_multiple(tree_instance_level1_trees_output_folder_path, tree_instance_level1_pois_output_folder_path, tree_instance_level1_merge_output_foler_path)
    merge_instances_csv_files_multiple(tree_instance_level0_merge_output_foler_path, tree_instance_level1_merge_output_foler_path, tree_instance_output_folder_path)
    destination_path = f"D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\instanceoutput"
    
    merge_instances_csv_files_multiple(tree_instance_level0_trees_output_folder_path, tree_instance_level0_pois_output_folder_path, destination_folder=destination_path)
    
    #merge_instances_csv_files_multiple(folder_0_tree, folder_0_pois, folder_1_tree, destination_folder=destination_path)
    
    # Method 2: For 4 folders
    #merge_instances_csv_files_multiple(folder_0_tree, folder_0_pois, folder_1_tree, folder_1_pois, destination_folder=destination_path)
    
    # Method 3: Using list (alternative)
    folders = [tree_instance_level0_trees_output_folder_path, tree_instance_level0_pois_output_folder_path]
    #merge_csv_files_from_folder_list(folders, destination_path)