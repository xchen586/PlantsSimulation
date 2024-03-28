import voxelfarm

class process_lambda_framework:

    def __init__(self):  
        pass

    def input_string(self, id, label, default = ""):
        return ""

    def log(self, message):
        pass

    def progress(self, progress, message):
        pass

    def get_cfg(self, name):
        return ""

    def get_scrap_folder(self):
        return ""

    def get_tools_folder(self):
        return ""

    def get_entity_folder(self, id = None):
        return ""

    def download_entity_files(self, id = None):
        return ""

    def download_entity_file(self, filename, id = None):
        return ""

    def attach_file(self, filename, id = None):
        pass

    def attach_folder(self, folder, id = None):
        pass

    def upload(self, filename, name, id = None):
        pass

    def set_exit_code(self, code):
        pass

    def get_entity(self, id = None):
        return None

    def get_entity_file_list(self, id = None):
        return []

    def export_file(self, local_file_location, drop_zone_file_location):
        pass

class process_lambda_host:

    def __init__(self, framework = None):  
        if framework:
            self.lambda_framework = framework
        else:
            if voxelfarm.voxelfarm_framework:
                self.lambda_framework = voxelfarm.voxelfarm_framework
            else:
                self.lambda_framework = process_lambda_framework()

    def input_string(self, id, label, default = ""):
        return self.lambda_framework.input_string(id, label, default)

    def log(self, message):
        self.lambda_framework.log(message)

    def progress(self, progress, message):
        self.lambda_framework.progress(progress, message)

    def get_cfg(self, name):
        return self.lambda_framework.GetCfg(name)

    def get_scrap_folder(self):
        return self.lambda_framework.get_scrap_folder()

    def get_tools_folder(self):
        return self.lambda_framework.get_tools_folder()

    def get_entity_folder(self, id = None):
        return self.lambda_framework.get_entity_folder(id)

    def download_entity_files(self, id = None):
        return self.lambda_framework.download_entity_files(id)

    def download_entity_file(self, filename, id = None):
        return self.lambda_framework.download_entity_file(filename, id)

    def attach_file(self, filename, id = None):
        self.lambda_framework.attach_file(filename, id)

    def attach_folder(self, folder, id = None):
        self.lambda_framework.attach_folder(folder, id)

    def upload(self, filename, name, id = None):
        self.lambda_framework.Upload(filename, name, id)

    def set_exit_code(self, code):
        self.lambda_framework.set_exit_code(code)

    def get_entity(self, id = None):
        return self.lambda_framework.get_entity(id)

    def get_entity_file_list(self, id = None):
        return self.lambda_framework.get_entity_file_list(id)

    def export_file(self, local_file_location, drop_zone_file_location):
        return self.lambda_framework.export_file(local_file_location, drop_zone_file_location)

