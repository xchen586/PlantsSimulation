import os
import shutil

def merge_instances_csv_files(folder_a, folder_b, destination_folder):
    # Ensure destination folder exists and clear it
    print(f'start to merge_instances_csv_files from folder_a : {folder_a} and folder_b : {folder_b} to destination_folder : {destination_folder}')
    if os.path.exists(destination_folder):
        shutil.rmtree(destination_folder)
    os.makedirs(destination_folder, exist_ok=True)
    
    files_a = set(os.listdir(folder_a))
    files_b = set(os.listdir(folder_b))
    
    all_files = files_a | files_b
    
    for file_name in all_files:
        path_a = os.path.join(folder_a, file_name)
        path_b = os.path.join(folder_b, file_name)
        dest_path = os.path.join(destination_folder, file_name)
        
        if file_name in files_a and file_name in files_b:
            # Merge files
            with open(path_a, 'r', newline='', encoding='utf-8') as file_a:
                reader_a = file_a.readlines()
            
            with open(path_b, 'r', newline='', encoding='utf-8') as file_b:
                reader_b = file_b.readlines()
            
            # Write merged content
            with open(dest_path, 'w', newline='', encoding='utf-8') as dest_file:
                dest_file.writelines(reader_a)  # Write all of file A
                dest_file.writelines(reader_b[1:])  # Append file B (skip header)
        
        elif file_name in files_a:
            shutil.copy(path_a, dest_path)
        
        elif file_name in files_b:
            shutil.copy(path_b, dest_path)
    
    print("merge_instances_csv_files completed successfully.")
    
    
tiles = 25
x = 8
y = 5
scrap_folder= f'D:\\Downloads\\XCTreeCreation'
Tree_Data_Folder_Name = 'Tree_Big_Creation'
Data_folder = os.path.join(scrap_folder, Tree_Data_Folder_Name)
Tree_output_folder = os.path.join(Data_folder, 'tree_output', f'{tiles}_{x}_{y}')
folder_a = os.path.join(Tree_output_folder, 'instanceoutput_level0')
folder_b = os.path.join(Tree_output_folder, 'instanceoutput_level1')
destination_folder = os.path.join(Tree_output_folder, 'instanceoutput')
merge_instances_csv_files(folder_a, folder_b, destination_folder)