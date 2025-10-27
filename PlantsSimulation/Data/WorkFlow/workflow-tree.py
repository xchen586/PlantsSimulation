from voxelfarm import workflow_lambda
from voxelfarm import voxelfarmclient

import os
import configparser

g_trigger_others = False

g_debug_generation = True

g_file_name_of_trigger_others = 'need_to_trigger_other_workflow.ini'
section_workflow = 'Workflow'
next_workflow_key = 'Next_Workflow'

info_ini_name = 'lambda_info.ini'
section_entity = 'Entity'

road_input_scale_width = 300
road_input_scale_height = 300
tile_size = 25
tile_x = 8
tile_y = 5
tile_scale = 1
level = 6
tree_lod =  8
forest_age = 15000
tree_iteration = 300
grid_delta =25
initial_density = 0.35
seed_density = 0.008
competition_factor = 0.7
growth_factor = 0.5
thinning_threshold = 1.0

pangea_next_project_id = '1D4CBBD1D957477E8CC3FF376FB87470'

g_available_workflows = ['WORKFLOW_TREE_GENERATION',
                         'TEST_WORKFLOW_TREE_GENERATION',
                         'WORKFLOW_BASEMESHES_GENERATION',
                         'WORKFLOW_CAVES_GENERATION',
                         'WORKFLOW_SMOOTH_LAYER_GENERATION',
                         'WORKFLOW_ONLY_SMOOTH_LAYER_GENERATION',
                         'WORKFLOW_ONLY_TREE_GENERATION'
                         'TEST_WORKFLOW_ONLY_TREE_GENERATION',
                         'WORKFLOW_ROAD_INPUT_GENERATION',
                         'WORKFLOW_ROAD_CHANGED_TREE_GENERATION',
                         'WORKFLOW_WHOLE_RESULT_GENERATION', 
                         'TEST_WORKFLOW_WHOLE_RESULT_GENERATION'
                         ]

