from voxelfarm import workflow_lambda
from voxelfarm import voxelfarmclient

import os
import configparser

tile_size = 10
tile_x = 8
tile_y = 5
level = 6
tree_lod =  8
forest_age = 15000
tree_iteration = 300
    
def create_view_for_basemesh_entity(vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('start create_view_for_basemesh_entity')
    
    entity_id = request.raw_entity_id
    
    info_ini_name = 'lambda_info.ini'
    section_entity = 'Entity'
    
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
    lambda_host.log('Received Python code')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating Python code raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

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
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

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
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

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
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
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
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

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
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
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
            'name' : 'Input files', 
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
    request.properties['my_property'] = 'my_value'

    lambda_host.log(f'request.product_folder_id is {request.product_folder_id}')
    lambda_host.log(f'request.active_version_folder_id is {request.active_version_folder_id}')
    lambda_host.log(f'request.version_folder_id is {request.version_folder_id}')

    pythoncode_active_version_property = request.get_product_property('PYTHON_CODE_FILES', 'raw_data')
    treelist_active_version_property = request.get_product_property('TREE_LIST_FILES', 'raw_data')
    roaddata_active_version_property = request.get_product_property('ROAD_DATA_FILES', 'raw_data')
    basemeshes_active_version_property = request.get_product_property('BASE_MESHES_FILES', 'raw_data')
    displacement_active_version_property = request.get_product_property('DISPLACEMENT_MAPS_FILES', 'raw_data')
    qtree_active_version_property = request.get_product_property('QUADTREE_FILES', 'raw_data')
    tools_active_version_property = request.get_product_property('TOOLS_FILES', 'raw_data')
    
    game_tree_entity_id_property = '0B4C084415C744B48B4BD13D9990E713' # xuan test 
    #game_tree_entity_id_property = "3A3CFEBA226B4692A8719C78335470DD"  #game entity 
    
    tree_instances_folder_id_property = '90F6348AD5D94FCEA85C7C1CD081CE97' #Pangea Next -> Instances
    tree_geochems_folder_id_property = 'C2C9E711B8A74E0FB8401646BCF3396C' #Pangea Next -> Workflow Output -> Workflow Tree GeoChems Output
    workflow_output_version_folder_id_property = '68396F90F7CE48B4BA1412EA020ED92A' #Pangea Next -> Workflow Output -> Workflow BaseMeshes Output
    
    if request.version_folder_id != None:
        tree_instances_folder_id_property = request.version_folder_id
        tree_geochems_folder_id_property = request.version_folder_id
        workflow_output_version_folder_id_property = request.version_folder_id
        lambda_host.log(f'Assign the output folder id as request.version_folder_id : {request.version_folder_id}')

    result = lambda_host.process_lambda_entity(
        workflow_request=request,
        name="Tree Lambda",
        inputs={
            'tree_instances_folder_id_property': tree_instances_folder_id_property,
            'game_tree_entity_id_property': game_tree_entity_id_property,
            'workflow_output_version_folder_id_property': workflow_output_version_folder_id_property,
            'tree_geochems_folder_id_property': tree_geochems_folder_id_property,
            'lambda_entity_id': request.raw_entity_id,
            'project_id': request.project_id,
            'pythoncode_active_version_property': pythoncode_active_version_property,
            'treelist_active_version_property': treelist_active_version_property,
            'roaddata_active_version_property': roaddata_active_version_property,
            'basemeshes_active_version_property': basemeshes_active_version_property,
            'displacement_active_version_property': displacement_active_version_property,
            'qtree_active_version_property': qtree_active_version_property,
            'tools_active_version_property': tools_active_version_property,
            'run_update_basemeshes_assets': False,
            'run_road_exe': True,
            'run_worldgen_road': True,
            'run_upload_smooth_layer': False,
            'run_make_basemeshes': True,
            'run_upload_basemeshes': False,
            'run_make_tree_instances':True,
            'run_upload_tree_instances': True,
            'run_create_geochem_entity': True,
            'run_generate_road_input': False,
            'tile_size': tile_size,
            'tile_x': tile_x,
            'tile_y': tile_y,
            'level' : level,
            'tree_lod': tree_lod,
            'forest_age': forest_age,
            'tree_iteration':tree_iteration
        },
        code='xc_cloud_tree_creation.py',
        files=['xc_cloud_tree_creation.py', 'xc_lambda-uploaddb.py'],
        update_type='msg')

    return {'success': result.success, 'complete': False, 'error_info': ''}

def tree_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    if update_type == 'msg':
        lambda_host.log('Tree generation stage complete')
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def basemeshes_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received base meshes generation data')
    request.properties['my_property'] = 'my_value'
    
    lambda_host.log(f'request.product_folder_id is {request.product_folder_id}')
    lambda_host.log(f'request.active_version_folder_id is {request.active_version_folder_id}')
    lambda_host.log(f'request.version_folder_id is {request.version_folder_id}')
    
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

    pythoncode_active_version_property = request.get_product_property('PYTHON_CODE_FILES', 'raw_data')
    treelist_active_version_property = request.get_product_property('TREE_LIST_FILES', 'raw_data')
    roaddata_active_version_property = request.get_product_property('ROAD_DATA_FILES', 'raw_data')
    basemeshes_active_version_property = request.get_product_property('BASE_MESHES_FILES', 'raw_data')
    displacement_active_version_property = request.get_product_property('DISPLACEMENT_MAPS_FILES', 'raw_data')
    qtree_active_version_property = request.get_product_property('QUADTREE_FILES', 'raw_data')
    tools_active_version_property = request.get_product_property('TOOLS_FILES', 'raw_data')
    
    game_tree_entity_id_property = '3A3CFEBA226B4692A8719C78335470DD' 
    
    tree_instances_folder_id_property = '90F6348AD5D94FCEA85C7C1CD081CE97' #Pangea Next -> Instances
    workflow_output_version_folder_id_property = '68396F90F7CE48B4BA1412EA020ED92A' #Pangea Next -> Workflow Output -> Workflow BaseMeshes Output
    tree_geochems_folder_id_property = 'C2C9E711B8A74E0FB8401646BCF3396C' #Pangea Next -> Workflow Output -> Workflow Tree GeoChems Output    
    if request.version_folder_id != None:
        tree_instances_folder_id_property = request.version_folder_id
        tree_geochems_folder_id_property = request.version_folder_id
        workflow_output_version_folder_id_property = request.version_folder_id
        lambda_host.log(f'Assign the output folder id as request.version_folder_id : {request.version_folder_id}')
    
    result = lambda_host.process_lambda_entity(
        workflow_request=request,
        name="Base Meshses Lambda",
        inputs={
            'tree_instances_folder_id_property': tree_instances_folder_id_property,
            'game_tree_entity_id_property': game_tree_entity_id_property,
            'workflow_output_version_folder_id_property': workflow_output_version_folder_id_property,
            'tree_geochems_folder_id_property': tree_geochems_folder_id_property,
            'lambda_entity_id': request.raw_entity_id,
            'project_id': request.project_id,
            'pythoncode_active_version_property': pythoncode_active_version_property,
            'treelist_active_version_property': treelist_active_version_property,
            'roaddata_active_version_property': roaddata_active_version_property,
            'basemeshes_active_version_property': basemeshes_active_version_property,
            'displacement_active_version_property': displacement_active_version_property,
            'qtree_active_version_property': qtree_active_version_property,
            'tools_active_version_property': tools_active_version_property,
            'run_update_basemeshes_assets': False,
            'run_road_exe': False,
            'run_worldgen_road': False,
            'run_upload_smooth_layer': False,
            'run_make_basemeshes': True,
            'run_upload_basemeshes': True,
            'run_make_tree_instances':False,
            'run_upload_tree_instances': False,
            'run_create_geochem_entity': False,
            'run_generate_road_input': False,
            'tile_size': tile_size,
            'tile_x': tile_x,
            'tile_y': tile_y,
            'level' : level,
            'tree_lod': tree_lod,
            'forest_age': forest_age,
            'tree_iteration':tree_iteration
        },
        code='xc_cloud_tree_creation.py',
        files=['xc_cloud_tree_creation.py', 'xc_lambda-uploaddb.py'],
        update_type='msg')
    
    return {'success': result.success, 'complete': False, 'error_info': ''}

def basemeshes_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Base meshes generation stage complete')
        #create_view_for_basemesh_entity(vf_api, request, lambda_host) #don't need it any more
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def smooth_layer_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received smooth layer generation data')
    request.properties['my_property'] = 'my_value'

    lambda_host.log(f'request.product_folder_id is {request.product_folder_id}')
    lambda_host.log(f'request.active_version_folder_id is {request.active_version_folder_id}')
    lambda_host.log(f'request.version_folder_id is {request.version_folder_id}')

    pythoncode_active_version_property = request.get_product_property('PYTHON_CODE_FILES', 'raw_data')
    treelist_active_version_property = request.get_product_property('TREE_LIST_FILES', 'raw_data')
    roaddata_active_version_property = request.get_product_property('ROAD_DATA_FILES', 'raw_data')
    basemeshes_active_version_property = request.get_product_property('BASE_MESHES_FILES', 'raw_data')
    displacement_active_version_property = request.get_product_property('DISPLACEMENT_MAPS_FILES', 'raw_data')
    qtree_active_version_property = request.get_product_property('QUADTREE_FILES', 'raw_data')
    tools_active_version_property = request.get_product_property('TOOLS_FILES', 'raw_data')
    
    game_tree_entity_id_property = "3A3CFEBA226B4692A8719C78335470DD"  #game entity 
    
    tree_instances_folder_id_property = '90F6348AD5D94FCEA85C7C1CD081CE97' #Pangea Next -> Instances
    tree_geochems_folder_id_property = 'C2C9E711B8A74E0FB8401646BCF3396C' #Pangea Next -> Workflow Output -> Workflow Tree GeoChems Output
    workflow_output_version_folder_id_property = '68396F90F7CE48B4BA1412EA020ED92A' #Pangea Next -> Workflow Output -> Workflow BaseMeshes Output
    
    if request.version_folder_id != None:
        tree_instances_folder_id_property = request.version_folder_id
        tree_geochems_folder_id_property = request.version_folder_id
        workflow_output_version_folder_id_property = request.version_folder_id
        lambda_host.log(f'Assign the output folder id as request.version_folder_id : {request.version_folder_id}')

    result = lambda_host.process_lambda_entity(
        workflow_request=request,
        name="Smooth Layers Lambda",
        inputs={
            'tree_instances_folder_id_property': tree_instances_folder_id_property,
            'game_tree_entity_id_property': game_tree_entity_id_property,
            'workflow_output_version_folder_id_property': workflow_output_version_folder_id_property,
            'tree_geochems_folder_id_property': tree_geochems_folder_id_property,
            'lambda_entity_id': request.raw_entity_id,
            'project_id': request.project_id,
            'pythoncode_active_version_property': pythoncode_active_version_property,
            'treelist_active_version_property': treelist_active_version_property,
            'roaddata_active_version_property': roaddata_active_version_property,
            'basemeshes_active_version_property': basemeshes_active_version_property,
            'displacement_active_version_property': displacement_active_version_property,
            'qtree_active_version_property': qtree_active_version_property,
            'tools_active_version_property': tools_active_version_property,
            'run_update_basemeshes_assets': False,
            'run_road_exe': True,
            'run_worldgen_road': True,
            'run_upload_smooth_layer': True,
            'run_make_basemeshes': False,
            'run_upload_basemeshes': False,
            'run_make_tree_instances':False,
            'run_upload_tree_instances': False,
            'run_create_geochem_entity': False,
            'run_generate_road_input': False,
            'tile_size': tile_size,
            'tile_x': tile_x,
            'tile_y': tile_y,
            'level' : level,
            'tree_lod': tree_lod,
            'forest_age': forest_age,
            'tree_iteration':tree_iteration
        },
        code='xc_cloud_tree_creation.py',
        files=['xc_cloud_tree_creation.py', 'xc_lambda-uploaddb.py'],
        update_type='msg')

    return {'success': result.success, 'complete': False, 'error_info': ''}

def smooth_layer_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Smooth layers stage complete')
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def road_input_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received road input generation data')
    request.properties['my_property'] = 'my_value'

    lambda_host.log(f'request.product_folder_id is {request.product_folder_id}')
    lambda_host.log(f'request.active_version_folder_id is {request.active_version_folder_id}')
    lambda_host.log(f'request.version_folder_id is {request.version_folder_id}')

    pythoncode_active_version_property = request.get_product_property('PYTHON_CODE_FILES', 'raw_data')
    treelist_active_version_property = request.get_product_property('TREE_LIST_FILES', 'raw_data')
    roaddata_active_version_property = request.get_product_property('ROAD_DATA_FILES', 'raw_data')
    basemeshes_active_version_property = request.get_product_property('BASE_MESHES_FILES', 'raw_data')
    displacement_active_version_property = request.get_product_property('DISPLACEMENT_MAPS_FILES', 'raw_data')
    qtree_active_version_property = request.get_product_property('QUADTREE_FILES', 'raw_data')
    tools_active_version_property = request.get_product_property('TOOLS_FILES', 'raw_data')
    
    game_tree_entity_id_property = "3A3CFEBA226B4692A8719C78335470DD"  #game entity 
    
    tree_instances_folder_id_property = '90F6348AD5D94FCEA85C7C1CD081CE97' #Pangea Next -> Instances
    tree_geochems_folder_id_property = 'C2C9E711B8A74E0FB8401646BCF3396C' #Pangea Next -> Workflow Output -> Workflow Tree GeoChems Output
    workflow_output_version_folder_id_property = '68396F90F7CE48B4BA1412EA020ED92A' #Pangea Next -> Workflow Output -> Workflow BaseMeshes Output
    
    if request.version_folder_id != None:
        tree_instances_folder_id_property = request.version_folder_id
        tree_geochems_folder_id_property = request.version_folder_id
        workflow_output_version_folder_id_property = request.version_folder_id
        lambda_host.log(f'Assign the output folder id as request.version_folder_id : {request.version_folder_id}')

    result = lambda_host.process_lambda_entity(
        workflow_request=request,
        name="Road Input Lambda",
        inputs={
            'tree_instances_folder_id_property': tree_instances_folder_id_property,
            'game_tree_entity_id_property': game_tree_entity_id_property,
            'workflow_output_version_folder_id_property': workflow_output_version_folder_id_property,
            'tree_geochems_folder_id_property': tree_geochems_folder_id_property,
            'lambda_entity_id': request.raw_entity_id,
            'project_id': request.project_id,
            'pythoncode_active_version_property': pythoncode_active_version_property,
            'treelist_active_version_property': treelist_active_version_property,
            'roaddata_active_version_property': roaddata_active_version_property,
            'basemeshes_active_version_property': basemeshes_active_version_property,
            'displacement_active_version_property': displacement_active_version_property,
            'qtree_active_version_property': qtree_active_version_property,
            'tools_active_version_property': tools_active_version_property,
            'run_update_basemeshes_assets': False,
            'run_road_exe': True,
            'run_worldgen_road': True,
            'run_upload_smooth_layer': False,
            'run_make_basemeshes': True,
            'run_upload_basemeshes': False,
            'run_make_tree_instances':True,
            'run_upload_tree_instances': False,
            'run_create_geochem_entity': False,
            'run_generate_road_input': True,
            'tile_size': tile_size,
            'tile_x': tile_x,
            'tile_y': tile_y,
            'level' : level,
            'tree_lod': tree_lod,
            'forest_age': forest_age,
            'tree_iteration':tree_iteration
        },
        code='xc_cloud_tree_creation.py',
        files=['xc_cloud_tree_creation.py', 'xc_lambda-uploaddb.py'],
        update_type='msg')

    return {'success': result.success, 'complete': False, 'error_info': ''}

def road_input_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Road input stage complete')
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
                        'id': 'TREE_LIST_FILES',
                        'name': 'Tree List Files',
                        'description': 'A collection of tree classes',
                        'icon': 'mesh',
                        'on_receive_data': tree_list_on_receive_data,
                    },
                    {
                        'id': 'PYTHON_CODE_FILES',
                        'name': 'Python Code Files Files',
                        'description': 'A collection of python code files',
                        'icon': 'mesh',
                        'on_receive_data': python_code_on_receive_data,
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
                'id': 'WORKFLOW_BASEMESHES_GENERATION',
                'name': 'Workflow Base Meshes Generation',
                'description': 'The generation of the base meshes',
                'icon': 'mesh',
                'on_receive_data': basemeshes_generation_on_receive_data,
                'on_stage_done': basemeshes_generation_on_stage_complete,
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
                'id': 'WORKFLOW_ROAD_INPUT_GENERATION',
                'name': 'Workflow Road Input Generation',
                'description': 'The generation of the road input',
                'icon': 'mesh',
                'on_receive_data': road_input_generation_on_receive_data,
                'on_stage_done': road_input_generation_on_stage_complete,
            }
        ],
    }
)