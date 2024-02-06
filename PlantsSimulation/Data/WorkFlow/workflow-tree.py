from voxelfarm import workflow_lambda
from voxelfarm import voxelfarmclient


def road_data_on_receive_data(
        vf_api : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received road data')
    return {'success': True, 'complete': True, 'error_info': 'None'}

def base_meshes_on_receive_data(
        vf_api : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received base meshes')
    return {'success': True, 'complete': True, 'error_info': 'None'}

def displacement_maps_on_receive_data(
        vf_api : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received displacement maps')
    return {'success': True, 'complete': True, 'error_info': 'None'}

def quadtree_on_receive_data(
        vf_api : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received quadtree')
    return {'success': True, 'complete': True, 'error_info': 'None'}

def tools_on_receive_data(
        vf_api : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received quadtree')
    return {'success': True, 'complete': True, 'error_info': 'None'}

def tree_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received tree generation data')
    request.properties['my_property'] = 'my_value'

    result = lambda_host.process_lambda_entity(
        workflow_request=request,
        name="Lambda",
        inputs={
            'project_id': request.project_id,
            'msg': 'Hello world'
        },
        code='xc_cloud_tree_creation.py',
        files=['xc_cloud_tree_creation.py'],
        update_type='msg')

    return {'success': True, 'complete': False, 'error_info': ''}

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

lambda_host = workflow_lambda.workflow_lambda_host()
lambda_host.set_workflow_definition(
    {
        'id': 'PANGEA_Tree',
        'name': 'Pangea Next Tree',
        'icon': 'mesh',
        'description': 'Pangea Tree workflow',
        'tracks': 
        [
            {
                'id': 'ASSETS',
                'name': 'Assets',
                'description': 'The assets to be used in generation',
                'icon': 'mesh',
                'tracks': [
                    {
                        'id': 'ROAD_DATA',
                        'name': 'Road Data',
                        'description': 'A Input data for generate road',
                        'icon': 'mesh',
                        'on_receive_data': road_data_on_receive_data,
                    },
                    {
                        'id': 'BASE_MESHES',
                        'name': 'Base Meshes',
                        'description': 'A collection of OBJ meshes',
                        'icon': 'mesh',
                        'on_receive_data': base_meshes_on_receive_data,
                    },
                    {
                        'id': 'DISPLACEMENT_MAPS',
                        'name': 'Displacement Maps',
                        'description': 'A collection of PNG heightmaps',
                        'icon': 'mesh',
                        'on_receive_data': displacement_maps_on_receive_data,
                    },
                    {
                        'id': 'QUADTREE',
                        'name': 'Quadtree',
                        'description': 'A quadtree of the world',
                        'icon': 'mesh',
                        'on_receive_data': quadtree_on_receive_data,
                    },
                    {
                        'id': 'TOOLS',
                        'name': 'Tools',
                        'description': 'The collection of tools for tree creation',
                        'icon': 'mesh',
                        'on_receive_data': tools_on_receive_data,
                    },
                    
                ]
            },
            {
                'id': 'TREE_GENERATION',
                'name': 'Tree Generation',
                'description': 'The generation of the tree',
                'icon': 'mesh',
                'on_receive_data': tree_generation_on_receive_data,
                'on_stage_done': tree_generation_on_stage_complete,
            }
        ],
    }
)