def has_trigger_other_workflow_file(vf : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    has_file = False
    data_entity_id = request.raw_entity_id
    lambda_host.log(f'Check if entity {data_entity_id} has file {g_file_name_of_trigger_others}')   
    file_list = []
    folder_path = lambda_host.download_entity_files(data_entity_id)
    lambda_host.log(f'folder_path of data_entity_id {data_entity_id} attach files is {folder_path}')
    for filename in os.listdir(folder_path):
        lambda_host.log(f'{filename} is in the folder {folder_path} of entity {data_entity_id}')
        file_list.append(filename)
    if g_file_name_of_trigger_others in file_list:
        has_file = True
        lambda_host.log(f'Found file {g_file_name_of_trigger_others} in the folder {folder_path} of entity {data_entity_id}')
    else:
        lambda_host.log(f'No file {g_file_name_of_trigger_others} in the folder {folder_path} of entity {data_entity_id}')
        
    return has_file

def get_next_workflow_from_file(vf : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    data_entity_id = request.raw_entity_id
    file_list = []
    folder_path = lambda_host.download_entity_files(data_entity_id)
    lambda_host.log(f'folder_path of data_entity_id {data_entity_id} attach files is {folder_path}')
    for filename in os.listdir(folder_path):
        lambda_host.log(f'{filename} is in the folder {folder_path} of entity {data_entity_id}')
        file_list.append(filename)
    
    if g_file_name_of_trigger_others in file_list:
        ini_file_path =  os.path.join(folder_path, g_file_name_of_trigger_others)
        lambda_host.log(f'Found file {g_file_name_of_trigger_others} in the folder {folder_path} of entity {data_entity_id}, the full path is {ini_file_path}')
        config = configparser.ConfigParser()
        config.read(ini_file_path)
        if section_workflow in config:
            if next_workflow_key in config[section_workflow]:
                next_workflow = config[section_workflow][next_workflow_key]
                lambda_host.log(f'Get next workflow {next_workflow} from file {g_file_name_of_trigger_others}')
                if next_workflow in g_available_workflows:
                    return next_workflow
                else:
                    lambda_host.log(f'Next workflow {next_workflow} is not in available workflows list')
                    return None
            else:
                lambda_host.log(f'No {next_workflow_key} in section {section_workflow} of file {g_file_name_of_trigger_others}')
                return None
        else:
            lambda_host.log(f'No section {section_workflow} in file {g_file_name_of_trigger_others}')
            return None
    
def trigger_other_project_workflow(
        vf : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host, 
        trigged_product_id : str):
    
        new_trigger_product_version = common_trigger_new_product_version(vf, request, lambda_host, trigged_product_id, [])
        if new_trigger_product_version is None:
            lambda_host.log(f'Failed to trigger new product version for {trigged_product_id}')
            return {'success': False, 'error_info': f'Failed to trigger new product version for {trigged_product_id}'}
        else:
            lambda_host.log(f'product : {trigged_product_id} has new_trigger_product_version: {new_trigger_product_version}')
            return {'success': True, 'complete': True, 'error_info': 'None'}

def common_trigger_new_product_version(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host, 
        product_id : str,
        file_paths : list[str]):
    
    inputs = {  
        "comment": f'new product version of {product_id} is Triggered by version folder : {request.version_folder_id}',
    }
    
    lambda_host.log(f'inputs for create_product_version: {inputs}')
    lambda_host.log(f'Start to create_product_version for {product_id}')
    new_product_verion = lambda_host.create_product_version(request.project_id, product_id, inputs, file_paths)
    if new_product_verion == None:
        lambda_host.log(f'Failed to create_product_version for {product_id}')
        return None
    else:
        lambda_host.log(f'{product_id} new product version : {new_product_verion} is triggered!')
    
    return new_product_verion

def common_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host,
        lambda_name : str,
        test_tree_result: bool,
        need_update_road_generated_input_version_property: bool,
        need_update_smooth_layer_generated_input_version_property: bool,
        need_update_basemeshes_generated_input_version_property: bool,
        need_update_caves_generated_input_version_property: bool,
        need_update_dungeons_generated_input_version_property: bool,
        need_update_tree_program_generated_input_version_property: bool,
        only_load_road_from_file: bool,
        run_road_exe: bool,
        run_worldgen_road: bool,
        run_upload_smooth_layer: bool,
        run_make_basemeshes: bool,
        run_upload_basemeshes: bool,
        run_make_caves: bool,
        run_upload_caves: bool,
        run_make_tree_instances:bool,
        run_upload_tree_instances: bool,
        run_create_geochem_entity: bool,
        run_generate_road_input: bool
        ):
    request.properties['my_property'] = 'my_value'
    
    request.properties['need_update_road_generated_input_version_property'] = need_update_road_generated_input_version_property
    lambda_host.log(f'request.properties[need_update_road_generated_input_version_property]: {need_update_road_generated_input_version_property}')
    request.properties['need_update_smooth_layer_generated_input_version_property'] = need_update_smooth_layer_generated_input_version_property
    lambda_host.log(f'request.properties[need_update_smooth_layer_generated_input_version_property]: {need_update_smooth_layer_generated_input_version_property}')
    request.properties['need_update_basemeshes_generated_input_version_property'] = need_update_basemeshes_generated_input_version_property
    lambda_host.log(f'request.properties[need_update_basemeshes_generated_input_version_property]: {need_update_basemeshes_generated_input_version_property}')
    request.properties['need_update_caves_generated_input_version_property'] = need_update_caves_generated_input_version_property
    lambda_host.log(f'request.properties[need_update_caves_generated_input_version_property]: {need_update_caves_generated_input_version_property}')
    request.properties['need_update_dungeons_generated_input_version_property'] = need_update_dungeons_generated_input_version_property
    lambda_host.log(f'request.properties[need_update_dungeons_generated_input_version_property]: {need_update_dungeons_generated_input_version_property}')
    request.properties['need_update_tree_program_generated_input_version_property'] = need_update_tree_program_generated_input_version_property
    lambda_host.log(f'request.properties[need_update_tree_program_generated_input_version_property]: {need_update_tree_program_generated_input_version_property}')
    
    lambda_host.log(f'request.product_folder_id is {request.product_folder_id}')
    lambda_host.log(f'request.active_version_folder_id is {request.active_version_folder_id}')
    lambda_host.log(f'request.version_folder_id is {request.version_folder_id}')
    
    '''
    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id
    
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    '''
    pythoncode_active_version_property = request.get_product_property('PYTHON_CODE_FILES', 'raw_data')
    treelist_active_version_property = request.get_product_property('TREE_LIST_FILES', 'raw_data')
    roaddata_active_version_property = request.get_product_property('ROAD_DATA_FILES', 'raw_data')
    basemeshes_active_version_property = request.get_product_property('BASE_MESHES_FILES', 'raw_data')
    caves_dungeons_asset_active_version_property = request.get_product_property('CAVES_DUNGEONS_ASSET_FILES', 'raw_data')
    displacement_active_version_property = request.get_product_property('DISPLACEMENT_MAPS_FILES', 'raw_data')
    qtree_active_version_property = request.get_product_property('QUADTREE_FILES', 'raw_data')
    tools_active_version_property = request.get_product_property('TOOLS_FILES', 'raw_data')
    tileinfo_active_version_property = request.get_product_property('TILE_INFO', 'raw_data')
    
    road_generated_input_version_property = request.get_product_property('ROAD_GENARATED_INPUT_FILES', 'raw_data')
    smooth_layer_generated_input_version_property = request.get_product_property('SMOOTH_LAYER_GENARATED_INPUT_FILES', 'raw_data')
    basemeshes_generated_input_version_property = request.get_product_property('BASE_MESHES_GENARATED_INPUT_FILES', 'raw_data')
    caves_generated_input_version_property = request.get_product_property('CAVES_GENARATED_INPUT_FILES', 'raw_data')
    dungeons_generated_input_version_property = request.get_product_property('DUNGEONS_GENARATED_INPUT_FILES', 'raw_data')
    tree_program_generated_input_version_property = request.get_product_property('TREE_PROGRAM_GENARATED_INPUT_FILES', 'raw_data')
    
    #game_tree_entity_id_property = 'BE04D7A0D18142AE9D024C1A3FD50BED'  #game entity "Pangea Next Game Instances Entity"
    #if test_tree_result:
    #    game_tree_entity_id_property = "6387B1C0BBC24AC3A7830F27807657A7" # xuan test "Pangea Next Test Instances Entity"
        
    game_tree_entity_id_property = '546217C1DE184353AFD6F14EFAED35C9'  #game entity 12_4_2_Pangea_Next_Game_Instances_Entity_573
    if test_tree_result:
        game_tree_entity_id_property = "DB5EFD08F7AF4725BABBFDC74E2C5ED8" # xuan test entity 12_4_2_Pangea_Test_Game_Instances_Entity_573
        
    lambda_host.log(f'game_tree_entity_id_property: {game_tree_entity_id_property}')
        
    #game_tree_entity_id_property = 'C45A2A58F7CA4482939921388FAECD0C'  #game entity 12_4_2_Pangea_Next_Game_Instances_Entity_New_Regions
    #if test_tree_result:
    #    game_tree_entity_id_property = "3D832611AA9A4BC69ABEB34039AAFED2" # xuan test 12_4_2_Pangea_Next_Test_Instances_Entity_New_Regions

    workflow_output_version_folder_id_property = '68396F90F7CE48B4BA1412EA020ED92A' #Pangea Next -> Workflow Output -> Workflow BaseMeshes Output
    if request.version_folder_id != None:
        workflow_output_version_folder_id_property = request.version_folder_id
        lambda_host.log(f'Assign the output folder id as request.version_folder_id : {request.version_folder_id}')
    
    result = lambda_host.process_lambda_entity(
        workflow_request=request,
        name=f'{lambda_name} Process Lambda',
        inputs={
            'game_tree_entity_id_property': game_tree_entity_id_property,
            'workflow_output_version_folder_id_property': workflow_output_version_folder_id_property,
            'lambda_entity_id': request.raw_entity_id,
            'project_id': request.project_id,
            'pythoncode_active_version_property': pythoncode_active_version_property,
            'treelist_active_version_property': treelist_active_version_property,
            'roaddata_active_version_property': roaddata_active_version_property,
            'basemeshes_active_version_property': basemeshes_active_version_property,
            'caves_dungeons_asset_active_version_property': caves_dungeons_asset_active_version_property,
            'displacement_active_version_property': displacement_active_version_property,
            'qtree_active_version_property': qtree_active_version_property,
            'tools_active_version_property': tools_active_version_property,
            'tileinfo_active_version_property': tileinfo_active_version_property,
            'road_generated_input_version_property': road_generated_input_version_property,
            'smooth_layer_generated_input_version_property': smooth_layer_generated_input_version_property,
            'basemeshes_generated_input_version_property': basemeshes_generated_input_version_property,
            'caves_generated_input_version_property': caves_generated_input_version_property, 
            'dungeons_generated_input_version_property': dungeons_generated_input_version_property, 
            'tree_program_generated_input_version_property': tree_program_generated_input_version_property,
            'need_update_road_generated_input_version_property': need_update_road_generated_input_version_property,
            'need_update_smooth_layer_generated_input_version_property': need_update_smooth_layer_generated_input_version_property,
            'need_update_basemeshes_generated_input_version_property': need_update_basemeshes_generated_input_version_property,
            'need_update_caves_generated_input_version_property': need_update_caves_generated_input_version_property,
            'need_update_dungeons_generated_input_version_property': need_update_dungeons_generated_input_version_property,
            'need_update_tree_program_generated_input_version_property': need_update_tree_program_generated_input_version_property,
            'only_load_road_from_file': only_load_road_from_file,
            'run_road_exe': run_road_exe,
            'run_worldgen_road': run_worldgen_road,
            'run_upload_smooth_layer': run_upload_smooth_layer,
            'run_make_basemeshes': run_make_basemeshes,
            'run_upload_basemeshes': run_upload_basemeshes,
            'run_make_caves': run_make_caves,
            'run_upload_caves': run_upload_caves,
            'run_make_tree_instances':run_make_tree_instances,
            'run_upload_tree_instances': run_upload_tree_instances,
            'run_create_geochem_entity': run_create_geochem_entity,
            'run_generate_road_input': run_generate_road_input,
            'road_input_scale_width': road_input_scale_width,
            'road_input_scale_height': road_input_scale_height,
            'tile_size': tile_size,
            'tile_x': tile_x,
            'tile_y': tile_y,
            'tile_scale': tile_scale,
            'level' : level,
            'tree_lod': tree_lod,
            'forest_age': forest_age,
            'tree_iteration':tree_iteration,
            'grid_delta': grid_delta,
            'initial_density': initial_density,
            'seed_density': seed_density,
            'competition_factor': competition_factor,
            'growth_factor': growth_factor,
            'thinning_threshold': thinning_threshold
        },
        code='xc_cloud_tree_creation.py',
        files=['xc_cloud_tree_creation.py', 'xc_lambda-uploaddb.py'],
        update_type='msg')
    
    request.properties['process_lambda_host_id'] = result.id
    lambda_host.log(f'process_lambda_host_id is {result.id}')
    
    return result

def trigger_new_product_version_from_data_entity(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host, 
        data_entity_id : str,
        product_id : str):
    
    data_entity = vf_api.get_entity(data_entity_id)
    if data_entity is None:
        return {'success': True, 'complete': False, 'error_info': f'Failed to get data_entity {data_entity_id}'} 
    
    #project_id = data_entity['project_id'] // not work
    #lambda_host.log(f'get project_id : {project_id} from data_entity[project_id] property')
    project_id = request.project_id
    lambda_host.log(f'get project_id : {project_id} from request.project_id property')
    
    folder_path = lambda_host.download_entity_files(data_entity_id)
    lambda_host.log(f'folder_path of data_entity_id {data_entity_id} attach files is {folder_path}')
    file_paths = []
    # Get all files in the folder
    for filename in os.listdir(folder_path):
        full_path = os.path.join(folder_path, filename)
        #if os.path.isfile(full_path) and (filename.endswith(".raw") or filename.endswith(".csv")):  # Check if it's a file
        if os.path.isfile(full_path):  # Check if it's a file
            file_paths.append(full_path)
            lambda_host.log(f'The attach file of {product_id} is {full_path}')
    
    inputs = {
            "comment": f'new product version of {product_id} is Triggered by version folder : {request.version_folder_id} with data_entity_id : {data_entity_id}',
        }
    lambda_host.log(f'inputs for create_product_version: {inputs}')
    lambda_host.log(f'Start to create_product_version for {product_id}')
    new_product_verion = lambda_host.create_product_version(project_id, product_id, inputs, file_paths)
    if new_product_verion is None:
        lambda_host.log(f'Failed to create_product_version for {product_id}')
        return {'success': False, 'complete': False, 'error_info': f'Failed to create_product_version for {product_id}'}
    else:
        lambda_host.log(f'{product_id} new product version : {new_product_verion} is triggered!')
        return {'success': True, 'complete': True, 'error_info': 'None'}
    
def common_trigger_new_product_version_from_data_entity_handler(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start common_trigger_new_product_version_from_data_entity_handler data')
    need_update_road_generated_input_version_property = request.properties['need_update_road_generated_input_version_property']
    lambda_host.log(f'need_update_road_generated_input_version_property: {need_update_road_generated_input_version_property}')
    need_update_smooth_layer_generated_input_version_property = request.properties['need_update_smooth_layer_generated_input_version_property']
    lambda_host.log(f'need_update_smooth_layer_generated_input_version_property: {need_update_smooth_layer_generated_input_version_property}')
    need_update_basemeshes_generated_input_version_property = request.properties['need_update_basemeshes_generated_input_version_property']
    lambda_host.log(f'need_update_basemeshes_generated_input_version_property: {need_update_basemeshes_generated_input_version_property}')
    need_update_caves_generated_input_version_property = request.properties['need_update_caves_generated_input_version_property']
    lambda_host.log(f'need_update_caves_generated_input_version_property: {need_update_caves_generated_input_version_property}')
    need_update_dungeons_generated_input_version_property = request.properties['need_update_dungeons_generated_input_version_property']
    lambda_host.log(f'need_update_dungeons_generated_input_version_property: {need_update_dungeons_generated_input_version_property}')
    need_update_tree_program_generated_input_version_property = request.properties['need_update_tree_program_generated_input_version_property']
    lambda_host.log(f'need_update_tree_program_generated_input_version_property: {need_update_tree_program_generated_input_version_property}')
    
    if not 'process_lambda_host_id' in request.properties:
        lambda_host.log(f'No process_lambda_host_id in request properties, skip trigger new product version')
        return {'success': False, 'complete': True, 'error_info': 'No process_lambda_host_id in request properties, skip trigger new product version'}
    
    process_lambda_host_id = request.properties['process_lambda_host_id']   
    lambda_host.log(f'process_lambda_host_id from request properties: {process_lambda_host_id}')
    
    process_lambda_host_entity = vf_api.get_entity(process_lambda_host_id)
    lambda_host.log(f'process_lambda_host_entity content is  : {process_lambda_host_entity}')
    
    property_prefix = 'property_'
    
    allGood = True
    error_msg = ''
    if need_update_road_generated_input_version_property:
        property_road_generated_input_entity_id = property_prefix + 'road_generated_input_entity_id'
        lambda_host.log(f'property_road_generated_input_entity_id property name is : {property_road_generated_input_entity_id}')
        if property_road_generated_input_entity_id in process_lambda_host_entity:
            road_generated_input_entity_id = process_lambda_host_entity[property_road_generated_input_entity_id]
            lambda_host.log(f'{property_road_generated_input_entity_id} from request properties: {road_generated_input_entity_id}')
            road_result = trigger_new_product_version_from_data_entity(vf_api, request, lambda_host, road_generated_input_entity_id, 'ROAD_GENARATED_INPUT_FILES')
            allGood = allGood and road_result['success']
            error_msg = error_msg + '\n' + road_result['error_info']
            if road_result['success'] == True:
                lambda_host.log(f'Successfully trigger new product version for ROAD_GENARATED_INPUT_FILES from entity {road_generated_input_entity_id}')
            else:
                road_error_msg = f'Failed to trigger new product version for ROAD_GENARATED_INPUT_FILES from entity {road_generated_input_entity_id}'
                lambda_host.log(road_error_msg)    
                error_msg = error_msg + '\n' + road_error_msg
        else:
            lambda_host.log(f'No {property_road_generated_input_entity_id} found in the process_lambda_host_entity')
            
    if need_update_smooth_layer_generated_input_version_property:
        property_smooth_layer_generated_input_entity_id = property_prefix + 'smooth_layer_generated_input_entity_id'
        lambda_host.log(f'property_smooth_layer_generated_input_entity_id property name is : {property_smooth_layer_generated_input_entity_id}')
        if property_smooth_layer_generated_input_entity_id in process_lambda_host_entity:
            smooth_layer_generated_input_entity_id = process_lambda_host_entity[property_smooth_layer_generated_input_entity_id]
            lambda_host.log(f'{property_smooth_layer_generated_input_entity_id} from request properties: {smooth_layer_generated_input_entity_id}')
            smooth_layer_result = trigger_new_product_version_from_data_entity(vf_api, request, lambda_host, smooth_layer_generated_input_entity_id, 'SMOOTH_LAYER_GENARATED_INPUT_FILES')
            allGood = allGood and smooth_layer_result['success']
            error_msg = error_msg + '\n' + smooth_layer_result['error_info']
            if smooth_layer_result['success'] == True:
                lambda_host.log(f'Successfully trigger new product version for SMOOTH_LAYER_GENARATED_INPUT_FILES from entity {smooth_layer_generated_input_entity_id}')
            else:
                smooth_layer_error_msg = f'Failed to trigger new product version for SMOOTH_LAYER_GENARATED_INPUT_FILES from entity {smooth_layer_generated_input_entity_id}'
                lambda_host.log(smooth_layer_error_msg)    
                error_msg = error_msg + '\n' + smooth_layer_error_msg
        else:
            lambda_host.log(f'No {property_smooth_layer_generated_input_entity_id} found in the process_lambda_host_entity')
            
    if need_update_basemeshes_generated_input_version_property:
        property_basemeshes_generated_input_entity_id = property_prefix + 'basemeshes_generated_input_entity_id'
        lambda_host.log(f'property_basemeshes_generated_input_entity_id property name is : {property_basemeshes_generated_input_entity_id}')
        if property_basemeshes_generated_input_entity_id in process_lambda_host_entity:
            basemeshes_generated_input_entity_id = process_lambda_host_entity[property_basemeshes_generated_input_entity_id]
            lambda_host.log(f'{property_basemeshes_generated_input_entity_id} from request properties: {basemeshes_generated_input_entity_id}')
            basemeshes_result = trigger_new_product_version_from_data_entity(vf_api, request, lambda_host, basemeshes_generated_input_entity_id, 'BASEMESHES_GENARATED_INPUT_FILES')
            allGood = allGood and basemeshes_result['success']
            error_msg = error_msg + '\n' + basemeshes_result['error_info']
            if basemeshes_result['success'] == True:
                lambda_host.log(f'Successfully trigger new product version for BASEMESHES_GENARATED_INPUT_FILES from entity {basemeshes_generated_input_entity_id}')
            else:
                basemeshes_error_msg = f'Failed to trigger new product version for BASEMESHES_GENARATED_INPUT_FILES from entity {basemeshes_generated_input_entity_id}'
                lambda_host.log(basemeshes_error_msg)    
                error_msg = error_msg + '\n' + basemeshes_error_msg
        else:
            lambda_host.log(f'No {property_basemeshes_generated_input_entity_id} found in the process_lambda_host_entity')
        
    if need_update_caves_generated_input_version_property:
        property_caves_generated_input_entity_id = property_prefix + 'caves_generated_input_entity_id'
        lambda_host.log(f'property_caves_generated_input_entity_id property name is : {property_caves_generated_input_entity_id}')
        if property_caves_generated_input_entity_id in process_lambda_host_entity:
            caves_generated_input_entity_id = process_lambda_host_entity[property_caves_generated_input_entity_id]
            lambda_host.log(f'{property_caves_generated_input_entity_id} from request properties: {caves_generated_input_entity_id}')
            caves_result = trigger_new_product_version_from_data_entity(vf_api, request, lambda_host, caves_generated_input_entity_id, 'CAVES_GENARATED_INPUT_FILES')
            allGood = allGood and caves_result['success']
            error_msg = error_msg + '\n' + caves_result['error_info']
            if caves_result['success'] == True:
                lambda_host.log(f'Successfully trigger new product version for CAVES_GENARATED_INPUT_FILES from entity {caves_generated_input_entity_id}')
            else:
                caves_error_msg = f'Failed to trigger new product version for CAVES_GENARATED_INPUT_FILES from entity {caves_generated_input_entity_id}'
                lambda_host.log(caves_error_msg)    
                error_msg = error_msg + '\n' + caves_error_msg
        else:
            lambda_host.log(f'No {property_caves_generated_input_entity_id} found in the process_lambda_host_entity')
            
    if need_update_dungeons_generated_input_version_property:
        property_dungeons_generated_input_entity_id = property_prefix + 'dungeons_generated_input_entity_id'
        lambda_host.log(f'property_dungeons_generated_input_entity_id property name is : {property_dungeons_generated_input_entity_id}')
        if property_dungeons_generated_input_entity_id in process_lambda_host_entity:
            dungeons_generated_input_entity_id = process_lambda_host_entity[property_dungeons_generated_input_entity_id]
            lambda_host.log(f'{property_dungeons_generated_input_entity_id} from request properties: {dungeons_generated_input_entity_id}')
            dungeons_result = trigger_new_product_version_from_data_entity(vf_api, request, lambda_host, dungeons_generated_input_entity_id, 'DUNGEONS_GENARATED_INPUT_FILES')
            allGood = allGood and dungeons_result['success']
            error_msg = error_msg + '\n' + dungeons_result['error_info']
            if dungeons_result['success'] == True:
                lambda_host.log(f'Successfully trigger new product version for DUNGEONS_GENARATED_INPUT_FILES from entity {dungeons_generated_input_entity_id}')
            else:
                dungeons_error_msg = f'Failed to trigger new product version for DUNGEONS_GENARATED_INPUT_FILES from entity {dungeons_generated_input_entity_id}'
                lambda_host.log(dungeons_error_msg)    
                error_msg = error_msg + '\n' + dungeons_error_msg
        else:
            lambda_host.log(f'No {property_dungeons_generated_input_entity_id} found in the process_lambda_host_entity')
            
    if need_update_tree_program_generated_input_version_property:
        property_tree_program_generated_input_entity_id = property_prefix + 'tree_program_generated_input_entity_id'
        lambda_host.log(f'property_tree_program_generated_input_entity_id property name is : {property_tree_program_generated_input_entity_id}')
        if property_tree_program_generated_input_entity_id in process_lambda_host_entity:
            tree_program_generated_input_entity_id = process_lambda_host_entity[property_tree_program_generated_input_entity_id]
            lambda_host.log(f'{property_tree_program_generated_input_entity_id} from request properties: {tree_program_generated_input_entity_id}')
            tree_program_result = trigger_new_product_version_from_data_entity(vf_api, request, lambda_host, tree_program_generated_input_entity_id, 'TREE_PROGRAM_GENARATED_INPUT_FILES')
            allGood = allGood and tree_program_result['success']
            error_msg = error_msg + '\n' + tree_program_result['error_info']
            if tree_program_result['success'] == True:
                lambda_host.log(f'Successfully trigger new product version for TREE_PROGRAM_GENARATED_INPUT_FILES from entity {tree_program_generated_input_entity_id}')
            else:
                tree_program_error_msg = f'Failed to trigger new product version for TREE_PROGRAM_GENARATED_INPUT_FILES from entity {tree_program_generated_input_entity_id}'
                lambda_host.log(tree_program_error_msg)    
                error_msg = error_msg + '\n' + tree_program_error_msg
        else:
            lambda_host.log(f'No {property_tree_program_generated_input_entity_id} found in the process_lambda_host_entity')
        
    return {'success': allGood, 'complete': True, 'error_info': error_msg}
    
def create_view_for_basemesh_entity(vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('start create_view_for_basemesh_entity')
    
    entity_id = request.raw_entity_id
    
    ini_file = lambda_host.download_entity_file(info_ini_name, entity_id)
    lambda_host.log(f'down load file {ini_file} with file name {info_ini_name} from entity {entity_id}')
    if not os.path.isfile(ini_file):
        lambda_host.log(f'{ini_file} is not a file')
        lambda_host.log(f'Fail to create_view_for_basemesh_entity')
        return
    
    config = configparser.ConfigParser()
    config.read(ini_file)
    
    # Check if the section exists
    if section_entity in config:
        # Loop through the section and print each key and value
        for key in config[section_entity]:
            value = config[section_entity][key]
            lambda_host.log(f'Basemesh entity name : {key}')
            lambda_host.log(f'Basemesh entity id : {value}')
            
            view_name = f'View for {key}'
            lambda_host.log(f'Creating View {view_name} ...') 
            lambda_host.log(f'view_type : com.voxelfarm.program.view.terrain , e : {value}')
            try:
                lambda_host.create_view(request, view_name, 'com.voxelfarm.program.view.terrain', None , {'e' : f'{value}'}, {})
            except Exception as e:
                error_message = str(e)
                repr_message = repr(e)
                lambda_host.log(f'Exception of create view {view_name} with entity id {value} with exception error_message of {error_message}')
                lambda_host.log(f'Exception of create view {view_name} with entity id {value} with exception repr_message of {repr_message}')
            lambda_host.log(f'Created View {view_name}...') 
    else:
        lambda_host.log(f'Section {section_entity} not found in the ini file {ini_file}')
        
    lambda_host.log(f'end create_view_for_basemesh_entity')
    
def python_code_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received Python Code')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating Python code raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Python Code Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

    return {'success': True, 'complete': True, 'error_info': 'None'}

def tile_info_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received Tile Info File')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating Tile Info File raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Tile Info File Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    triggerOthers = has_trigger_other_workflow_file(vf, request, lambda_host)
    if triggerOthers:
        lambda_host.log('Triggering others')
        trigged_product_id = 'WORKFLOW_WHOLE_RESULT_GENERATION'
        if g_debug_generation:
            trigged_product_id = 'TEST_WORKFLOW_WHOLE_RESULT_GENERATION'
            lambda_host.log(f'g_debug_generation is {g_debug_generation}, use {trigged_product_id} to trigger')
        next_workflow = get_next_workflow_from_file(vf, request, lambda_host)
        if next_workflow is not None:
            trigged_product_id = next_workflow
            lambda_host.log(f'next_workflow from file is {next_workflow}, use {trigged_product_id} to trigger')
        ret = trigger_other_project_workflow(vf, request, lambda_host, trigged_product_id)
        return ret

    return {'success': True, 'complete': True, 'error_info': 'None'}

def tree_list_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received Tree list')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating Tree list raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Tree list Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    triggerOthers = has_trigger_other_workflow_file(vf, request, lambda_host)
    if triggerOthers:
        lambda_host.log('Triggering others')
        trigged_product_id = 'WORKFLOW_ONLY_TREE_GENERATION'
        if g_debug_generation:
            trigged_product_id = 'TEST_WORKFLOW_ONLY_TREE_GENERATION'
            lambda_host.log(f'g_debug_generation is {g_debug_generation}, use {trigged_product_id} to trigger')
        next_workflow = get_next_workflow_from_file(vf, request, lambda_host)
        if next_workflow is not None:
            trigged_product_id = next_workflow
            lambda_host.log(f'next_workflow from file is {next_workflow}, use {trigged_product_id} to trigger')
        ret = trigger_other_project_workflow(vf, request, lambda_host, trigged_product_id)
        return ret

    return {'success': True, 'complete': True, 'error_info': 'None'}

def road_data_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received road data')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id
    
    lambda_host.log('Updating road data raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Road Data Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    triggerOthers = has_trigger_other_workflow_file(vf, request, lambda_host)
    if triggerOthers:
        lambda_host.log('Triggering others')
        trigged_product_id = 'WORKFLOW_ROAD_CHANGED_TREE_GENERATION'
        next_workflow = get_next_workflow_from_file(vf, request, lambda_host)
        if next_workflow is not None:
            trigged_product_id = next_workflow
            lambda_host.log(f'next_workflow from file is {next_workflow}, use {trigged_product_id} to trigger')
        ret = trigger_other_project_workflow(vf, request, lambda_host, trigged_product_id)
        return ret

    return {'success': True, 'complete': True, 'error_info': 'None'}

def base_meshes_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received base meshes')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating base meshes raw entity...') 
    
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Base Meshes Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    triggerOthers = has_trigger_other_workflow_file(vf, request, lambda_host)
    if triggerOthers:
        lambda_host.log('Triggering others')
        trigged_product_id = 'WORKFLOW_BASEMESHES_GENERATION'
        next_workflow = get_next_workflow_from_file(vf, request, lambda_host)
        if next_workflow is not None:
            trigged_product_id = next_workflow
            lambda_host.log(f'next_workflow from file is {next_workflow}, use {trigged_product_id} to trigger')
        ret = trigger_other_project_workflow(vf, request, lambda_host, trigged_product_id)
        return ret
    
    return {'success': True, 'complete': True, 'error_info': 'None'}

def caves_dungeons_asset_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received caves and dungeons asset files')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating caves and dungeons asset raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : ' Caves and dungeons asset files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    triggerOthers = has_trigger_other_workflow_file(vf, request, lambda_host)
    if triggerOthers:
        lambda_host.log('Triggering others')
        trigged_product_id = 'WORKFLOW_CAVES_GENERATION'
        next_workflow = get_next_workflow_from_file(vf, request, lambda_host)
        if next_workflow is not None:
            trigged_product_id = next_workflow
            lambda_host.log(f'next_workflow from file is {next_workflow}, use {trigged_product_id} to trigger')
        ret = trigger_other_project_workflow(vf, request, lambda_host, trigged_product_id)
        return ret
    
    return {'success': True, 'complete': True, 'error_info': 'None'}

def displacement_maps_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received displacement maps')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating displacement maps raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Displacement Maps Input Files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

    triggerOthers = has_trigger_other_workflow_file(vf, request, lambda_host)
    if triggerOthers:
        lambda_host.log('Triggering others')
        trigged_product_id = 'WORKFLOW_ONLY_SMOOTH_LAYER_GENERATION' # WORKFLOW_SMOOTH_LAYER_GENERATION
        next_workflow = get_next_workflow_from_file(vf, request, lambda_host)
        if next_workflow is not None:
            trigged_product_id = next_workflow
            lambda_host.log(f'next_workflow from file is {next_workflow}, use {trigged_product_id} to trigger')
        ret = trigger_other_project_workflow(vf, request, lambda_host, trigged_product_id)
        return ret
        
    return {'success': True, 'complete': True, 'error_info': 'None'}

def quadtree_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received quadtree')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating quadtree raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Quad Tree Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    triggerOthers = has_trigger_other_workflow_file(vf, request, lambda_host)
    if triggerOthers:
        lambda_host.log('Triggering others')
        trigged_product_id = 'WORKFLOW_WHOLE_RESULT_GENERATION'
        if g_debug_generation:
            trigged_product_id = 'TEST_WORKFLOW_WHOLE_RESULT_GENERATION'
            lambda_host.log(f'g_debug_generation is {g_debug_generation}, use {trigged_product_id} to trigger')
            next_workflow = get_next_workflow_from_file(vf, request, lambda_host)
        if next_workflow is not None:
            trigged_product_id = next_workflow
            lambda_host.log(f'next_workflow from file is {next_workflow}, use {trigged_product_id} to trigger')
        ret = trigger_other_project_workflow(vf, request, lambda_host, trigged_product_id)
        return ret
    
    return {'success': True, 'complete': True, 'error_info': 'None'}

def road_generated_input_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received road generated input')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating road generated input raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Road Generated Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    return {'success': True, 'complete': True, 'error_info': 'None'}

def smooth_layer_generated_input_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received smooth layer generated input')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating smooth layer generated input raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Smooth Layer Generated Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    triggerOthers = has_trigger_other_workflow_file(vf, request, lambda_host)
    if triggerOthers:
        lambda_host.log('Triggering others')
        trigged_product_id = 'WORKFLOW_ONLY_TREE_GENERATION'
        if g_debug_generation:
            trigged_product_id = 'TEST_WORKFLOW_ONLY_TREE_GENERATION'
            lambda_host.log(f'g_debug_generation is {g_debug_generation}, use {trigged_product_id} to trigger')
        next_workflow = get_next_workflow_from_file(vf, request, lambda_host)
        if next_workflow is not None:
            trigged_product_id = next_workflow
            lambda_host.log(f'next_workflow from file is {next_workflow}, use {trigged_product_id} to trigger')
        ret = trigger_other_project_workflow(vf, request, lambda_host, trigged_product_id)
        return ret
    
    return {'success': True, 'complete': True, 'error_info': 'None'}

def basemeshes_generated_input_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received basemeshes generated input')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating basemeshes generated input raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Basemeshes Generated Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    return {'success': True, 'complete': True, 'error_info': 'None'}

def caves_generated_input_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received caves generated input')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating caves generated input raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Caves Generated Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    triggerOthers = has_trigger_other_workflow_file(vf, request, lambda_host)
    if triggerOthers:
        lambda_host.log('Triggering others')
        trigged_product_id = 'WORKFLOW_ROAD_CHANGED_TREE_GENERATION'
        next_workflow = get_next_workflow_from_file(vf, request, lambda_host)
        if next_workflow is not None:
            trigged_product_id = next_workflow
            lambda_host.log(f'next_workflow from file is {next_workflow}, use {trigged_product_id} to trigger')
        ret = trigger_other_project_workflow(vf, request, lambda_host, trigged_product_id)
        return ret
    
    return {'success': True, 'complete': True, 'error_info': 'None'}

def dungeons_generated_input_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received dungeons generated input')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating dungeons generated input raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Dungeons Generated Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    triggerOthers = has_trigger_other_workflow_file(vf, request, lambda_host)
    if triggerOthers:
        lambda_host.log('Triggering others')
        trigged_product_id = 'WORKFLOW_ONLY_TREE_GENERATION'
        if g_debug_generation:
            trigged_product_id = 'TEST_WORKFLOW_ONLY_TREE_GENERATION'
            lambda_host.log(f'g_debug_generation is {g_debug_generation}, use {trigged_product_id} to trigger')
        next_workflow = get_next_workflow_from_file(vf, request, lambda_host)
        if next_workflow is not None:
            trigged_product_id = next_workflow
            lambda_host.log(f'next_workflow from file is {next_workflow}, use {trigged_product_id} to trigger')
        ret = trigger_other_project_workflow(vf, request, lambda_host, trigged_product_id)
        return ret
    
    return {'success': True, 'complete': True, 'error_info': 'None'}

def tree_program_generated_input_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received tree program generated input')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating tree program generated input raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Tree program Generated Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    triggerOthers = has_trigger_other_workflow_file(vf, request, lambda_host)
    if triggerOthers:
        lambda_host.log('Triggering others')
        trigged_product_id = 'WORKFLOW_ROAD_CHANGED_TREE_GENERATION'
        ret = trigger_other_project_workflow(vf, request, lambda_host, trigged_product_id)
        return ret
    
    return {'success': True, 'complete': True, 'error_info': 'None'}

def tools_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received tools')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating tools raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Tools Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

    return {'success': True, 'complete': True, 'error_info': 'None'}


def tree_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received tree generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name="Tree Generation"
                                             , test_tree_result=False
                                             , need_update_road_generated_input_version_property=True
                                             , need_update_smooth_layer_generated_input_version_property=True   
                                             , need_update_basemeshes_generated_input_version_property=True
                                             , need_update_caves_generated_input_version_property=True
                                             , need_update_dungeons_generated_input_version_property=True
                                             , need_update_tree_program_generated_input_version_property=False
                                             , only_load_road_from_file=False
                                             , run_road_exe=True
                                             , run_worldgen_road=True
                                             , run_upload_smooth_layer=True
                                             , run_make_basemeshes=True
                                             , run_upload_basemeshes=False
                                             , run_make_caves=True
                                             , run_upload_caves=False
                                             , run_make_tree_instances=True
                                             , run_upload_tree_instances=True
                                             , run_create_geochem_entity=True
                                             , run_generate_road_input=False)

    return {'success': result.success, 'complete': False, 'error_info': ''}

def tree_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start tree_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    if update_type == 'msg':
        lambda_host.log('Tree generation stage complete')
        common_trigger_new_product_version_from_data_entity_handler(vf_api, request, lambda_host)
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def test_tree_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received test tree generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name="Test Tree Generation"
                                             , test_tree_result=True
                                             , need_update_road_generated_input_version_property=True
                                             , need_update_smooth_layer_generated_input_version_property=True   
                                             , need_update_basemeshes_generated_input_version_property=True
                                             , need_update_caves_generated_input_version_property=True
                                             , need_update_dungeons_generated_input_version_property=True
                                             , need_update_tree_program_generated_input_version_property=False
                                             , only_load_road_from_file=False
                                             , run_road_exe=True
                                             , run_worldgen_road=True
                                             , run_upload_smooth_layer=True
                                             , run_make_basemeshes=True
                                             , run_upload_basemeshes=False
                                             , run_make_caves=True
                                             , run_upload_caves=False
                                             , run_make_tree_instances=True
                                             , run_upload_tree_instances=True
                                             , run_create_geochem_entity=True
                                             , run_generate_road_input=False)
    
    return {'success': result.success, 'complete': False, 'error_info': ''}

def test_tree_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start test_tree_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    if update_type == 'msg':
        lambda_host.log('Test tree generation stage complete')
        common_trigger_new_product_version_from_data_entity_handler(vf_api, request, lambda_host)
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def basemeshes_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received base meshes generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name='Base Meshes Generation'
                                             , test_tree_result=False
                                             , need_update_road_generated_input_version_property=False
                                             , need_update_smooth_layer_generated_input_version_property=False  
                                             , need_update_basemeshes_generated_input_version_property=True
                                             , need_update_caves_generated_input_version_property=False
                                             , need_update_dungeons_generated_input_version_property=False
                                             , need_update_tree_program_generated_input_version_property=False
                                             , only_load_road_from_file=False
                                             , run_road_exe=False
                                             , run_worldgen_road=False
                                             , run_upload_smooth_layer=False
                                             , run_make_basemeshes=True
                                             , run_upload_basemeshes=True
                                             , run_make_caves=False
                                             , run_upload_caves=False
                                             , run_make_tree_instances=False
                                             , run_upload_tree_instances=False
                                             , run_create_geochem_entity=False
                                             , run_generate_road_input=False)
    return {'success': result.success, 'complete': False, 'error_info': ''}

