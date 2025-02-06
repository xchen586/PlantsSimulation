from voxelfarm import process_lambda
from voxelfarm import voxelfarmclient

def process(vf : voxelfarmclient.rest, lambda_host : process_lambda.process_lambda_host):
    lambda_host.log('Process called.')
    project_id = lambda_host.input_string('project_id', 'project_id')
    entity_id = lambda_host.input_string('entity_id', 'entity_id')

    lambda_host.log(f'project_id: {project_id}')
    lambda_host.log(f'entity_id: {entity_id}')

    attached_files = vf.get_file_list(project_id, entity_id)

    for file_name in attached_files:
        lambda_host.log(f'Removing file {file_name} from entity {entity_id}')    
        # Enable the remove_file line when you are sure everything works as expected. Check the logs first.
        lambda_host.remove_file(file_name, entity_id)
    return {'success': True, 'complete': True, 'error_info': 'None'}

def post_process(vf : voxelfarmclient.rest, lambda_host : process_lambda.process_lambda_host, update_type : str): 
    lambda_host.log('Post-Process called.')
    return {'success': True, 'complete': True, 'error_info': 'None'}

process_api = process_lambda.process_lambda_host()
process_api.log('process_api created')

process_api.log('Call set_workflow_definition')
process_api.set_lambda_definition(process, post_process)