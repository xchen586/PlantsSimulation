
import json
import base64
import time
import requests
from datetime import datetime, timedelta
import os
import random
import pandas as pd
from concurrent.futures import ThreadPoolExecutor
from concurrent.futures import as_completed
from io import StringIO
import io

class rest:

    OrgId = '2343243456678890'
    class entity_type:
        View = 'VIEW'
        Project = 'PROJECT'
        VoxelTerrain = 'VOXSURF'
        RealtimeVoxelTerrain = 'VOXOP'
        BlockModel = 'VOXBM'
        IndexedPointCloud = 'IDXPC'
        VoxelGenerator = 'VOXGEN'
        RawPointCloud = 'RAWPC'
        RawHeightmap = 'RAWHM'
        RawBlockModel = 'RAWBM'
        RawMesh = 'RAWMESH'
        IndexedMesh = 'IDXMESH'
        VoxelMesh = 'VOXMESH'
        OrthoImagery = 'ORTHO'
        IndexedOrthoImagery = 'IDXORTHO'
        Program = 'PROGRAM'
        Folder = 'FOLDER'
        RawDensity = 'RAWDENSITY'
        IndexedDensity = 'IDXDENSITY'
        MaterialTracking = 'VOXMT'
        MaterialTrackingOperation = 'VOXMTOP'
        RawDrillHoles = 'RAWDH'
        DrillHoles = 'IDXDH'
        OrthoVox='ORTHOVOX'
        Process='PROCESS'
        Export='EXPORT'
        Report = 'REPORT'
        VoxelPC = 'VOXPC'

    class lambda_type:
        Generator = 'VOXEL'
        Report = 'REPORT'
        View = 'VIEW'
        Workflow = 'WORKFLOW'
        Process = 'PROCESS'

    CRSFields = [
        "coord_origin_x",
        "coord_origin_y",
        "coord_origin_z",
        "coord_hdatum",
        "coord_vdatum",
        "coord_projection",
        "coord_unit",
        "voxel_size",
        "coord_projection_tm_falseEasting",
        "coord_projection_tm_falseNorthing",
        "coord_projection_tm_latOriginDeg",
        "coord_projection_tm_longMeridianDeg",
        "coord_projection_tm_scaleFactor",
        "coord_projection_lcc_falseEasting",
        "coord_projection_lcc_falseNorthing",
        "coord_projection_lcc_latOfOriginDeg",
        "coord_projection_lcc_longOfOriginDeg", 
        "coord_projection_lcc_firstStdParallelDeg",
        "coord_projection_lcc_secondStdParallelDeg",
        "coord_projection_aeac_falseEasting",
        "coord_projection_aeac_falseNorthing",
        "coord_projection_aeac_latOfOriginDeg",
        "coord_projection_aeac_longOfOriginDeg", 
        "coord_projection_aeac_firstStdParallelDeg",
        "coord_projection_aeac_secondStdParallelDeg",
        "coord_projection_amg_zone",
        "coord_projection_utm_easting",
        "coord_projection_utm_northing"]

    DefaultFields = {
        entity_type.VoxelTerrain : {
            'include_classification' : '0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31',
        },
        entity_type.VoxelMesh : {
            'translate_x' : '0',
            'translate_y' : '0',
            'translate_z' : '0',
            'scale_x' : '1',
            'scale_y' : '1',
            'scale_z' : '1',
            'rotate_x' : '0',
            'rotate_y' : '0',
            'rotate_z' : '0',
            'rotation_order' : 'ZXZ',
        },
        entity_type.IndexedMesh : {
            'translate_x' : '0',
            'translate_y' : '0',
            'translate_z' : '0',
            'scale_x' : '1',
            'scale_y' : '1',
            'scale_z' : '1',
            'rotate_x' : '0',
            'rotate_y' : '0',
            'rotate_z' : '0',
            'rotation_order' : 'ZXZ',
        },
        entity_type.BlockModel : {
            'translate_x' : '0',
            'translate_y' : '0',
            'translate_z' : '0',
            'scale_x' : '1',
            'scale_y' : '1',
            'scale_z' : '1',
            'rotate_x' : '0',
            'rotate_y' : '0',
            'rotate_z' : '0',
            'rotation_order' : 'ZXZ',
            'bm_type': 'VOXBM1'
        },
        entity_type.OrthoVox : {
            'translate_x' : '0',
            'translate_y' : '0',
            'translate_z' : '0',
            'scale_x' : '1',
            'scale_y' : '1',
            'scale_z' : '1',
            'rotate_x' : '0',
            'rotate_y' : '0',
            'rotate_z' : '0',
            'rotation_order' : 'ZXZ'
        }
    }

    HTTPSession = requests.Session()

    def __init__(self, apiurl="https://api.voxelfarm.com"):
        self.proxy = None
        self.VFAPIURL = apiurl
        self.VFAPIURL_ENTITY = self.VFAPIURL + '/entity.ashx'
        self.VFAPIURL_FILE = self.VFAPIURL + '/file.ashx'
        self.VFAPIURL_EVENT = self.VFAPIURL + '/events.ashx'
        self.token: str = None
        self.aad_credentials : dict = None
        self.ts_api_version = "2020-07-31"
        self.ts_environmentName = os.getenv('vf_ts_hub_name')
        self.ts_applicationName = os.getenv('vf_ts_app_name')

    def set_proxy(self, proxy):
        self.proxy = proxy

    class api_result:
        def __init__(self):      
            self.success = False
            self.error_info = ''

    class creation_result(api_result):
        def __init__(self):      
            self.id = ''

    class multiple_creation_result():
        def __init__(self):      
            self.success = False
            self.error_info = ''
            self.ids = []

    class wait_result(api_result):
        def __init__(self):      
            self.complete = False
            self.times = {}

    def add_default_fields(self, type, fields):
        if type in self.DefaultFields:
            for field in self.DefaultFields[type]:
                if not field in fields:
                    fields[field] = self.DefaultFields[type][field]
        if not 'file_date' in fields:
            fields['file_date'] = str(1000 * int(time.time()))

    class crs_result(api_result):
        def __init__(self):      
            self.crs = {}

    def get_project_crs(self, project):
        result = self.crs_result()
        params = {'id': project, 'project': project}

        projectRequest = self.HTTPSession.get(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), proxies=self.proxy)
        if projectRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {projectRequest.status_code}' 
            return result
        entityJson = json.loads(projectRequest.text)
        values = {
            'coord_hunit': entityJson['coord_unit'],
            'coord_vunit': entityJson['coord_unit']
        }
        for field in self.CRSFields:
            if field in entityJson:
                values[field] = entityJson[field]
            else:
                values[field] = '0'
        result.crs = values
        result.success = True
        return result

    def get_entity(self, id, project = None):
        params={'id': id}
        if project:
            params['project'] = project
        projectRequest = self.HTTPSession.get(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), proxies=self.proxy)
        if projectRequest.status_code != 200:
            return None
        entityJson = json.loads(projectRequest.text)
        return entityJson

    def get_projects_names(self):
        result = self.collection_result()
        entityRequest = self.HTTPSession.get(self.VFAPIURL_ENTITY, headers=self.get_auth_headers())
        if (entityRequest.status_code == 200):
            entityJson = entityRequest.json()
            items = []
            for x in entityJson:
                v = entityJson[x]
                if v["type"] == "PROJECT_REF":
                    items.append(v["project_ref"])

            result.items = items
            result.success = True
        else:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'  
        return result

    def create_project(self, name, fields):
        result = self.creation_result()
        if fields == None:
            fields = {}
        fields['name'] = name
        self.add_default_fields(type, fields)
        data = {'operation': 'create', 'data': json.dumps(fields)}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, data=data, headers=self.get_auth_headers(), proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result
        result.success = True
        result.id = apiResult['id']
        return result

    def delete_project(self, project):
        items = []
        # Get all entities in project
        result = self.get_collection(project)
        if not result.success:
            print(result.error_info)
            exit(3)
        entities = result.items
        for e in entities:
            entity = entities[e]
            if entity['type'] != 'USER':
                items.append(entity["ID"])
        # Get all user_ref objects for project (users associated with the project)
        result = self.get_collection('users:' + project)
        if not result.success:
            print(result.error_info)
            exit(4)
        user_refs = result.items
        for user_ref in user_refs:
            if user_refs[user_ref]['type'] == 'USER_REF':
                user_id = user_refs[user_ref]['user_ref']
                items.append(user_id + ':' + project)
                items.append(project + ':' + user_id)
        items.append('R3Vlc3Q=:' + project) # guest user
        self.delete_entities(project, items)

    def clean_project(self, project):
        items = []
        # Get all entities in project
        result = self.get_collection(project)
        if not result.success:
            print(result.error_info)
            exit(3)
        entities = result.items
        for e in entities:
            entity = entities[e]
            if entity['type'] != 'USER' and entity['type'] != 'PROJECT':
                items.append(entity["ID"])

        self.delete_entities(project, items)        

    def create_entity_raw(self, project, type, name, fields, crs):
        result = self.creation_result()
        if fields == None:
            fields = {}
        fields['name'] = name
        fields['type'] = 'FILE'
        fields['file_type'] = type
        fields['state'] = 'COMPLETE'
        fields['project'] = project
        for key, value in crs.items():
            fields['entity_' + key] = value
        self.add_default_fields(type, fields)
        params={'project': project}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}' 
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result
        result.success = True
        result.id = apiResult['id']
        return result

    def update_entity(self, project, id, fields):
        result = self.creation_result()
        if fields == None:
            fields = {}
        fields['ID'] = id
        fields['project'] = project
        params={'project': project, 'id': id}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result

        result.success = True
        result.id = id
        return result

    def create_folder(self, project, folder, name, fields = {}):
        result = self.creation_result()
        if folder == None:
            folder = '0'

        fields['name'] = name
        fields['type'] = 'FILE'
        fields['file_type'] = 'FOLDER'
        fields['state'] = 'COMPLETE'
        fields['project'] = project
        fields['file_folder'] = folder
        self.add_default_fields(type, fields)
        params={'project': project}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result

        result.success = True
        result.id = apiResult['id']
        return result

    def create_entity_processed(self, project, type, name, fields, crs, callback=None, update_type = None):
        result = self.creation_result()
        if fields == None:
            fields = {}
        fields['name'] = name
        fields['type'] = 'FILE'
        fields['file_type'] = type
        fields['project'] = project
        fields['state'] = 'PARTIAL'
        if update_type:
            fields['workflow_update_type'] = update_type
        for key, value in crs.items():
            fields[key] = value

        self.add_default_fields(type, fields)
        params={'project': project}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result

        result.id = apiResult['id']
        params={'project': project, 'org': self.OrgId, 'id': result.id}
        data={'process':'PROCESS'}
        if callback != None:
            data['callback'] = callback
        entityRequest = self.HTTPSession.post(self.VFAPIURL_EVENT, params=params, headers=self.get_auth_headers(), data=data, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result
        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result
        result.success = True
        return result

    def create_process_entity(self, project, type, name, fields, crs, files, callback=None, update_type = None):
        result = self.creation_result()

        # Create the Entity
        if fields == None:
            fields = {}
        fields['name'] = name
        fields['type'] = 'FILE'
        fields['file_type'] = type
        fields['state'] = 'PARTIAL'
        fields['project'] = project
        fields['code_origin'] = 'PYTHON_FILE'
        if update_type:
            fields['workflow_update_type'] = update_type

        for key, value in crs.items():
            fields['entity_' + key] = value
        self.add_default_fields(type, fields)
        params={'project': project}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result

        result.id = apiResult['id']

        # Attach files to the entity
        if files:
            for file in files:
                if (not os.path.exists(file)):
                    result.success = False
                    result.error_info = 'File not found: ' + file
                    return result

                attach_files = {'file': open(file, 'rb')}
                apiResult = self.attach_files(project=project, id=result.id, files=attach_files)
                if not apiResult.success:
                    result.success = False
                    result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
                    return result

        fields.clear()
        fields['partial_status'] = ""
        fields['partial_progress'] = "0"

        result = self.update_entity(
            project=project, 
            id=result.id, 
            fields=fields)

        # Process the Entity
        params={'project': project, 'org': self.OrgId, 'id': result.id}
        data={'process':'PROCESS'}
        if callback != None:
            data['callback'] = callback
        entityRequest = self.HTTPSession.post(self.VFAPIURL_EVENT, params=params, headers=self.get_auth_headers(), data=data, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result
        result.success = True
        return result        

    def create_python_process_entity(self, project, name, code, fields, inputs, files = None, callback = None, update_type = None):
        result = self.creation_result()

        # Create the Entity
        if fields == None:
            fields = {}
        fields['name'] = name
        fields['type'] = 'FILE'
        fields['file_type'] = 'PROCESS'
        fields['state'] = 'PARTIAL'
        fields['project'] = project

        fields['code'] = code
        if files and code in files:
            fields['code_origin'] = 'PYTHON_FILE'
        else:
            fields['code_origin'] = 'PROGRAM'

        if update_type:
            fields['workflow_update_type'] = update_type

        if inputs:
            for input in inputs:
                fields['input_value_' + input] = inputs[input]
                fields['input_filter_' + input] = '0'
                fields['input_type_' + input] = '0'
                fields['input_label_' + input] = '0'

        self.add_default_fields(type, fields)
        params={'project': project}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result

        result.id = apiResult['id']

        # Attach files to the entity
        if files:
            for file in files:
                if (not os.path.exists(file)):
                    result.success = False
                    result.error_info = 'File not found: ' + file
                    return result

                attach_files = {'file': open(file, 'rb')}
                apiResult = self.attach_files(project=project, id=result.id, files=attach_files)
                if not apiResult.success:
                    result.success = False
                    result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
                    return result

        fields.clear()
        fields['partial_status'] = ""
        fields['partial_progress'] = "0"

        result = self.update_entity(
            project=project, 
            id=result.id, 
            fields=fields)

        # Process the Entity
        params={'project': project, 'org': self.OrgId, 'id': result.id}
        data={'process':'PROCESS'}
        if callback != None:
            data['callback'] = callback
        entityRequest = self.HTTPSession.post(self.VFAPIURL_EVENT, params=params, headers=self.get_auth_headers(), data=data, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result
        result.success = True
        return result        

    def create_workflow_entity(self, project, name, fields, inputs):
        result = self.creation_result()

        # Create the Entity
        if fields == None:
            fields = {}
        fields['name'] = name
        fields['type'] = 'FILE'
        fields['file_type'] = 'PROCESS'
        fields['process_type'] = 'WORKFLOW'
        fields['state'] = 'PARTIAL'
        fields['project'] = project

        for input in inputs:
            fields['input_value_' + input] = inputs[input]
            fields['input_filter_' + input] = '0'
            fields['input_type_' + input] = '0'
            fields['input_label_' + input] = '0'

        self.add_default_fields(type, fields)
        params={'project': project}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result

        result.id = apiResult['id']

        fields.clear()
        fields['partial_status'] = ""
        fields['partial_progress'] = "0"

        result = self.update_entity(
            project=project, 
            id=result.id, 
            fields=fields)

        # Process the Entity
        params={'project': project, 'org': self.OrgId, 'id': result.id}
        data={'process':'PROCESS'}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_EVENT, params=params, headers=self.get_auth_headers(), data=data, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result
        result.success = True
        return result        

    def create_report(self, project, program, lod, region, name, fields, inputs, callback=None, update_type = None):
        result = self.creation_result()
        if fields == None:
            fields = {}
        fields['name'] = name
        fields['type'] = 'FILE'
        fields['file_type'] = 'REPORT'
        fields['program'] = program
        fields['project'] = project
        fields['region'] = region
        fields['lod'] = str(lod)
        fields['state'] = 'PARTIAL'
        if update_type:
            fields['workflow_update_type'] = update_type
        for input in inputs:
            fields['input_value_' + input] = inputs[input]
            fields['input_filter_' + input] = '0'
            fields['input_type_' + input] = '0'
            fields['input_label_' + input] = '0'
        self.add_default_fields(type, fields)
        params={'project': project}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result

        result.id = apiResult['id']
        params={'project': project, 'org': self.OrgId, 'id': result.id}
        data={'process':'RUN_REPORT'}
        if callback != None:
            data['callback'] = callback
        entityRequest = self.HTTPSession.post(self.VFAPIURL_EVENT, params=params, headers=self.get_auth_headers(), data=data, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
        else:
            result.success = True

        return result

    def create_export(self, project, program, lod, region, name, fields, inputs, callback=None, update_type = None):
        result = self.creation_result()
        if fields == None:
            fields = {}
        fields['name'] = name
        fields['type'] = 'FILE'
        fields['file_type'] = 'EXPORT'
        fields['program'] = program
        fields['project'] = project
        fields['region'] = region
        fields['lod'] = str(lod)
        fields['state'] = 'PARTIAL'
        if update_type:
            fields['workflow_update_type'] = update_type
        for input in inputs:
            fields['input_value_' + input] = inputs[input]
            fields['input_filter_' + input] = '0'
            fields['input_type_' + input] = '0'
            fields['input_label_' + input] = '0'
        self.add_default_fields(type, fields)
        params={'project': project}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result
        result.id = apiResult['id']
        params={'project': project, 'org': self.OrgId, 'id': result.id}
        data={'process':'RUN_EXPORT'}
        if callback != None:
            data['callback'] = callback
        entityRequest = self.HTTPSession.post(self.VFAPIURL_EVENT, params=params, headers=self.get_auth_headers(), data=data, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
        else:
            result.success = True

        return result

    def reprocess_entity(self, id):
        result = self.creation_result()
        result.id = id

        report_entity = self.get_entity(id)
        if report_entity:
            project = report_entity["project"]

            params = {
                'project': project,
                'id' : id
            }

            fields = {
                'state': 'PARTIAL',
                'partial_progress': '0',
                'partial_status': '',
                'proccessing_time': '0'
            }

            entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
            if entityRequest.status_code != 200:
                result.success = False
                result.error_info = f'HTTP Error code {entityRequest.status_code}'
                return result

            apiResult = json.loads(entityRequest.text)
            if ('result' not in apiResult) or (apiResult['result'] != 'success'):
                result.success = False
                if 'error_info' in apiResult:
                    result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
                else:    
                    result.error_info = 'error_info not found'
                return result

            params={'project': project, 'org': self.OrgId, 'id': id}
            data={'process':'RUN_REPORT'}
            entityRequest = self.HTTPSession.post(self.VFAPIURL_EVENT, params=params, headers=self.get_auth_headers(), data=data, proxies=self.proxy)
            if entityRequest.status_code != 200:
                result.success = False
                result.error_info = f'HTTP Error code {entityRequest.status_code}'
                return result
            apiResult = json.loads(entityRequest.text)
            if ('result' not in apiResult) or (apiResult['result'] != 'success'):
                result.success = False
                if 'error_info' in apiResult:
                    result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
                else:    
                    result.error_info = 'error_info not found'
            else:
                result.success = True
        else:
            result.success = False

        return result        

    def create_lambda_python(self, project, type, name, code, fields, files=None):
        result = self.creation_result()
        if fields == None:
            fields = {}

        fields['name'] = name
        fields['type'] = 'FILE'
        fields['file_type'] = 'PROGRAM'
        fields['program_type'] = type
        fields['state'] = 'COMPLETE'

        if code.endswith('.py'):
            fields['code_origin'] = 'PYTHON_FILE'
            fields['code'] = code
        else:
            generatorLambdaCode = code.encode('ascii')
            generatorLambdaCode = base64.b64encode(generatorLambdaCode).decode('ascii')
            fields['code'] = generatorLambdaCode

        fields['project'] = project
        self.add_default_fields(type, fields)
        params={'project': project}
        entityRequest = requests.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result
        
        lambda_id = apiResult['id']
        if files:
            for file in files:
                if (not os.path.exists(file)):
                    result.success = False
                    result.error_info = 'File not found: ' + file
                    return result

                attach_files = {'file': open(file, 'rb')}
                apiResult = self.attach_files(project=project, id=lambda_id, files=attach_files)
                if not apiResult.success:
                    result.success = False
                    result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
                    return result
                        
        result.success = True
        result.id = lambda_id
        return result
    
    def create_generator(self, project, program, name, fields, inputs):
        result = self.creation_result()
        if fields == None:
            fields = {}
        fields['name'] = name
        fields['type'] = 'FILE'
        fields['file_type'] = 'VOXGEN'
        fields['program'] = program
        fields['state'] = 'COMPLETE'
        fields['project'] = project
        for input in inputs:
            fields['input_value_' + input] = inputs[input]
            fields['input_filter_' + input] = '0'
            fields['input_type_' + input] = '0'
            fields['input_label_' + input] = '0'
        self.add_default_fields(type, fields)
        params={'project': project}
        entityRequest = requests.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), json=fields, proxies=self.proxy)
        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result
        result.success = True
        result.id = apiResult['id']
        return result

    def is_processing_complete(self, project, ids):
        result = self.wait_result()
        for id in ids:
            params = {'id': id, 'project': project}
            entityRequest = self.HTTPSession.get(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers())
            if entityRequest.status_code == 200:
                entityJson = json.loads(entityRequest.text)
                if not 'state' in entityJson:
                    result.complete = False
                    result.success = False
                    result.error_info = json.dumps(entityJson)
                    break
                if entityJson['state'] == 'PARTIAL':
                    result.complete = False
                    result.success = True
                    break
                if entityJson['state'] == 'COMPLETE':
                    result.complete = True
                    result.success = True
                    if 'processing_time' in entityJson:
                        result.times[id] = entityJson['processing_time']
                    else:    
                        result.times[id] = ''
                if entityJson['state'] == 'ERROR':
                    result.complete = False
                    result.success = False
                    result.error_info = base64.b64decode(entityJson['error_info']).decode('ascii')
                    break
            else:
                result.complete = False
                result.success = False
                result.error_info = 'HTTP Error ' + str(entityRequest.status_code)
                break

        return result

    def delete_entities(self, project, ids):
        result = self.api_result()
        deleteIdentifierList = ''
        for id in ids:
            deleteIdentifierList = deleteIdentifierList + id + ' '
        params = {'project': project, 'org': self.OrgId, 'operation': 'delete'}
        data = {'operation': 'delete', 'items': deleteIdentifierList}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers(), data=data, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = 'Error deleting entities'
            return result

        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result
        else:    
            result.success = True
        return result

    def attach_files(self, project, id, files):
        result = self.api_result()
        params = {'project': project, 'org': self.OrgId, 'id': id}
        entityRequest = self.HTTPSession.post(self.VFAPIURL_FILE, params=params, headers=self.get_auth_headers(), files=files, proxies=self.proxy)
        if entityRequest.status_code != 200:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
            return result
        apiResult = json.loads(entityRequest.text)
        if ('result' not in apiResult) or (apiResult['result'] != 'success'):
            result.success = False
            if 'error_info' in apiResult:
                result.error_info = base64.b64decode(apiResult['error_info']).decode('ascii')
            else:    
                result.error_info = 'error_info not found'
            return result
        else:    
            result.success = True
        return result

    def get_file(self, project, entity, filename):
        params = {'id': entity, 'project': project, 'org': self.OrgId, 'filename': filename}
        entityRequest = self.HTTPSession.get(self.VFAPIURL_FILE, params=params, headers=self.get_auth_headers(), proxies=self.proxy)
        if entityRequest.status_code != 200:
            return ''
        return entityRequest.text

    def get_binary_file(self, project, entity, filename):
        params = {'id': entity, 'project': project, 'org': self.OrgId, 'filename': filename}
        entityRequest = self.HTTPSession.get(self.VFAPIURL_FILE, params=params, headers=self.get_auth_headers(), proxies=self.proxy)
        if entityRequest.status_code != 200:
            return None
        return entityRequest.content  

    def download_file(self, project, entity, filename, destination):
        params = {'id': entity, 'project': project, 'org': self.OrgId, 'filename': filename}
        entityRequest = self.HTTPSession.get(self.VFAPIURL_FILE, params=params, headers=self.get_auth_headers(), proxies=self.proxy)
        if (entityRequest.status_code) != 200:
            return False
        with open(destination, 'wb') as file:
            file.write(entityRequest.content)
        return True

    def get_file_list(self, project, entity):
        params = {'id': entity, 'project': project, 'org': self.OrgId}
        entityRequest = self.HTTPSession.get(self.VFAPIURL_FILE, params=params, headers=self.get_auth_headers(), proxies=self.proxy)

        list = []
        if entityRequest.status_code == 200:
            for line in entityRequest.text.split("\n"):
                if not line.strip():
                    continue
                list.append(line.lstrip())        
        return list

    class collection_result(api_result):
        items = None

    def get_collection(self, id):
        result = self.collection_result()
        params = {'project': id}
        entityRequest = self.HTTPSession.get(self.VFAPIURL_ENTITY, params=params, headers=self.get_auth_headers())
        if (entityRequest.status_code == 200):
            entityJson = json.loads(entityRequest.text)
            result.items = entityJson
            result.success = True
        else:
            result.success = False
            result.error_info = f'HTTP Error code {entityRequest.status_code}'
        return result

    def compute_region_from_aabbs(self, entity_id):
        entity = self.get_entity(entity_id)
        min_x = float(entity["aabb_min_x"]) - 20
        max_x = float(entity["aabb_max_x"]) + 20
        min_y = float(entity["aabb_min_y"]) - 20
        max_y = float(entity["aabb_max_y"]) + 20
        min_z = float(entity["aabb_min_z"]) - 20
        max_z = float(entity["aabb_max_z"]) + 20
        region = '1'
        region += "," + str(min_z)
        region += "," + str(max_z)
        region += "," + str(min_x)
        region += "," + str(min_y)
        region += "," + str(max_x)
        region += "," + str(min_y)
        region += "," + str(max_x)
        region += "," + str(max_y)
        region += "," + str(min_x)
        region += "," + str(max_y)
        return region

    def workflow_get_product_property(self, project_id, product_id, property, version = 0):
        project = self.get_entity(project_id, project_id)
        if project:
            product_key = 'workflow_folder_' + product_id
            if product_key in project:
                product_folder_id = project[product_key]
                if version > 0:
                    apiResult = self.get_collection(project_id)
                    if apiResult.success:
                        version_name = 'Version ' + str(version)
                        entities = apiResult.items
                        for e in entities:
                            entity = entities[e]
                            if ('file_folder' in entity) and (entity['file_folder'] == product_folder_id) and ('file_type' in entity) and (entity['file_type'] == 'FOLDER'):
                                if entity["name"] == version_name:
                                    extended_prop = 'version_' + property
                                    if extended_prop in entity:
                                        return entity[extended_prop]
                                    break    
                else:
                    product_entity = self.get_entity(product_folder_id, project_id)
                    if product_entity and ('version_active' in product_entity):
                        active_version_id = product_entity['version_active']
                        active_version = self.get_entity(active_version_id, project_id)
                        if active_version:
                            extended_prop = 'version_' + property
                            if extended_prop in active_version:
                                return active_version[extended_prop]
        return None

    def workflow_get_parameter_dataframe(self, project_id, product_id, version = 0):
        attribute_product = self.workflow_get_product_property(project_id, product_id, 'report_entity', version)
        if attribute_product:
            attribute = self.get_file(project_id, attribute_product, 'report.csv')
            if attribute:
                content = StringIO(attribute)
                return pd.read_csv(content)
        return pd.DataFrame()

    def workflow_get_entity_id(self, project_id, product_id, alias_id, version = 0):
        project = self.get_entity(project_id, project_id)
        if project:
            product_key = 'workflow_folder_' + product_id
            if product_key in project:
                product_folder_id = project[product_key]
                if version > 0:
                    apiResult = self.get_collection(project_id)
                    if apiResult.success:
                        version_name = 'Version ' + str(version)
                        entities = apiResult.items
                        for e in entities:
                            entity = entities[e]
                            if ('file_folder' in entity) and (entity['file_folder'] == product_folder_id) and ('file_type' in entity) and (entity['file_type'] == 'FOLDER'):
                                if entity["name"] == version_name:
                                    extended_prop = 'alias_' + alias_id
                                    if extended_prop in entity:
                                        return entity[extended_prop]
                                    break    
                else:
                    product_entity = self.get_entity(product_folder_id, project_id)
                    if product_entity and ('version_active' in product_entity):
                        active_version_id = product_entity['version_active']
                        active_version = self.get_entity(active_version_id, project_id)
                        if active_version:
                            extended_prop = 'alias_' + alias_id
                            if extended_prop in active_version:
                                return active_version[extended_prop]
        return None

    def workflow_get_dataframe(self, project_id, product_id, alias_id, file_name = None, version = 0):
        attribute_product = self.workflow_get_entity_id(project_id, product_id, alias_id, version)
        if attribute_product:
            dataframe_file = 'report.csv'
            if file_name:
                dataframe_file = file_name

            split_tup = os.path.splitext(dataframe_file)
            file_extension = split_tup[1].lower()
            if file_extension == '.csv':
                attribute = self.get_file(project_id, attribute_product, dataframe_file)
                if attribute:
                    content = StringIO(attribute)
                    return pd.read_csv(content)
            elif file_extension == '.ftr':
                attribute = self.get_binary_file(project_id, attribute_product, dataframe_file)
                if attribute:
                    content = io.BytesIO(attribute)
                    return pd.read_feather(content)

        return pd.DataFrame()

    def workflow_get_timings(self, project_id):
        entities = []
        collections = self.get_collection(project_id).items
        
        for entity_id in collections:
            if "processing_start_date" in collections[entity_id] and "file_folder" in collections[entity_id]:
                file_folder_entity = collections[collections[entity_id]["file_folder"]]
                processing_time = float(collections[entity_id]["processing_time"]) if "processing_time" in collections[entity_id] else 0
                if processing_time > 0:
                    file_date = int(collections[entity_id]["file_date"]) if "file_date" in collections[entity_id] else 0
                    processing_start_date = int(collections[entity_id]["processing_start_date"]) if "processing_start_date" in collections[entity_id] else 0
                    queue_time = processing_start_date - file_date
                    workflow_time = file_date - int(file_folder_entity["capture_date"] if "capture_date" in file_folder_entity else file_date)
                    entities.append({
                        "id": collections[entity_id]["ID"],
                        "file_folder": collections[entity_id]["file_folder"],
                        "file_date": pd.to_datetime(file_date / 1000, unit='s'),
                        "workflow_name": file_folder_entity["workflow_product"] if "workflow_product" in file_folder_entity else "N/A",
                        "name": collections[entity_id]["name"],
                        "state": collections[entity_id]["state"],
                        "version": int(file_folder_entity["version_number"]) if "version_number" in file_folder_entity else "N/A",
                        "workflow_time_ms": workflow_time,
                        "processing_time_ms": processing_time,
                        "queue_time_ms": queue_time
                    })

        return pd.DataFrame.from_dict(entities)    

#region AAD_Credentials
    def set_file_credentials(self, filename: str):
        self.aad_credentials = self.get_env_file_as_dict(filename)
        self.token = self.get_token()
        if (self.token == None):
            print("credentials file: "+filename )
            raise Exception("Check that the credentials are correct")

    def set_credentials(self, tenant: str, client_id: str, client_secret: str):
        self.aad_credentials = {
            "TENANT": tenant,
            "CLIENT_ID": client_id,
            "CLIENT_SECRET":client_secret
        }
        self.token = self.get_token()
        if (self.token == None):
            print("Set Credentials failed ClientId : "+ client_id)
            raise Exception("Check that the credentials are correct")

    def get_env_file_as_dict(self, path: str) -> dict:
        if not os.path.isfile(path):
            t_path = os.path.dirname(__file__)+ path
            if os.path.isfile(t_path):    
                path = t_path
        with open(path, 'r') as f:
            return dict(tuple(line.replace('\n', '').split('=')) for line 
                        in f.readlines() if not line.startswith('#'))

    def get_token(self, ressource=None):
        if ressource is None:
            ressource = self.aad_credentials["CLIENT_ID"]

        if (self.aad_credentials["TENANT"] != None):
            authority = "https://login.microsoftonline.com/"+self.aad_credentials["TENANT"]+"/oauth2/token"
            headers = {'Content-Type': 'application/x-www-form-urlencoded'}
            data = {
                "grant_type" : "client_credentials",
                "client_id" : self.aad_credentials["CLIENT_ID"],
                "client_secret": self.aad_credentials["CLIENT_SECRET"],
                "scope" : "https://graph.microsoft.com/.default",
                "resource": ressource,
            }

            HTTPSession = requests.Session()
            tokenRequest = HTTPSession.post(authority, data=data, headers=headers)
            HTTPSession.close()

            if tokenRequest.status_code == 200:
                tokenResult = tokenRequest.json()
                return tokenResult["access_token"]
        return None

    def check_ExpiredToken(self, token: str):
        body = token.split(".")[1]
        body += "=" * ((4 - len(body) % 4) % 4) #ugh
        txt = base64.b64decode(body).decode('utf-8')
        jwt = json.loads(txt)
        expire = int(jwt["exp"])
        return (datetime.fromtimestamp(expire)) < (datetime.now()+timedelta(minutes=2))

    def get_auth_headers(self):
        if (self.token==None):
            return None
        if self.check_ExpiredToken(self.token):
            self.token = self.get_token()    
        
        return { 'Authorization': f'Bearer {self.token}' }
# endregion AAD_Credentials

# region GetTimeSeriesData
    def get_time_series_data_by_search(self, timespan, projectID, spatialID=None, objectID=None, properties=None, filter=None):
        authorizationToken = self.get_token(
            ressource="https://api.timeseries.azure.com")
        searchString = ""
        if spatialID is not None and spatialID != "" and spatialID != "*":
            searchString = searchString + spatialID + ", "
        if objectID is not None and objectID != "" and objectID != "*":
            searchString = searchString + objectID

        if(searchString[len(searchString)-2:] == ", "):
            searchString = searchString[:len(searchString)-2]

        url = (
            "https://"
            + self.get_environment_id(authorizationToken)
            + ".env.timeseries.azure.com/timeseries/instances/search?"
        )
        querystring = {"api-version": self.ts_api_version}

        payload = {"searchString": f"\"{searchString}\"",
                   "instances": {"pageSize": 100}}

        headers = {
            "Authorization": f"Bearer {authorizationToken}",
            "Content-Type": "application/json",
            "cache-control": "no-cache",
        }

        try:
            jsonResponse = requests.request(
                "POST",
                url,
                data=json.dumps(payload),
                headers=headers,
                params=querystring,
            )
            jsonResponse.raise_for_status()
        except requests.exceptions.ConnectTimeout:
            raise Exception("The request to the TSI api timed out.")
        except requests.exceptions.HTTPError:
            raise Exception(
                "The request to the TSI api returned an unsuccessfull status code.")

        response = json.loads(jsonResponse.text)
        if response["instances"]["hits"]:
            result = response["instances"]["hits"]
            processes = []
            with ThreadPoolExecutor(max_workers=100) as executor:
                for i, _ in enumerate(result):
                    processes.append(
                        executor.submit(
                            lambda p: self.get_data_by_id(*p),
                            [
                                result[i]["timeSeriesId"],
                                properties,
                                filter,
                                timespan,
                                authorizationToken,
                            ],
                        )
                    )

                while "continuationToken" in list(response["instances"].keys()):
                    headers["x-ms-continuation"] = response["instances"]["continuationToken"]
                    jsonResponse = requests.request(
                        "POST",
                        url,
                        data=json.dumps(payload),
                        headers=headers,
                        params=querystring,
                    )
                    jsonResponse.raise_for_status()

                    response = json.loads(jsonResponse.text)
                    result = response["instances"]["hits"]
                    for i, _ in enumerate(result):
                        processes.append(
                            executor.submit(
                                lambda p: self.get_data_by_id(*p),
                                [
                                    result[i]["timeSeriesId"],
                                    properties,
                                    filter,
                                    timespan,
                                    authorizationToken,
                                ],
                            )
                        )
            return self.build_dataframe_from_processes(processes, properties)
        else:
            return pd.DataFrame()

    def get_time_series_data_by_ids(self, projectID, timeseriesIDs, timespan):
        authorizationToken = self.get_token(
            ressource="https://api.timeseries.azure.com")

        processes = []
        with ThreadPoolExecutor(max_workers=100) as executor:
            for i, id in enumerate(timeseriesIDs):
                processes.append(
                    executor.submit(
                        lambda p: self.get_data_by_id(*p),
                        [
                            [id["spatialID"], id["objectID"]],
                            id["properties"] if "properties" in id else None,
                            id["filter"] if "filter" in id else None,
                            timespan,
                            authorizationToken,
                        ],
                    )
                )

        return self.build_dataframe_from_processes(processes, None)

    def get_environment_id(self, authorizationToken):
        url = "https://api.timeseries.azure.com/environments"

        querystring = {"api-version": self.ts_api_version}

        payload = ""
        headers = {
            "x-ms-client-application-name": self.ts_applicationName,
            "Authorization": f"Bearer {authorizationToken}",
            "Content-Type": "application/json",
            "cache-control": "no-cache",
        }

        try:
            response = requests.request(
                "GET",
                url,
                data=payload,
                headers=headers,
                params=querystring,
                timeout=10,
            )
            response.raise_for_status()
        except requests.exceptions.ConnectTimeout:
            raise Exception("The request to the TSI api timed out.")
        except requests.exceptions.HTTPError:
            raise Exception(
                "The request to the TSI api returned an unsuccessfull status code.")

        environments = json.loads(response.text)["environments"]
        environmentId = None
        for environment in environments:
            if environment["displayName"] == self.ts_environmentName:
                environmentId = environment["environmentId"]
                break
        if environmentId == None:
            raise Exception(
                "TSI environment not found. Check the spelling or create an environment in Azure TSI.")

        return environmentId

    def get_data_by_id(
        self,
        timeseries,
        properties,
        filter,
        timespan,
        authorizationToken,
    ):

        url = (
            "https://"
            + self.get_environment_id(authorizationToken)
            + ".env.timeseries.azure.com/timeseries/query?"
        )
        querystring = {"api-version": self.ts_api_version}
        requestType = "getEvents"

        result = {}

        projectedProperties = None
        if(properties != "*" and properties != None and properties != ["*"]):
            projectedProperties = properties
        retry = 0

        payload = {
            requestType: {
                "timeSeriesId": timeseries,
                "searchSpan": {"from": timespan[0], "to": timespan[1]},
                "take": 250000,
                "projectedProperties": projectedProperties,
            }
        }

        if filter is not None and filter != "":
            payload[requestType]["filter"] = {
                "tsx": filter
            }

        headers = {
            "x-ms-client-application-name": self.ts_applicationName,
            "Authorization": f"Bearer {authorizationToken}",
            "Content-Type": "application/json",
            "cache-control": "no-cache",
        }

        while retry < 50:  # Looping with a small random wait if the API returns us problems. This solve an issue where consicutive requests goes beyond TSI max number of concurrent requests.
            try:
                jsonResponse = requests.request(
                    "POST",
                    url,
                    data=json.dumps(payload),
                    headers=headers,
                    params=querystring,
                )
                jsonResponse.raise_for_status()
                break
            except requests.exceptions.HTTPError:
                error = json.loads(jsonResponse.text)
                if(error["error"]["code"] == "TooManyRequests"):
                    # Too many requests at the same time. Waiting for a bit before retrying."
                    time.sleep(random.randint(1, 5))
                    retry += 1
                    continue
                else:
                    raise Exception(
                        "The request to the TSI api returned an unsuccessfull status code.")

        response = json.loads(jsonResponse.text)
        result = response
        result["timeseries"] = timeseries
        retry = 0

        while "continuationToken" in list(response.keys()):
            headers = {
                "x-ms-client-application-name": self.ts_applicationName,
                "Authorization": f"Bearer {authorizationToken}",
                "Content-Type": "application/json",
                "cache-control": "no-cache",
                "x-ms-continuation": response["continuationToken"],
            }
            while retry < 50:
                try:
                    jsonResponse = requests.request(
                        "POST",
                        url,
                        data=json.dumps(payload),
                        headers=headers,
                        params=querystring,
                    )
                    jsonResponse.raise_for_status()
                    break
                except requests.exceptions.HTTPError:
                    error = json.loads(jsonResponse.text)
                    if(error["error"]["code"] == "TooManyRequests"):
                        # Too many requests at the same time. Waiting for a bit before retrying."
                        time.sleep(random.randint(1, 5))
                        retry += 1
                        continue
                    else:
                        raise Exception(
                            "The request to the TSI api returned an unsuccessfull status code.")

            response = json.loads(jsonResponse.text)
            result["timestamps"].extend(response["timestamps"])
            for i, property in enumerate(response["properties"]):
                result["properties"][i]["values"].extend(property["values"])
        retry = 0
        return result

    def build_dataframe_from_processes(self, processes, dTypes):
        if processes is None or len(processes) == 0:
            return pd.DataFrame()

        lambdaData = []
        dfColumns = ["timestamp", "spatialID", "objectID"]
        for task in as_completed(processes):
            result = task.result()
            if "timestamps" in result:
                for i, (timestamp) in enumerate(result["timestamps"]):
                    lambdaData.append({ "timestamp": timestamp,
                                        "spatialID": result["timeseries"][0],
                                        "objectID": result["timeseries"][1],
                                        "properties": []})
                    properties = []
                    for (j, property) in enumerate(result["properties"]):
                        if property["name"] == "spatialId" or property["name"] == "objectId":
                            continue
                        properties.append({
                            "name": property["name"],
                            "value": property["values"][i],
                        })
                        if not property["name"] in dfColumns:
                            dfColumns.append(property["name"])
                    lambdaData[len(lambdaData)-1]["properties"] = properties

        dataframe = pd.DataFrame(columns=dfColumns)
        arDf = []
        for i, (event) in enumerate(lambdaData):
            values = [pd.to_datetime(event["timestamp"]), event["spatialID"], event["objectID"]]
            indexes = ['timestamp', 'spatialID', 'objectID']
            for _, (property) in enumerate(event["properties"]):
                if(property["name"] in indexes):
                    # Received multiple different values for the exact same date and property. Update the value to the latest data we received.
                    values[indexes.index(property["name"])] = property["value"]
                else :
                    values.append(property["value"])
                    indexes.append(property["name"])
            arDf.append(pd.DataFrame([values], columns=indexes))
        dataframe = pd.concat([dataframe, pd.concat(arDf)])

        # Force dtype to what has been requestion in the properties object when calling a search, if any.
        for dtype in enumerate(dTypes):
            name = dtype[1]["name"]
            type = dtype[1]["type"]
            if name in dataframe.columns and type.lower() == 'double':
                dataframe[name] = dataframe[name].astype('float64')
        return dataframe
# endregion GetTimeSeriesData