def basemeshes_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start basemeshes_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Base meshes generation stage complete')
        #create_view_for_basemesh_entity(vf_api, request, lambda_host) #don't need it any more
        common_trigger_new_product_version_from_data_entity_handler(vf_api, request, lambda_host)
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def caves_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received base meshes generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name='Caves and Dungeons Meshes Generation'
                                             , test_tree_result=False
                                             , need_update_road_generated_input_version_property=False
                                             , need_update_smooth_layer_generated_input_version_property=False   
                                             , need_update_basemeshes_generated_input_version_property=False
                                             , need_update_caves_generated_input_version_property=True
                                             , need_update_dungeons_generated_input_version_property=True
                                             , need_update_tree_program_generated_input_version_property=False
                                             , only_load_road_from_file=False
                                             , run_road_exe=False
                                             , run_worldgen_road=False
                                             , run_upload_smooth_layer=False
                                             , run_make_basemeshes=False
                                             , run_upload_basemeshes=False
                                             , run_make_caves=True
                                             , run_upload_caves=True
                                             , run_make_tree_instances=False
                                             , run_upload_tree_instances=False
                                             , run_create_geochem_entity=False
                                             , run_generate_road_input=False)
    return {'success': result.success, 'complete': False, 'error_info': ''}

def caves_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start caves_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Caves and Dungeons Meshes generation stage complete')
        common_trigger_new_product_version_from_data_entity_handler(vf_api, request, lambda_host)
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def smooth_layer_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received smooth layer generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name='Smooth Layer Generation'
                                             , test_tree_result=False
                                             , need_update_road_generated_input_version_property=True
                                             , need_update_smooth_layer_generated_input_version_property=True   
                                             , need_update_basemeshes_generated_input_version_property=False
                                             , need_update_caves_generated_input_version_property=False
                                             , need_update_dungeons_generated_input_version_property=False
                                             , need_update_tree_program_generated_input_version_property=False
                                             , only_load_road_from_file=False
                                             , run_road_exe=True
                                             , run_worldgen_road=True
                                             , run_upload_smooth_layer=True
                                             , run_make_basemeshes=False
                                             , run_upload_basemeshes=False
                                             , run_make_caves=False
                                             , run_upload_caves=False
                                             , run_make_tree_instances=False
                                             , run_upload_tree_instances=False
                                             , run_create_geochem_entity=False
                                             , run_generate_road_input=False)
    return {'success': result.success, 'complete': False, 'error_info': ''}

def smooth_layer_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start smooth_layer_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Smooth layers generation stage complete')
        common_trigger_new_product_version_from_data_entity_handler(vf_api, request, lambda_host)
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def only_smooth_layer_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received only smooth layer generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name='Only Smooth Layer Generation'
                                             , test_tree_result=False
                                             , need_update_road_generated_input_version_property=False
                                             , need_update_smooth_layer_generated_input_version_property=True   
                                             , need_update_basemeshes_generated_input_version_property=False
                                             , need_update_caves_generated_input_version_property=False
                                             , need_update_dungeons_generated_input_version_property=False
                                             , need_update_tree_program_generated_input_version_property=False
                                             , only_load_road_from_file=False
                                             , run_road_exe=False
                                             , run_worldgen_road=True
                                             , run_upload_smooth_layer=True
                                             , run_make_basemeshes=False
                                             , run_upload_basemeshes=False
                                             , run_make_caves=False
                                             , run_upload_caves=False
                                             , run_make_tree_instances=False
                                             , run_upload_tree_instances=False
                                             , run_create_geochem_entity=False
                                             , run_generate_road_input=False)
    return {'success': result.success, 'complete': False, 'error_info': ''}

def only_smooth_layer_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start only_smooth_layer_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Only smooth layers generation stage complete')
        common_trigger_new_product_version_from_data_entity_handler(vf_api, request, lambda_host)
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def only_tree_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received only tree generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name="Only Tree Generation"
                                             , test_tree_result=False
                                             , need_update_road_generated_input_version_property=False
                                             , need_update_smooth_layer_generated_input_version_property=False   
                                             , need_update_basemeshes_generated_input_version_property=False
                                             , need_update_caves_generated_input_version_property=False
                                             , need_update_dungeons_generated_input_version_property=False
                                             , need_update_tree_program_generated_input_version_property=False
                                             , only_load_road_from_file=False
                                             , run_road_exe=False
                                             , run_worldgen_road=False
                                             , run_upload_smooth_layer=False
                                             , run_make_basemeshes=False
                                             , run_upload_basemeshes=False
                                             , run_make_caves=False
                                             , run_upload_caves=False
                                             , run_make_tree_instances=True
                                             , run_upload_tree_instances=True
                                             , run_create_geochem_entity=True
                                             , run_generate_road_input=False)

    return {'success': result.success, 'complete': False, 'error_info': ''}

def only_tree_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start only_tree_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    if update_type == 'msg':
        lambda_host.log('Only Tree generation stage complete')
        common_trigger_new_product_version_from_data_entity_handler(vf_api, request, lambda_host)
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def test_only_tree_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received test only tree generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name="Test Only Tree Generation"
                                             , test_tree_result=True
                                             , need_update_road_generated_input_version_property=False
                                             , need_update_smooth_layer_generated_input_version_property=False   
                                             , need_update_basemeshes_generated_input_version_property=False
                                             , need_update_caves_generated_input_version_property=False
                                             , need_update_dungeons_generated_input_version_property=False
                                             , need_update_tree_program_generated_input_version_property=False
                                             , only_load_road_from_file=False
                                             , run_road_exe=False
                                             , run_worldgen_road=False
                                             , run_upload_smooth_layer=False
                                             , run_make_basemeshes=False
                                             , run_upload_basemeshes=False
                                             , run_make_caves=False
                                             , run_upload_caves=False
                                             , run_make_tree_instances=True
                                             , run_upload_tree_instances=True
                                             , run_create_geochem_entity=True
                                             , run_generate_road_input=False)
    
    return {'success': result.success, 'complete': False, 'error_info': ''}

def test_only_tree_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start test_only_tree_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    if update_type == 'msg':
        lambda_host.log('Test only tree generation stage complete')
        common_trigger_new_product_version_from_data_entity_handler(vf_api, request, lambda_host)
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def road_input_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received road input generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name='Road Input Generation'
                                             , test_tree_result=False
                                             , need_update_road_generated_input_version_property=False
                                             , need_update_smooth_layer_generated_input_version_property=False   
                                             , need_update_basemeshes_generated_input_version_property=False
                                             , need_update_caves_generated_input_version_property=False
                                             , need_update_dungeons_generated_input_version_property=False
                                             , need_update_tree_program_generated_input_version_property=True
                                             , only_load_road_from_file=False
                                             , run_road_exe=False
                                             , run_worldgen_road=True
                                             , run_upload_smooth_layer=False
                                             , run_make_basemeshes=True
                                             , run_upload_basemeshes=False
                                             , run_make_caves=True
                                             , run_upload_caves=False
                                             , run_make_tree_instances=True
                                             , run_upload_tree_instances=False
                                             , run_create_geochem_entity=False
                                             , run_generate_road_input=True)
    
    request.properties['road_input_generation_lambda_id'] = result.id
    lambda_host.log(f'road_input_generation_lambda_id : {result.id}')
    
    return {'success': result.success, 'complete': False, 'error_info': ''}

def road_input_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log(f'Start road_input_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Road input generation stage complete')
        common_trigger_new_product_version_from_data_entity_handler(vf_api, request, lambda_host)
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def road_changed_tree_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received road changed tree generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name='Road Changed Tree Generation'
                                             , test_tree_result=g_debug_generation
                                             , need_update_road_generated_input_version_property=False
                                             , need_update_smooth_layer_generated_input_version_property=False   
                                             , need_update_basemeshes_generated_input_version_property=False
                                             , need_update_caves_generated_input_version_property=False
                                             , need_update_dungeons_generated_input_version_property=False
                                             , need_update_tree_program_generated_input_version_property=False
                                             , only_load_road_from_file=False
                                             , run_road_exe=True
                                             , run_worldgen_road=True
                                             , run_upload_smooth_layer=True
                                             , run_make_basemeshes=False
                                             , run_upload_basemeshes=False
                                             , run_make_caves=False
                                             , run_upload_caves=False
                                             , run_make_tree_instances=True
                                             , run_upload_tree_instances=True
                                             , run_create_geochem_entity=True
                                             , run_generate_road_input=False)   

    return {'success': result.success, 'complete': False, 'error_info': ''}

def road_changed_tree_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start road_changed_tree_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Road changed tree generation stage complete')
        
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}


def whole_result_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received whole result generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name='Whole Result Generation'
                                             , test_tree_result=False
                                             , need_update_road_generated_input_version_property=True
                                             , need_update_smooth_layer_generated_input_version_property=True   
                                             , need_update_basemeshes_generated_input_version_property=True
                                             , need_update_caves_generated_input_version_property=True
                                             , need_update_dungeons_generated_input_version_property=True
                                             , need_update_tree_program_generated_input_version_property=False
                                             , only_load_road_from_file=False
                                             , run_road_exe=True
                                             , run_worldgen_road=True
                                             , run_upload_smooth_layer=True
                                             , run_make_basemeshes=True
                                             , run_upload_basemeshes=True
                                             , run_make_caves=True
                                             , run_upload_caves=True
                                             , run_make_tree_instances=True
                                             , run_upload_tree_instances=True
                                             , run_create_geochem_entity=True
                                             , run_generate_road_input=False)

    return {'success': result.success, 'complete': False, 'error_info': ''}

def whole_result_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start whole_result_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Whole result generation stage complete')
        common_trigger_new_product_version_from_data_entity_handler(vf_api, request, lambda_host)
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def test_whole_result_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Received test whole result generation data')
    result = common_generation_on_receive_data(vf ,request ,lambda_host
                                             , lambda_name='Test Whole Result Generation'
                                             , test_tree_result=True
                                             , need_update_road_generated_input_version_property=True
                                             , need_update_smooth_layer_generated_input_version_property=True   
                                             , need_update_basemeshes_generated_input_version_property=True
                                             , need_update_caves_generated_input_version_property=True
                                             , need_update_dungeons_generated_input_version_property=True
                                             , need_update_tree_program_generated_input_version_property=False
                                             , only_load_road_from_file=False
                                             , run_road_exe=True
                                             , run_worldgen_road=True
                                             , run_upload_smooth_layer=True
                                             , run_make_basemeshes=True
                                             , run_upload_basemeshes=True
                                             , run_make_caves=True
                                             , run_upload_caves=True
                                             , run_make_tree_instances=True
                                             , run_upload_tree_instances=True
                                             , run_create_geochem_entity=True
                                             , run_generate_road_input=False)
    
    return {'success': result.success, 'complete': False, 'error_info': ''}

def test_whole_result_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    lambda_host.log('Start test_whole_result_generation_on_stage_complete')
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Test whole result generation stage complete')
        common_trigger_new_product_version_from_data_entity_handler(vf_api, request, lambda_host)
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

lambda_host = workflow_lambda.workflow_lambda_host()
lambda_host.set_workflow_definition(
    {
        'id': 'PANGEA_NEXT_TREE_WORKFLOW',
        'name': 'Pangea Next Tree Generation',
        'icon': 'mesh',
        'description': 'Pangea Next Tree Generation Workflow',
        'tracks': 
        [
            {
                'id': 'ASSETS_FILES',
                'name': 'Workflow Assets Files',
                'description': 'The assets to be used in generation',
                'icon': 'mesh',
                'tracks': [
                    {
                        'id': 'TOOLS_FILES',
                        'name': 'Tools Files',
                        'description': 'The collection of tools for tree creation',
                        'icon': 'mesh',
                        'on_receive_data': tools_on_receive_data,
                    },
                    {
                        'id': 'QUADTREE_FILES',
                        'name': 'Quadtree Files',
                        'description': 'A quadtree of the world',
                        'icon': 'mesh',
                        'on_receive_data': quadtree_on_receive_data,
                    },
                    {
                        'id': 'ROAD_DATA_FILES',
                        'name': 'Road Data Files',
                        'description': 'A Input data for generate road',
                        'icon': 'mesh',
                        'on_receive_data': road_data_on_receive_data,
                    },
                    {
                        'id': 'DISPLACEMENT_MAPS_FILES',
                        'name': 'Displacement Maps Files',
                        'description': 'A collection of PNG heightmaps',
                        'icon': 'mesh',
                        'on_receive_data': displacement_maps_on_receive_data,
                    },
                    {
                        'id': 'BASE_MESHES_FILES',
                        'name': 'Base Meshes Files',
                        'description': 'A collection of OBJ meshes',
                        'icon': 'mesh',
                        'on_receive_data': base_meshes_on_receive_data,
                    },
                    {
                        'id': 'CAVES_DUNGEONS_ASSET_FILES',
                        'name': 'Caves and Dungeons Asset Files',
                        'description': 'A collection of Caves and Dungeons Asset Files',
                        'icon': 'mesh',
                        'on_receive_data': caves_dungeons_asset_on_receive_data,
                    },
                    {
                        'id': 'TREE_LIST_FILES',
                        'name': 'Tree List Files',
                        'description': 'A collection of tree classes',
                        'icon': 'mesh',
                        'on_receive_data': tree_list_on_receive_data,
                    },
                    {
                        'id': 'TILE_INFO',
                        'name': 'Tile Info File',
                        'description': 'A tile info files',
                        'icon': 'mesh',
                        'on_receive_data': tile_info_on_receive_data,
                    },
                    {
                        'id': 'PYTHON_CODE_FILES',
                        'name': 'Python Code Files Files',
                        'description': 'A collection of python code files',
                        'icon': 'mesh',
                        'on_receive_data': python_code_on_receive_data,
                    },
                    {
                        'id': 'ROAD_GENARATED_INPUT_FILES',
                        'name': 'Road Generated Input Files',
                        'description': 'A collection of Road Generated Input Files',
                        'icon': 'mesh',
                        'on_receive_data': road_generated_input_on_receive_data,
                    },
                    {
                        'id': 'SMOOTH_LAYER_GENARATED_INPUT_FILES',
                        'name': 'Smooth Layer Generated Input Files',
                        'description': 'A collection of Smooth Layer Generated Input Files',
                        'icon': 'mesh',
                        'on_receive_data': smooth_layer_generated_input_on_receive_data,
                    },
                    {
                        'id': 'BASE_MESHES_GENARATED_INPUT_FILES',
                        'name': 'Base Meshes Generated Input Files',
                        'description': 'A collection of Base Meshes Generated Input Files',
                        'icon': 'mesh',
                        'on_receive_data': basemeshes_generated_input_on_receive_data,
                    },
                    {
                        'id': 'CAVES_GENARATED_INPUT_FILES',
                        'name': 'Caves Generated Input Files',
                        'description': 'A collection of Caves Generated Input Files',
                        'icon': 'mesh',
                        'on_receive_data': caves_generated_input_on_receive_data,
                    },
                    {
                        'id': 'DUNGEONS_GENARATED_INPUT_FILES',
                        'name': 'Dungeons Generated Input Files',
                        'description': 'A collection of Dungeons Generated Input Files',
                        'icon': 'mesh',
                        'on_receive_data': dungeons_generated_input_on_receive_data,
                    },
                    {
                        'id': 'TREE_PROGRAM_GENARATED_INPUT_FILES',
                        'name': 'Tree Program Generated Input Files',
                        'description': 'A collection of Tree Program Generated Input Files',
                        'icon': 'mesh',
                        'on_receive_data': tree_program_generated_input_on_receive_data,
                    },
                ]
            },
            {
                'id': 'WORKFLOW_TREE_GENERATION', 
                'name': 'Workflow Tree Generation',
                'description': 'The generation of the tree',
                'icon': 'mesh',
                'on_receive_data': tree_generation_on_receive_data,
                'on_stage_done': tree_generation_on_stage_complete,
            },
            {
                'id': 'TEST_WORKFLOW_TREE_GENERATION',
                'name': 'Test Workflow Tree Generation',
                'description': 'The test generation of the tree',
                'icon': 'mesh',
                'on_receive_data': test_tree_generation_on_receive_data,
                'on_stage_done': test_tree_generation_on_stage_complete,
            },
            {
                'id': 'WORKFLOW_BASEMESHES_GENERATION', 
                'name': 'Workflow Base Meshes Generation',
                'description': 'The generation of the base meshes',
                'icon': 'mesh',
                'on_receive_data': basemeshes_generation_on_receive_data,
                'on_stage_done': basemeshes_generation_on_stage_complete,
            },
            {
                'id': 'WORKFLOW_CAVES_GENERATION', 
                'name': 'Workflow Caves and Dungeons Meshes Generation',
                'description': 'The generation of the Caves and Dungeons meshes',
                'icon': 'mesh',
                'on_receive_data': caves_generation_on_receive_data,
                'on_stage_done': caves_generation_on_stage_complete,
            },
            {
                'id': 'WORKFLOW_SMOOTH_LAYER_GENERATION',
                'name': 'Workflow Smooth Layers Generation',
                'description': 'The generation of the smooth layers',
                'icon': 'mesh',
                'on_receive_data': smooth_layer_generation_on_receive_data,
                'on_stage_done': smooth_layer_generation_on_stage_complete,
            },
            {
                'id': 'WORKFLOW_ONLY_SMOOTH_LAYER_GENERATION',
                'name': 'Workflow Only Smooth Layers Generation',
                'description': 'The generation of the Only smooth layers',
                'icon': 'mesh',
                'on_receive_data': only_smooth_layer_generation_on_receive_data,
                'on_stage_done': only_smooth_layer_generation_on_stage_complete,
            },
            {
                'id': 'WORKFLOW_ONLY_TREE_GENERATION', 
                'name': 'Workflow Only Tree Generation',
                'description': 'The generation of the only tree',
                'icon': 'mesh',
                'on_receive_data': only_tree_generation_on_receive_data,
                'on_stage_done': only_tree_generation_on_stage_complete,
            },
            {
                'id': 'TEST_WORKFLOW_ONLY_TREE_GENERATION',
                'name': 'Test Workflow Only Tree Generation',
                'description': 'The test generation of the only tree',
                'icon': 'mesh',
                'on_receive_data': test_only_tree_generation_on_receive_data,
                'on_stage_done': test_only_tree_generation_on_stage_complete,
            },
            {
                'id': 'WORKFLOW_ROAD_INPUT_GENERATION',
                'name': 'Workflow Road Input Generation',
                'description': 'The generation of the road input',
                'icon': 'mesh',
                'on_receive_data': road_input_generation_on_receive_data,
                'on_stage_done': road_input_generation_on_stage_complete,
            },
            {
                'id': 'WORKFLOW_ROAD_CHANGED_TREE_GENERATION',
                'name': 'Workflow Road Changed Tree Generation',
                'description': 'The tree generation trigger by road changed',
                'icon': 'mesh',
                'on_receive_data': road_changed_tree_generation_on_receive_data,
                'on_stage_done': road_changed_tree_generation_on_stage_complete,
            },
            {
                'id': 'WORKFLOW_WHOLE_RESULT_GENERATION',
                'name': 'Workflow Whole Result Generation',
                'description': 'The generation of the whole result',
                'icon': 'mesh',
                'on_receive_data': whole_result_generation_on_receive_data,
                'on_stage_done': whole_result_generation_on_stage_complete,
            },
            {
                'id': 'TEST_WORKFLOW_WHOLE_RESULT_GENERATION',
                'name': 'Test Workflow Whole Result Generation',
                'description': 'The test generation of the whole result',
                'icon': 'mesh',
                'on_receive_data': test_whole_result_generation_on_receive_data,
                'on_stage_done': test_whole_result_generation_on_stage_complete,
            }
        ],
    }
)
